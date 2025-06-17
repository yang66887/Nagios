@echo off
@chcp 437 >NUL

set arg=%1
if %arg%==os (
    wmic os get Caption|findstr /V Caption
)
if %arg%==kernel (
    wmic os get Version|findstr /V Version
)
if %arg%==cpu (
    setlocal enabledelayedexpansion
    set "total=0"
    set "has_data=0"
    
    for /f "tokens=2 delims==" %%a in (
        'wmic path Win32_Processor get NumberOfLogicalProcessors /value 2^>nul ^| findstr "="'
    ) do (
        set "core_count=%%a"
        set /a "total+=core_count" >nul 2>&1
        set "has_data=1"
    )
    
    if !has_data! equ 0 (
        for /f "skip=1 tokens=*" %%a in (
            'wmic cpu get NumberOfLogicalProcessors /format:list 2^>nul'
        ) do (
            set "line=%%a"
            set "line=!line:NumberOfLogicalProcessors=!"
            set "line=!line: =!"
            
            if "!line!" neq "" (
                set /a "total+=!line!" >nul 2>&1
                set "has_data=1"
            )
        )
    )
    
    if !has_data! equ 0 (
        for /f "skip=1 tokens=*" %%a in (
            'wmic cpu get NumberOfLogicalProcessors 2^>nul ^| findstr "[0-9]"'
        ) do (
            set "line=%%a"
            set "line=!line: =!"
            
            if "!line!" neq "" (
                set /a "total+=!line!" >nul 2>&1
                set "has_data=1"
            )
        )
    )
    
    if !has_data! equ 1 (
        <nul set /p "=!total!"
    )
    endlocal
)
