#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <curl/curl.h>
#include <jansson.h>
#include <signal.h>
#include <ctype.h>

#define VERSION "1.2.4"
#define USER_AGENT "check_ncpa/" VERSION

// 全局变量用于超时处理
volatile sig_atomic_t timeout_flag = 0;

// 数据结构存储选项
typedef struct {
    char *hostname;
    char *metric;
    int port;
    char *warning;
    char *critical;
    char *units;
    char *unit;
    char *arguments;
    char *token;
    int timeout;
    int delta;
    int list;
    int verbose;
    int debug;
    int version;
    char *queryargs;
    int secure;
    int performance;
} Options;

// 用于存储CURL响应数据
struct MemoryStruct {
    char *memory;
    size_t size;
};

// 超时信号处理器
void timeout_handler(int sig) {
    (void)sig;
    timeout_flag = 1;
}

// 初始化选项
void init_options(Options *opts) {
    memset(opts, 0, sizeof(Options));
    opts->port = 5693;
    opts->timeout = 55;
}

// 释放选项内存
void free_options(Options *opts) {
    if (opts->hostname) free(opts->hostname);
    if (opts->metric) free(opts->metric);
    if (opts->warning) free(opts->warning);
    if (opts->critical) free(opts->critical);
    if (opts->units) free(opts->units);
    if (opts->unit) free(opts->unit);
    if (opts->arguments) free(opts->arguments);
    if (opts->token) free(opts->token);
    if (opts->queryargs) free(opts->queryargs);
}

// CURL写回调函数
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

// 构建URL - 对需要转义的字符进行编码
char *build_url(Options *opts) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return NULL;
    }
    
    // 基础URL
    size_t base_len = snprintf(NULL, 0, "https://%s:%d/api", opts->hostname, opts->port);
    
    // 计算总长度
    size_t total_len = base_len + 1; // +1 for null terminator
    
    // 为转义后的metric和arguments预留空间
    char *escaped_metric = NULL;
    char *escaped_arguments = NULL;
    
    // 转义metric部分
    if (opts->metric && *opts->metric) {
        escaped_metric = curl_easy_escape(curl, opts->metric, 0);
        if (!escaped_metric) {
            curl_easy_cleanup(curl);
            return NULL;
        }
        total_len += strlen(escaped_metric) + 1; // +1 for '/'
    }
    
    // 转义arguments部分
    if (opts->arguments && *opts->arguments) {
        escaped_arguments = curl_easy_escape(curl, opts->arguments, 0);
        if (!escaped_arguments) {
            if (escaped_metric) {
                curl_free(escaped_metric);
            }
            curl_easy_cleanup(curl);
            return NULL;
        }
        total_len += strlen(escaped_arguments) + 1; // +1 for '/'
    }
    
    // 添加结尾斜杠
    total_len += 1; // for trailing '/'
    
    // 分配内存
    char *full_url = malloc(total_len);
    if (!full_url) {
        if (escaped_metric) {
            curl_free(escaped_metric);
        }
        if (escaped_arguments) {
            curl_free(escaped_arguments);
        }
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    // 构建基础URL
    char *p = full_url;
    p += snprintf(p, total_len, "https://%s:%d/api", opts->hostname, opts->port);
    
    // 添加转义后的metric部分
    if (escaped_metric) {
        // 确保路径分隔
        *p++ = '/';
        
        // 复制转义后的metric字符串
        size_t metric_len = strlen(escaped_metric);
        memcpy(p, escaped_metric, metric_len);
        p += metric_len;
        
        // 释放转义后的metric内存
        curl_free(escaped_metric);
    }
    
    // 添加转义后的arguments部分
    if (escaped_arguments) {
        // 确保路径分隔
        *p++ = '/';
        
        // 复制转义后的arguments字符串
        size_t args_len = strlen(escaped_arguments);
        memcpy(p, escaped_arguments, args_len);
        p += args_len;
        
        // 释放转义后的arguments内存
        curl_free(escaped_arguments);
    }
    
    // 确保路径以斜杠结尾（匹配Python版本行为）
    *p++ = '/';
    *p = '\0';
    
    curl_easy_cleanup(curl);
    return full_url;
}

// 构建查询字符串 - 修复内存错误
char *build_query_string(Options *opts) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return strdup("");
    }
    
    // 使用动态缓冲区
    char *query = NULL;
    size_t query_size = 0;
    size_t query_capacity = 256;
    
    query = malloc(query_capacity);
    if (!query) {
        curl_easy_cleanup(curl);
        return strdup("");
    }
    query[0] = '\0';
    
    // 添加基本参数
    if (opts->token && *opts->token) {
        char *token_enc = curl_easy_escape(curl, opts->token, 0);
        if (token_enc) {
            size_t len_needed = strlen(query) + strlen(token_enc) + 8; // "token=" + token_enc
            if (len_needed >= query_capacity) {
                query_capacity = len_needed + 256;
                char *new_query = realloc(query, query_capacity);
                if (!new_query) {
                    free(query);
                    curl_free(token_enc);
                    curl_easy_cleanup(curl);
                    return strdup("");
                }
                query = new_query;
            }
            strcat(query, "token=");
            strcat(query, token_enc);
            curl_free(token_enc);
        }
    }
    
    if (!opts->list) {
        if (opts->warning && *opts->warning) {
            char *warning_enc = curl_easy_escape(curl, opts->warning, 0);
            if (warning_enc) {
                size_t len_needed = strlen(query) + strlen(warning_enc) + 10; // "&warning=" + warning_enc
                if (len_needed >= query_capacity) {
                    query_capacity = len_needed + 256;
                    char *new_query = realloc(query, query_capacity);
                    if (!new_query) {
                        free(query);
                        curl_free(warning_enc);
                        curl_easy_cleanup(curl);
                        return strdup("");
                    }
                    query = new_query;
                }
                strcat(query, query[0] ? "&warning=" : "warning=");
                strcat(query, warning_enc);
                curl_free(warning_enc);
            }
        }
        if (opts->critical && *opts->critical) {
            char *critical_enc = curl_easy_escape(curl, opts->critical, 0);
            if (critical_enc) {
                size_t len_needed = strlen(query) + strlen(critical_enc) + 11; // "&critical=" + critical_enc
                if (len_needed >= query_capacity) {
                    query_capacity = len_needed + 256;
                    char *new_query = realloc(query, query_capacity);
                    if (!new_query) {
                        free(query);
                        curl_free(critical_enc);
                        curl_easy_cleanup(curl);
                        return strdup("");
                    }
                    query = new_query;
                }
                strcat(query, query[0] ? "&critical=" : "critical=");
                strcat(query, critical_enc);
                curl_free(critical_enc);
            }
        }
        if (opts->delta) {
            size_t len_needed = strlen(query) + 7; // "&delta=1"
            if (len_needed >= query_capacity) {
                query_capacity = len_needed + 256;
                char *new_query = realloc(query, query_capacity);
                if (!new_query) {
                    free(query);
                    curl_easy_cleanup(curl);
                    return strdup("");
                }
                query = new_query;
            }
            strcat(query, query[0] ? "&delta=1" : "delta=1");
        }
        
        size_t len_needed = strlen(query) + 8; // "&check=1"
        if (len_needed >= query_capacity) {
            query_capacity = len_needed + 256;
            char *new_query = realloc(query, query_capacity);
            if (!new_query) {
                free(query);
                curl_easy_cleanup(curl);
                return strdup("");
            }
            query = new_query;
        }
        strcat(query, query[0] ? "&check=1" : "check=1");
        
        if (opts->unit && *opts->unit) {
            char *unit_enc = curl_easy_escape(curl, opts->unit, 0);
            if (unit_enc) {
                size_t len_needed = strlen(query) + strlen(unit_enc) + 7; // "&unit=" + unit_enc
                if (len_needed >= query_capacity) {
                    query_capacity = len_needed + 256;
                    char *new_query = realloc(query, query_capacity);
                    if (!new_query) {
                        free(query);
                        curl_free(unit_enc);
                        curl_easy_cleanup(curl);
                        return strdup("");
                    }
                    query = new_query;
                }
                strcat(query, "&unit=");
                strcat(query, unit_enc);
                curl_free(unit_enc);
            }
        }
    }
    
    if (opts->units && *opts->units) {
        char *units_enc = curl_easy_escape(curl, opts->units, 0);
        if (units_enc) {
            size_t len_needed = strlen(query) + strlen(units_enc) + 8; // "&units=" + units_enc
            if (len_needed >= query_capacity) {
                query_capacity = len_needed + 256;
                char *new_query = realloc(query, query_capacity);
                if (!new_query) {
                    free(query);
                    curl_free(units_enc);
                    curl_easy_cleanup(curl);
                    return strdup("");
                }
                query = new_query;
            }
            strcat(query, query[0] ? "&units=" : "units=");
            strcat(query, units_enc);
            curl_free(units_enc);
        }
    }
    
    // 添加额外查询参数
    if (opts->queryargs && *opts->queryargs) {
        char *args = strdup(opts->queryargs);
        char *saveptr = NULL;
        char *token = strtok_r(args, ",", &saveptr);
        
        while (token) {
            char *eq = strchr(token, '=');
            if (eq) {
                *eq = '\0';
                char *key = token;
                char *value = eq + 1;
                
                char *key_enc = curl_easy_escape(curl, key, 0);
                char *value_enc = curl_easy_escape(curl, value, 0);
                
                if (key_enc && value_enc) {
                    size_t len_needed = strlen(query) + strlen(key_enc) + strlen(value_enc) + 2; // "&" + "="
                    if (len_needed >= query_capacity) {
                        query_capacity = len_needed + 256;
                        char *new_query = realloc(query, query_capacity);
                        if (!new_query) {
                            free(query);
                            free(args);
                            curl_free(key_enc);
                            curl_free(value_enc);
                            curl_easy_cleanup(curl);
                            return strdup("");
                        }
                        query = new_query;
                    }
                    strcat(query, query[0] ? "&" : "");
                    strcat(query, key_enc);
                    strcat(query, "=");
                    strcat(query, value_enc);
                }
                
                if (key_enc) curl_free(key_enc);
                if (value_enc) curl_free(value_enc);
            }
            token = strtok_r(NULL, ",", &saveptr);
        }
        
        free(args);
    }
    
    curl_easy_cleanup(curl);
    return query;
}

// 执行HTTP请求
CURLcode perform_request(Options *opts, struct MemoryStruct *chunk, long *http_code) {
    CURL *curl;
    CURLcode res = CURLE_OK;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (!curl) {
        return CURLE_FAILED_INIT;
    }
    
    // 构建基础URL
    char *base_url = build_url(opts);
    if (!base_url) {
        curl_easy_cleanup(curl);
        return CURLE_OUT_OF_MEMORY;
    }
    
    // 构建查询字符串
    char *query = build_query_string(opts);
    
    // 合并为完整URL
    char *full_url = NULL;
    if (query && *query) {
        full_url = malloc(strlen(base_url) + strlen(query) + 2);
        if (full_url) {
            sprintf(full_url, "%s?%s", base_url, query);
        }
    } else {
        full_url = strdup(base_url);
    }
    
    if (!full_url) {
        free(base_url);
        if (query) free(query);
        curl_easy_cleanup(curl);
        return CURLE_OUT_OF_MEMORY;
    }
    
    if (opts->verbose) {
        fprintf(stderr, "Connecting to: %s\n", full_url);
    }
    
    // 设置URL
    curl_easy_setopt(curl, CURLOPT_URL, full_url);
    
    // 设置回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);
    
    // 设置超时
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, opts->timeout);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, opts->timeout);
    
    // 设置User-Agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    
    // 修复：SSL验证逻辑 - 匹配Python版本的行为
    // Python版本默认不验证SSL，除非使用-s选项
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, opts->secure ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, opts->secure ? 2L : 0L);
    
    // 执行请求
    res = curl_easy_perform(curl);
    
    // 获取HTTP状态码
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, http_code);
    
    // 清理
    curl_easy_cleanup(curl);
    free(base_url);
    if (query) free(query);
    free(full_url);
    
    return res;
}

// 处理JSON响应
int process_json(Options *opts, json_t *root, char **output) {
    if (opts->list) {
        // 列表模式
        char *json_str = json_dumps(root, JSON_INDENT(4));
        if (!json_str) return 3;
        
        *output = json_str;
        return 0;
    } else {
        // 检查模式
        json_t *value = json_object_get(root, "value");
        if (value) {
            // NCPA < 2 兼容处理
            root = value;
        }
        
        const char *stdout_str = NULL;
        int returncode = 3; // 默认未知状态
        char *allocated_stdout = NULL;
        
        json_t *stdout_json = json_object_get(root, "stdout");
        json_t *returncode_json = json_object_get(root, "returncode");
        json_t *error_json = json_object_get(root, "error");
        
        if (error_json && json_is_string(error_json)) {
            stdout_str = json_string_value(error_json);
            returncode = 2; // CRITICAL
        } else if (stdout_json && returncode_json) {
            // 处理旧版本NCPA的兼容性问题
            if (json_is_integer(stdout_json) && !json_is_integer(returncode_json)) {
                // 交换stdout和returncode
                int tmp = json_integer_value(stdout_json);
                if (json_is_string(returncode_json)) {
                    stdout_str = json_string_value(returncode_json);
                } else {
                    stdout_str = "UNKNOWN: Invalid response format";
                }
                returncode = tmp;
            } else {
                if (json_is_string(stdout_json)) {
                    stdout_str = json_string_value(stdout_json);
                } else if (json_is_integer(stdout_json)) {
                    // 处理数值型输出
                    allocated_stdout = malloc(64);
                    if (allocated_stdout) {
                        snprintf(allocated_stdout, 64, "%lld", json_integer_value(stdout_json));
                        stdout_str = allocated_stdout;
                    } else {
                        stdout_str = "UNKNOWN: Memory allocation error";
                    }
                } else if (json_is_real(stdout_json)) {
                    // 处理浮点型输出
                    allocated_stdout = malloc(64);
                    if (allocated_stdout) {
                        snprintf(allocated_stdout, 64, "%.2f", json_real_value(stdout_json));
                        stdout_str = allocated_stdout;
                    } else {
                        stdout_str = "UNKNOWN: Memory allocation error";
                    }
                }
                if (json_is_integer(returncode_json)) {
                    returncode = json_integer_value(returncode_json);
                }
            }
        }
        
        if (!stdout_str) {
            *output = strdup("UNKNOWN: Invalid response format");
            if (allocated_stdout) free(allocated_stdout);
            return 3;
        }
        
        // 性能数据处理
        if (opts->performance && strstr(stdout_str, "|") == NULL) {
            *output = malloc(strlen(stdout_str) + 50);
            if (*output) {
                sprintf(*output, "%s | 'status'=%d;1;2;;", stdout_str, returncode);
            } else {
                *output = strdup("UNKNOWN: Memory allocation error");
            }
        } else {
            *output = strdup(stdout_str);
        }
        
        if (allocated_stdout) free(allocated_stdout);
        return returncode;
    }
}

// 主逻辑函数
int run_check(Options *opts, char **output) {
    struct MemoryStruct chunk = {0};
    CURLcode res;
    json_t *root = NULL;
    json_error_t error;
    int exit_code = 3; // 默认未知状态
    long http_code = 0;
    
    // 设置超时处理器
    signal(SIGALRM, timeout_handler);
    alarm(opts->timeout);
    
    // 执行HTTP请求
    res = perform_request(opts, &chunk, &http_code);
    
    if (timeout_flag) {
        *output = strdup("UNKNOWN: Execution exceeded timeout threshold");
        goto cleanup;
    }
    
    if (res != CURLE_OK) {
        const char *error_str = curl_easy_strerror(res);
        *output = malloc(strlen("UNKNOWN: ") + strlen(error_str) + 2);
        if (*output) {
            sprintf(*output, "UNKNOWN: %s", error_str);
        } else {
            *output = strdup("UNKNOWN: Memory allocation error");
        }
        goto cleanup;
    }
    
    // 检查HTTP状态码
    if (http_code != 200) {
        *output = malloc(100);
        if (*output) {
            sprintf(*output, "UNKNOWN: HTTP error %ld", http_code);
        } else {
            *output = strdup("UNKNOWN: HTTP error");
        }
        
        // 添加响应内容到错误信息（如果有）
        if (chunk.memory && chunk.size > 0) {
            size_t len = chunk.size > 200 ? 200 : chunk.size;
            char *tmp = malloc(strlen(*output) + len + 50);
            if (tmp) {
                sprintf(tmp, "%s: %.*s", *output, (int)len, chunk.memory);
                free(*output);
                *output = tmp;
            }
        }
        exit_code = 3;
        goto cleanup;
    }
    
    // 检查是否有数据返回
    if (!chunk.memory || chunk.size == 0) {
        *output = strdup("UNKNOWN: Empty response from server");
        goto cleanup;
    }
    
    // 调试输出：打印原始响应
    if (opts->debug) {
        fprintf(stderr, "Raw response: %.*s\n", (int)(chunk.size > 500 ? 500 : chunk.size), chunk.memory);
    }
    
    // 检查响应是否以'{'或'['开头（JSON特征）
    if (chunk.size > 0 && chunk.memory[0] != '{' && chunk.memory[0] != '[') {
        *output = malloc(100 + chunk.size);
        if (*output) {
            sprintf(*output, "UNKNOWN: Invalid response (not JSON). First 100 chars: %.*s", 
                    100, chunk.memory);
        } else {
            *output = strdup("UNKNOWN: Invalid response (not JSON)");
        }
        exit_code = 3;
        goto cleanup;
    }
    
    // 解析JSON
    root = json_loads(chunk.memory, 0, &error);
    if (!root) {
        *output = malloc(strlen("UNKNOWN: JSON parse error: ") + strlen(error.text) + 1);
        if (*output) {
            sprintf(*output, "UNKNOWN: JSON parse error: %s", error.text);
        } else {
            *output = strdup("UNKNOWN: JSON parse error");
        }
        
        // 添加位置信息
        char *tmp = malloc(strlen(*output) + 50);
        if (tmp) {
            sprintf(tmp, "%s at line %d, column %d", *output, error.line, error.column);
            free(*output);
            *output = tmp;
        }
        
        // 添加原始响应片段
        if (chunk.size > 0) {
            int start = error.position - 20;
            if (start < 0) start = 0;
            int length = 40;
            if (start + length > chunk.size) length = chunk.size - start;
            
            tmp = malloc(strlen(*output) + length + 50);
            if (tmp) {
                sprintf(tmp, "%s\nContext: %.*s", *output, length, chunk.memory + start);
                free(*output);
                *output = tmp;
            }
        }
        
        goto cleanup;
    }
    
    // 处理JSON
    exit_code = process_json(opts, root, output);
    
cleanup:
    // 清理资源
    if (chunk.memory) free(chunk.memory);
    if (root) json_decref(root);
    curl_global_cleanup();
    
    return exit_code;
}

// 解析命令行参数
int parse_args(int argc, char *argv[], Options *opts) {
    static struct option long_options[] = {
        {"hostname", required_argument, 0, 'H'},
        {"metric", required_argument, 0, 'M'},
        {"port", required_argument, 0, 'P'},
        {"warning", required_argument, 0, 'w'},
        {"critical", required_argument, 0, 'c'},
        {"units", required_argument, 0, 'u'},
        {"unit", required_argument, 0, 'n'},
        {"arguments", required_argument, 0, 'a'},
        {"token", required_argument, 0, 't'},
        {"timeout", required_argument, 0, 'T'},
        {"delta", no_argument, 0, 'd'},
        {"list", no_argument, 0, 'l'},
        {"verbose", no_argument, 0, 'v'},
        {"debug", no_argument, 0, 'D'},
        {"version", no_argument, 0, 'V'},
        {"queryargs", required_argument, 0, 'q'},
        {"secure", no_argument, 0, 's'},
        {"performance", no_argument, 0, 'p'},
        {0, 0, 0, 0}
    };
    
    int c;
    int option_index = 0;
    
    while ((c = getopt_long(argc, argv, "H:M:P:w:c:u:n:a:t:T:dlvDVq:sp", 
                          long_options, &option_index)) != -1) {
        switch (c) {
            case 'H':
                opts->hostname = strdup(optarg);
                break;
            case 'M':
                opts->metric = strdup(optarg);
                break;
            case 'P':
                opts->port = atoi(optarg);
                break;
            case 'w':
                opts->warning = strdup(optarg);
                break;
            case 'c':
                opts->critical = strdup(optarg);
                break;
            case 'u':
                opts->units = strdup(optarg);
                break;
            case 'n':
                opts->unit = strdup(optarg);
                break;
            case 'a':
                opts->arguments = strdup(optarg);
                break;
            case 't':
                opts->token = strdup(optarg);
                break;
            case 'T':
                opts->timeout = atoi(optarg);
                break;
            case 'd':
                opts->delta = 1;
                break;
            case 'l':
                opts->list = 1;
                break;
            case 'v':
                opts->verbose = 1;
                break;
            case 'D':
                opts->debug = 1;
                break;
            case 'V':
                opts->version = 1;
                break;
            case 'q':
                opts->queryargs = strdup(optarg);
                break;
            case 's':
                opts->secure = 1;
                break;
            case 'p':
                opts->performance = 1;
                break;
            default:
                return 1; // 解析错误
        }
    }
    
    // 验证参数
    if (opts->version) {
        printf("check_ncpa, Version %s\n", VERSION);
        return 2; // 特殊返回码表示需要退出
    }
    
    if (!opts->hostname) {
        fprintf(stderr, "Hostname is required\n");
        return 1;
    }
    
    if (!opts->metric && !opts->list) {
        fprintf(stderr, "No metric given, use --list to list all possible items\n");
        return 1;
    }
    
    if (opts->arguments && opts->metric && !strstr(opts->metric, "plugin")) {
        fprintf(stderr, "Arguments can only be used with custom plugins\n");
        return 1;
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    Options opts;
    init_options(&opts);
    
    int parse_result = parse_args(argc, argv, &opts);
    if (parse_result == 2) { // 版本请求
        free_options(&opts);
        return 0;
    } else if (parse_result != 0) {
        free_options(&opts);
        return 3; // 未知状态退出
    }
    
    char *output = NULL;
    int exit_code = run_check(&opts, &output);
    
    if (output) {
        printf("%s\n", output);
        free(output);
    }
    
    free_options(&opts);
    return exit_code;
}
