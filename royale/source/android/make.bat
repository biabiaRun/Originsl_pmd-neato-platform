:: ---------------------------------------------------------------------- ::
:: Copyright (C) 2018 pmdtechnologies ag                                  ::
::                                                                        ::
:: THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY ::
:: KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE    ::
:: IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A             ::
:: PARTICULAR PURPOSE.                                                    ::
:: ---------------------------------------------------------------------- ::

:: initialize and build the jroyale project
:: helpful to not copy all dependencies into the project

@echo on

IF EXIST .temp DEL /F /Q .temp

:: find the libroyale folders and store them into variables

DIR /B | FINDSTR \libroyale-[0-9.]*-ANDROID-arm-32Bit$ >> .temp
SET /P LIBROYALE_ANDROID_ARM_32BIT=<.temp
DEL /F /Q .temp

DIR /B | FINDSTR \libroyale-[0-9.]*-ANDROID-arm-64Bit$ >> .temp
SET /P LIBROYALE_ANDROID_ARM_64BIT=<.temp
DEL /F /Q .temp

:: copy the content needed to build the android library
:: into the right project location

XCOPY ".\%LIBROYALE_ANDROID_ARM_32BIT%\bin\libroyale.so"      /Y ".\JRoyale\jroyale\src\main\jniLibs\armeabi-v7a\"                | EXIT -1
XCOPY ".\%LIBROYALE_ANDROID_ARM_32BIT%\bin\libspectre.so"    /Y ".\JRoyale\jroyale\src\main\jniLibs\armeabi-v7a\"                | EXIT -1
XCOPY ".\%LIBROYALE_ANDROID_ARM_32BIT%\bin\libusb_android.so" /Y ".\JRoyale\jroyale\src\main\jniLibs\armeabi-v7a\"                | EXIT -1
XCOPY ".\%LIBROYALE_ANDROID_ARM_32BIT%\include"               /Y ".\JRoyale\jroyale\src\main\jniLibs\armeabi-v7a\include\"        | EXIT -1
XCOPY ".\%LIBROYALE_ANDROID_ARM_32BIT%\include\royale"        /Y ".\JRoyale\jroyale\src\main\jniLibs\armeabi-v7a\include\royale\" | EXIT -1

XCOPY ".\%LIBROYALE_ANDROID_ARM_64BIT%\bin\libroyale.so"      /Y ".\JRoyale\jroyale\src\main\jniLibs\arm64-v8a\"                | EXIT -1
XCOPY ".\%LIBROYALE_ANDROID_ARM_64BIT%\bin\libspectre.so"    /Y ".\JRoyale\jroyale\src\main\jniLibs\arm64-v8a\"                | EXIT -1
XCOPY ".\%LIBROYALE_ANDROID_ARM_64BIT%\bin\libusb_android.so" /Y ".\JRoyale\jroyale\src\main\jniLibs\arm64-v8a\"                | EXIT -1
XCOPY ".\%LIBROYALE_ANDROID_ARM_64BIT%\include"               /Y ".\JRoyale\jroyale\src\main\jniLibs\arm64-v8a\include\"        | EXIT -1
XCOPY ".\%LIBROYALE_ANDROID_ARM_64BIT%\include\royale"        /Y ".\JRoyale\jroyale\src\main\jniLibs\arm64-v8a\include\royale\" | EXIT -1

:: call gradle to build the android library

CD ".\JRoyale\"
CALL .\gradlew :jroyale:assemble --stacktrace | EXIT -1
CD ".\.."

:: copy the result from gradle into the build directory

XCOPY  ".\JRoyale\jroyale\build\outputs\aar\jroyale-release.aar" /Y ".\build\" | EXIT -1
XCOPY  ".\JRoyale\README.md"                                     /Y ".\build\" | EXIT -1
