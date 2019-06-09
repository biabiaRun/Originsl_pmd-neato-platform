# Information

v0.0.1 - 2015-08-04
-------------------

Place all packages (extracted from the main zip) in dir release (if not existing; create it);
if you need to you can also create a new package on your current platform by using the command cpack in the main build directory.

NOTICE: PLEASE EXECUTE THE FOLLOWING LINES **exactly** AS DESCRIBED BELOW!!!
        THIS RELEASEANALYZER REQUIRES THAT **test_royale_bdd** can be run from the console

FOR WINDOWS:
- Make sure you have 7Zip installed in the following Path "C:\Program Files\7-Zip" (in this path you should have the *.exe located "7z.exe") if
  the delivered library is an NSIS package. Make sure you have at least 7-Zip 9.38 beta (03.01.2015) for Windows (http://www.7-zip.de/)
- ALWAYS Use the VS console for execution
- Make sure your repository is placed in a short path, with less than 260 characters (otherwise you might get errors during execution later)

BEFORE YOU START:
- Make sure that the repository from which you got the source code has the same version as the package that you are trying
  to verify (after you got a release, make sure you updated your repository!)
- Make sure your Qt bin dir is in the path (QTDIR as to be set to the correct folder and PATH shall include %QTDIR%\bin)
- Make sure you can start test_royale_bdd

1. Create directory "release"
2. Copy platform dependent compressed releasearchives in there
3. Set the build version number (example: 1.0.0.XXX) to the environment variables (ROYALE_VERSION_BUILD=<BUILDNO> -> example: ROYALE_VERSION_BUILD=29)
4. Execute autocheck (for windows use autocheck.bat and for unix you can use autocheck.sh)
5. Check output and Logfile in bin/ReleaseAnalyzer_LOG.txt (however at the end you should get a report)
6. For cleaning you can run the autocheck in clean mode ("autocheck.bat -clean" / "autocheck.sh -clean")

# APPENDIX

## How to ONLY build releaseanalyzer
To build only releaseanalyzer without runnung the complete process you can use the convenience function
"autocheck.sh -compileonly" this will configure releaseanalyzer properly and compile it

## How to generate expectation files
### 1. Generate expectation files for specific platforms
To generate an expectation file for a specific platform from a given package you shall
place the SDK package in the directory structure provided by the settings.xml in the value "packagePath".
Afterwards compile releaseanalyzer only without autorun with "autocheck.sh compileonly" and run it afterwards
with "bin\releaseanalyzer -settingsFile settings.xml -usecreateconfig <PLATFORMNAME>" while <PLATFORMNAME> is one of
the names in the settings.xml (in the tag Platform for attribute PlatformName) - the expectation file is placed in
testing\releaseanalyzer\expectationFiles.

### 2. Generate expectation files for all platforms
To generate expectation files for all platforms from a given packages you shall
place the SDK packages in the directory structure provided by the settings.xml in the value "packagePath".
Afterwards run "autocheck.sh -buildExpectationFiles" it automatically generates the platform files.

## How to Build a compressed platform package
### 1. Configure project
Configure your project by directing to the main directory and making the build directory "mkdir build"
Browse to the build directory via "cd build" and execute the configuration "cmake ../"

### 2. Build the royale project
FOR WINDOWS: Use "cmake --build . --config Release" in the build directory you should already be in
FOR other Platforms: Use "cmake --build ." in the build directory you should already be in

### 3. Generate the Package
FOR WINDOWS: Make sure you have NSIS installed (at least version 3.x)
execute "cpack" in the main build directory

The generated package can be used for verification of the releaseanalyzer and for creating referenceFiles.
