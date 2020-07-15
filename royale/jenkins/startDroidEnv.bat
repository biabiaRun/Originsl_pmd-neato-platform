:: ****************************************************************************
:: Copyright (C) 2019 pmdtechnologies ag
::
:: THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
:: KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
:: IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
:: PARTICULAR PURPOSE.
::
:: ****************************************************************************

@echo off

set SKRIPTDIR=%~dp0
setlocal ENABLEDELAYEDEXPANSION
set WORKDIR=%cd%

:: Parse server and image name from jenkins script
for /F "tokens=2 delims==" %%G IN ('findstr /I "dockerRegistry=" "%SKRIPTDIR%android_arm32.groovy"') DO set "DOREG=%%G"
set "DOREG=!DOREG:'=!"
for /F "tokens=2 delims==" %%G IN ('findstr /I "dockerImage=" "%SKRIPTDIR%android_arm32.groovy"') DO set "DOIMG=%%G"
set "DOIMG=!DOIMG:'=!"
set "IMAGE=%DOREG%/%DOIMG%"

:: Parse commandline parameters
set "ENVIRONMENT="
set "VERBOSE="
:loop
IF NOT [%1]==[] (
    IF [%1]==[--workdir] (
        SET WORKDIR=%~f2
        SHIFT /1
		SHIFT /1
		GOTO :loop
    )
    IF [%1]==[-w] (
        SET WORKDIR=%~f2
        SHIFT /1
		SHIFT /1
		GOTO :loop
    )
    IF [%1]==[--image] (
        SET "IMAGE=%2"
        SHIFT /1
		SHIFT /1
		GOTO :loop
    )
    IF [%1]==[-i] (
        SET "IMAGE=%2"
        SHIFT /1
		SHIFT /1
		GOTO :loop
    )

    IF [%1]==[--verbose] (
        SET "VERBOSE=true"
        SHIFT /1
		GOTO :loop
    )
    IF [%1]==[-v] (
        SET "VERBOSE=true"
        SHIFT /1
		GOTO :loop
    )
    IF [%1]==[-e] (
		set "VALNAME=%2"
		if !VALNAME:~0^,1!!VALNAME:~-1! equ "" (
			SET "ENVIRONMENT=%ENVIRONMENT%-e %2 "
		) else (
			SET "ENVIRONMENT=%ENVIRONMENT%-e "%2^=%~3" "
			SHIFT /1
		)
        SHIFT /1
		SHIFT /1
		GOTO :loop
    )
    IF [%1]==[--] (
        SET WORKDIR=%~f2
        SHIFT /1
		GOTO :BREAK
    )
    GOTO :BREAK
)
:BREAK
set PARAMETERS=
:loext
IF NOT [%1]==[] (
    SET "PARAMETERS=%PARAMETERS%%1 "
	SHIFT /1
    GOTO :loext
)
:: Rewrite WORKDIR to Unix style TargetDir
set "WORKDIR=!WORKDIR:\=/!"
set "TARGETDIR=!WORKDIR::=_!"
set "TARGETDIR=!TARGETDIR: =_!"
:: Copy proxy variables from environment variables if not defined on commandline
if "!ENVIRONMENT:HTTP_PROXY=!"=="!ENVIRONMENT!" (
	if DEFINED HTTP_PROXY SET "ENVIRONMENT=%ENVIRONMENT%-e "HTTP_PROXY=%HTTP_PROXY%" "
)
if "!ENVIRONMENT:HTTPS_PROXY=!"=="!ENVIRONMENT!" (
	if DEFINED HTTPS_PROXY SET "ENVIRONMENT=%ENVIRONMENT%-e "HTTPS_PROXY=%HTTPS_PROXY%" "
)
if "!ENVIRONMENT:NO_PROXY=!"=="!ENVIRONMENT!" (
	if DEFINED NO_PROXY SET "ENVIRONMENT=%ENVIRONMENT%-e "NO_PROXY=%NO_PROXY%" "
)
if "!ENVIRONMENT:JAVA_TOOL_OPTIONS=!"=="!ENVIRONMENT!" (
	if DEFINED JAVA_TOOL_OPTIONS SET "ENVIRONMENT=%ENVIRONMENT%-e "JAVA_TOOL_OPTIONS=%JAVA_TOOL_OPTIONS%" "
)

:: Check if image is set
IF [%IMAGE%]==[] (
    echo "Missing image name"
    exit /b 1
)

if DEFINED VERBOSE echo ON
docker run -it --rm -w "/%TARGETDIR%" -v "%WORKDIR%:/%TARGETDIR%:rw,z" %ENVIRONMENT% "%IMAGE%" %PARAMETERS%
