@echo off
@chcp 437 >NUL

set arg=%1
if %arg%==os (
    systeminfo.exe|findstr /B OS|findstr "Name: Version:"
)
if %arg%==cpu (
    wmic cpu get numberoflogicalprocessors|findstr /V Number
)
