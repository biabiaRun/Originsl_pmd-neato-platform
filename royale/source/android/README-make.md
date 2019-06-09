How to make the jroyale-release.aar
===================================

Nessesary tools are:
- Java JDK 1.8 or heigher
- Android SDK 27+
- NDK   (tested on 16b and 17)
- CMake (tested on 3.4.3 and 3.7.2)

To build jroyale you have to place the royale android builds inside this folder and unzip them.
Therefor it is nessesary that both 32Bit an 64Bit are included.

E.g.
unzip libroyale-3.13.0.47-ANDROID-arm-32Bit.zip into libroyale-3.13.0.47-ANDROID-arm-32Bit
and
unzip libroyale-3.13.0.47-ANDROID-arm-64Bit.zip into libroyale-3.13.0.47-ANDROID-arm-64Bit
into this folder.

Afterwords run make.bat.
The result of the build will be found in the folder build.
