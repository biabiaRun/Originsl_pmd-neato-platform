# Introduction

**NOTICE**: **Normally you should NOT run the releaseanalyzer via console directly; it is part of the release autobuild
and validation process which is controlled by a batch script "autocheck" in testing/releaseanalyzer which controls and
executes the complete necessary workflow; it is NOT planned to start any actions manually**

The **Releaseanalyzer** provides the functionality for checking royale release packages automatically - therefore
it manages the unpacking of .tar/.tar.gz .zip and bz2 archives.

The files and the structure are checked before and during decompression; so that no files are lost during
decompression and the disk structure is furthermore checked with an expectation file after decompression.

Releaseanalyzer currently supports the following platforms:

- Windows 7/8
- Linux
- Mac

Releaseanalyzer operates in a three step mode; these three steps are described as single shots
however step 2 and step 3 can be combined

By executing: *releaseanalyzer.exe -help* the following help is provided:
<code>releaseanalyzer.exe -help</code>
<pre><code>
**  Copyright(C) 2015 Infineon Technologies                              **
**  All rights reserved.                                                 **
</code></pre>
<pre><code>
-settingsFile "filename" .................... Can be used to provide an XML Settings file
-createReference "unpack-path" "filename" ... creates a reference file from the given directory which has to be used for -validationFileName
-7zPath "path"............................... specifies the path to 7zip to manage the control flow via 7zip (needed in case of NSIS packages)
-package "package-path" ..................... specify the complete path to the package which shall be extracted
-unpackpath "unpack-path" ................... specify the path to which the package (-package) shall be extracted
-checkPath "check-path" ..................... specify the directory tree that shall be parsed for comparing with the expectation file;
                                              might differ from -unpackpath
-ignorepathandsubs "path" ................... adds a path to the ignorelist for checking completeness (might be given multiple times); doc
                                              directory might be a case; given directories are ignored during creation of referenceFile
-compileSamples ............................. specify if the samples of the pmd package shall be configured and compiled and checked
-compileBDD ................................. specify if BDD shall be configured and compiled and linked against the delivered library of pmd
-runBDD ..................................... specify if the BDD tests shall be run automatically after compiling and linking against the pmd
                                              delivered library
-validationFileName "fileName" .............. specify the filename which shall be used to ensure completeness after unpacking
-differenceFile "fileName" .................. specifies the filename the unexpected differences between the file structure and the expectation
                                              file
-debug ...................................... turn debug switch on; this is for human execution - automatic execution must not use that feature
-fullautomatic .............................. specify a full automatic run; useful if you already plugged in a camera otherwise a notification
                                              might come in handy
-usecreateconfig <config> ................... Reads specific config from settingsfile, useful to generate expectationfiles
</code></pre>
<pre><code>
Additional Information:
[ ].......................................... means this workflow task was planned to be executed, but for any unknown reason it wasn't
[X].......................................... means this workflow task was executed successfully
[I].......................................... Information for the user to keep track of the program execution path
[W].......................................... means there is a warning; something the user should know about
[E].......................................... means this workflow task caused any unknown failure
[~].......................................... means the workflow caused a serious error; any state was entered unplanned!

</code></pre>

## Step 1 Creating a reference file
The reference file has to be created for each platform separately and is necessary for checking the integrity after unpacking later
the following command is an example of how to create such a reference file; the reference file is always placed in the root directory
of the executable.

<pre><code>
 releaseanalyzer.exe
   -createReference "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit" expectedStructureLINUX64.txt
</code></pre>

## Step 2 Unpacking and checking the a package
In this step the original package is decompressed, while the decompression algorithm is chosen automatically. While doing the
decompression of an archive file two files are written and compared after finishing the unpacking. One file is generated while
reading from the archive directly while the other file includes the files that have really been written on the disc. These two
files are compared later on to ensure that all files have been decompressed; errors are reported on the fly during workflow printout
in debug mode.
<pre><code>releaseanalyzer.exe
  -package "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit.tar.gz"
  -unpackpath "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit"
  -checkPath "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit"
  -debug
</code></pre>

## Step 3 Validation against expectation
After the package has been unpacked it can be validated against the expectation with an expectation file (this is generally
the file that was generated in step 1; without the directories which are not necessary to check) see the example below:
<pre><code>releaseanalyzer.exe
  -validationFileName expectedStructureLINUX64.txt
  -checkPath "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit"
  -ignorepathandsubs doc
  -debug
</code></pre>

## Combination of Step 2 and 3
The above described steps 2 and 3 can be combined with the following command:
<pre><code>releaseanalyzer.exe
  -package "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit.tar.gz"
  -unpackpath "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit"
  -checkPath "C:/Festplatte_D/DEV Tests/libroyale_0.8.0/libroyale-0.8.0-LINUX-64Bit"
  -validationFileName expectedStructureLINUX64.txt
  -ignorepathandsubs doc
  -debug
</code></pre>

The combination of step 2 and 3 can also be achieved by using a separate settingsFile; the format is specified by the settings.xsd file
<code>C:\Festplatte_D\RoyaleGit\build\Debug>releaseanalyzer.exe -settingsFile settings.xml</code>

## Additional Information of internals

### Generate expectation files for specific platforms
To generate an expectation file for a specific platform from a given package you shall
place the SDK package in the directory structure provided by the settings.xml in the value "packagePath".
Afterwards compile releaseanalyzer only without autorun with "autocheck.sh compileonly" and run it afterwards
with "bin\releaseanalyzer -settingsFile settings.xml -usecreateconfig <PLATFORMNAME>" while <PLATFORMNAME> is one of
the names in the settings.xml (in the tag Platform for attribute PlatformName) - the expectation file is placed in
testing\releaseanalyzer\expectationFiles.

### How do autoscripts / batches operate
1. Place a release package for the current platform in testing/releaseanalyzer/release/
2. The autoscripts are cleaning up the created folders (you have to pass "clean" to the autocheck.sh to achieve a cleanup) and
   recreating them.
3. The autoscript is then building the whole source tree with the releaseanalyzer which is not existing at the time the autocheck was
   started
4. The releaseanalyzer is started and checks the release (by parsing the directory for the correct platform) against the expectation 
   if there were errors according to the expectation the script continues, otherwise the error reporting is done and the script exits
5. The releaseanalyzer copies the royale library to testing/bdd/lib
6. The releaseanalyzer compiles and links the BDD tests against the library in testing/bdd/lib
7. The executable and the needed libraries are copied to testing/releaseanalyzer/bin
8. Finally the BDD tests are automatically executed (if -fullautomatic was given via command line or settingsfile)

### Where is Releaseanalyzer operating?
The Releaseanalyzer is furthermore an automated cmake configuration and compilation tool to unpack a given releasepackage, compile
the samples.
Later on it copies the royale library to a fixed path (testing/bdd/lib) and creates a build folder in testing/bdd (testing/bdd/build)
where the bdd project is compiled and linked again the royale library in testing/bdd/lib.
After the compilation of the bdd tests the executable for running the BDD tests is copied to testing/releaseanalyzer/bin.
