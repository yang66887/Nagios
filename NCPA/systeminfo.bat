@echo off
@chcp 437 >NUL

set arg=%1
if %arg%==system (
    set key=Name:
    systeminfo.exe|findstr /B OS|findstr Name:
)
if %arg%==kernel (
    set key=Version:
    systeminfo.exe|findstr /B OS|findstr Version:
)
if %arg%==cpu (
    wmic cpu get numberoflogicalprocessors|findstr /V Number
)