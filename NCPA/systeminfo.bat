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
    wmic cpu get numberoflogicalprocessors|findstr /V Number
)
