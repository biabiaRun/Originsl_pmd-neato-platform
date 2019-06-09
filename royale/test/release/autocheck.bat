@echo off
if "%1" equ "-clean" (
  if exist build rmdir build /s /q
  if exist bin rmdir bin /s /q
  if exist release (
    for /d %%a in (release\*) do rmdir %%a /s /q
  )
  if exist release rmdir release /s /q

  if exist "..\behavior\build" (
    for /d %%a in (..\behavior\build\*) do rmdir %%a /s /q
  )
  if exist "..\behavior\build" rmdir "..\behavior\build" /s /q
  goto :eof
)

if "%ROYALE_VERSION_BUILD%" equ "" (
  echo ERROR Build version not defined - set it with "set ROYALE_VERSION_BUILD=number"
  goto :eof
)

if not exist "build" mkdir build
if not exist "bin" mkdir bin
  if exist release (
    for /d %%a in (release\*) do rmdir %%a /s /q
  )
if exist bin\ReleaseAnalyzer_LOG.txt del bin\ReleaseAnalyzer_LOG.txt

if "%ROYALE_RELEASE_TEST_PLATFORM%"=="X64" (
  if "%ROYALE_RELEASE_TEST_VS_VERSION%"=="14.0" (
    set ROYALE_VS_VERSION=Visual Studio 14 2015 Win64
    set CMAKE_TMP_GENERATOR=-G "Visual Studio 14 2015 Win64"
  )
  if "%ROYALE_RELEASE_TEST_VS_VERSION%"=="12.0" (
    set ROYALE_VS_VERSION=Visual Studio 12 2013 Win64
    set CMAKE_TMP_GENERATOR=-G "Visual Studio 12 2013 Win64"
  )
  if "%ROYALE_RELEASE_TEST_VS_VERSION%"=="11.0" (
    set ROYALE_VS_VERSION=Visual Studio 11 2012 Win64
    set CMAKE_TMP_GENERATOR=-G "Visual Studio 11 2012 Win64"
  )
)

if "%1" equ "-compileonly" (
  cd build
  cmake %CMAKE_TMP_GENERATOR% -DROYALE_ENABLE_PANDOC=OFF -DROYALE_DO_VERIFICATION=ON -DROYALE_ENABLE_RELEASEANALYZER=ON ../../../ | call ../wtee.bat ../bin/ReleaseAnalyzer_LOG.txt
  cmake --build . --config Release
cd ..
  goto :eof
)

if "%1" equ "-buildExpectationFiles" (
  cd build
  cmake %CMAKE_TMP_GENERATOR% -DROYALE_ENABLE_PANDOC=OFF -DROYALE_DO_VERIFICATION=ON -DROYALE_ENABLE_RELEASEANALYZER=ON ../../../ | call ../wtee.bat ../bin/ReleaseAnalyzer_LOG.txt
  cmake --build . --config Release | call ../wtee.bat ../bin/ReleaseAnalyzer_LOG.txt +
  cd ../bin
  releaseanalyzer.exe -settingsFile settings.xml -usecreateconfig "WINDOWS-32Bit"
  releaseanalyzer.exe -settingsFile settings.xml -usecreateconfig "WINDOWS-64Bit"
  releaseanalyzer.exe -settingsFile settings.xml -usecreateconfig "APPLE-64Bit"
  releaseanalyzer.exe -settingsFile settings.xml -usecreateconfig "ANDROID-32Bit"
  releaseanalyzer.exe -settingsFile settings.xml -usecreateconfig "LINUX-64Bit"
  releaseanalyzer.exe -settingsFile settings.xml -usecreateconfig "LINUX-32Bit"
  cd ..
  goto :eof
)

cd build
cmake %CMAKE_TMP_GENERATOR% -DROYALE_ENABLE_PANDOC=OFF -DROYALE_DO_VERIFICATION=ON -DROYALE_ENABLE_RELEASEANALYZER=ON ../../../ | call ../wtee.bat ../bin/ReleaseAnalyzer_LOG.txt
cmake --build . --config Release | call ../wtee.bat ../bin/ReleaseAnalyzer_LOG.txt +
cd ../bin
releaseanalyzer.exe -settingsFile settings.xml
cd ..
