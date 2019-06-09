# Build instructions
------------------------------

## Requirements independent of platform
- CMake >= 3.3
- Python 3.6
- Qt 5.5.1
- Git
- Doxygen
- Graphviz
- Astyle 3.0.1

## Windows
### Requirements
- Windows >= 7
- Visual Studio 2013 Update 5 or Visual Studio 2015

### Steps
- Set the environment variable QTDIR to your Qt VS2013 lib folder, e.g. Qt/5.5/msvc2013_64/lib/cmake
- Start the CMake GUI
- Select the Royale folder and a build folder
- Hit configure and select your MSVC compiler
- Hit generate
- Open the resulting solution file in your build folder

#### Optional packages

Some parts of Royale require third-party SDKs for compilation, Royale's CMake scripts will
automatically detect these, and enable or disable the functionality.  For some of these items the
automatic detection will only run when configuring a clean build tree.

To support USB (Amundsen and Enclustra) devices, including the pico flexx, pico monstar, Animator
and pmdModules:

- Royale uses a host-side library from Cypress to talk to the Windows USB stack. The headers for
  that host-side library are part of the Cypress FX3 SDK, which is licensed based on whether you are
  using the associated hardware (the CX3/FX3 USB adaptor chip).
- The Cypress SDK can be downloaded from here : http://www.cypress.com/documentation/software-and-drivers/ez-usb-fx3-software-development-kit
- Install the Cypress SDK
- The installer should add an environment variable `FX3_INSTALL_PATH`
- After this configure a clean build tree in CMake and the CyAPI library should be found

To build the Matlab wrapper:

- Download the appropriate Matlab MCR from here : http://de.mathworks.com/products/compiler/mcr/
- Install the MCR

To build the Python wrapper:

- Install SWIG
- Install Python for Windows
- After this hit configure again in CMake and the SWIG and Python variables should show up

To view the markdown documentation in HTML:

- Install Pandoc

#### Troubleshooting

If the RoyaleDotNet tests are not showing up check the architecture (Test->Test Settings->Default Processor Architecture)
and rebuild RoyaleDotNet.

Depending on the CMake variable JENKINS_BUILD_TYPE, Windows builds will display a warning message when opening a camera device. The behavior is as follows:

- DEVELOPER (default) or DIRTY build type --> Dirty Warning on Windows
- RELEASE build type --> No Warning
- NIGHTLY build type --> version 0.0.0, Dirty Warning on Windows
- CMake variable EXCEPT_DIRTY_DEV == True --> No Warning if the current user has the username that compiled this build, otherwise behave according to build type


## Linux
### Requirements
- Ubuntu 16.04

### Steps
- Install latest CMake (https://cmake.org/download)
- Install the build-essential package (sudo apt-get update && sudo apt-get install build-essential)
- Install Freeglut (sudo apt-get install freeglut3-dev)
- Install additional Glut libraries (sudo apt-get install libxmu-dev libxi-dev)
- Install additional libusb libraries (sudo apt-get install libusb-1.0.0-dev)
- Install Qt 5.5 (http://www.qt.io/download-open-source/ )
- Set the environment variable QTDIR to point to your Qt libs, e.g. Qt/5.5/gcc64/lib/cmake
- Create a build folder
- Inside this build folder call CMake
    - cmake path_to_royale
- Call make

#### Optional packages

Some parts of Royale require third-party SDKs for compilation, Royale's CMake scripts will
automatically detect these, and enable or disable the functionality.  For some of these items the
automatic detection will only run when configuring a clean build tree.

To build the Matlab wrapper:
- Download the appropriate Matlab MCR from here : http://de.mathworks.com/products/compiler/mcr/
- Install the MCR
- Set the CMake variable Matlab_ROOT_DIR to the MCR path

To build the Python wrapper:
- Install SWIG (sudo apt-get install swig3.0)
- Install Python and libs (sudo apt-get install python3 libpython3-dev)
- After this the SWIG and Python variables should be set by CMake on the next run

The Cypress SDK is not needed on non-Windows platforms, as Royale uses libusb instead.

## Linux Arm cross compilation
### Requirements
- Raspbian Jessie >= May 2016 (target machine)
- Ubuntu >= 14.04 (host machine)

### Steps
- Install additional libusb libraries on target machine. ``sudo apt-get update && sudo apt-get install libusb-dev``
- Install latest CMake on host machine(https://cmake.org/download)
- Install the build-essential package on host machine ``sudo apt-get update && sudo apt-get install build-essential``
- Install the git package on host machine ``sudo apt-get install git``
- Install the cross compiler on the host machine by cloning the raspberry cross compiler toolchain. ``git clone https://github.com/raspberrypi/tools``
- Set the environment variable ``ARM_COMPILER_TOOLS`` to point to the tools directory from the last step, e.g. ``/usr/local/raspberry/tools``
- Create a directory to store copies of files from the target machine, e.g. ``/usr/local/raspberry/files``.
- Copy the following files from the target machine including their parent directories into the directory on the host machine created in the last step.
    - ``/usr/lib/arm-linux-gnueabihf/libusb-1.0.a``
    - ``/usr/lib/arm-linux-gnueabihf/libusb-1.0.so``
    - ``/usr/include/libusb-1.0/libusb.h``
    - ``/lib/arm-linux-gnueabihf/libudev.so.1.5.0``
    - ``/lib/arm-linux-gnueabihf/libudev.so.1``

  To do this in one step execute the following command on the target machine:
  ``rsync -avR  --copy-unsafe-links /lib/arm-linux-gnueabihf/libudev.so.1*  /usr/lib/arm-linux-gnueabihf/libusb-1.0.*  /usr/include/libusb-1.0/libusb.h <USER>@<HOSTMACHINE>:/usr/local/raspberry/files``
- Set the environment variable ``ARM_ENVIRONMENT_PATH`` on the host to point to the directory from the last steps, e.g. ``/usr/local/raspberry/files``
- Create a build folder
- Inside this build folder call CMake
  ``cmake -DCMAKE_TOOLCHAIN_FILE=/path_to_royale/cmake/toolchains/arm_linux.cmake path_to_royale``
- Call make

## Linux Arm 64 cross compilation
### Requirements
- Ubuntu >= 16.04 ARM 64 (target machine)
- Ubuntu >= 14.04 x86_64 (host machine)

### Steps
- Install additional libusb libraries on target machine. ``sudo apt-get update && sudo apt-get install libusb-dev``
- Install latest CMake on host machine(https://cmake.org/download)
- Install the build-essential package on host machine ``sudo apt-get update && sudo apt-get install build-essential``
- Install the git package on host machine ``sudo apt-get install git``
- Install the cross compiler on the host machine by downloading it from
  ``https://releases.linaro.org/components/toolchain/binaries/latest-5/aarch64-linux-gnu/gcc-linaro-5.4.1-2017.05-x86_64_aarch64-linux-gnu.tar.xz``
  and unpackinbg it with ``tar xJvf gcc-linaro-5.4.1-2017.05-x86_64_aarch64-linux-gnu.tar.xz``
- Set the environment variable ``ARM64_COMPILER_TOOLS`` to point to the compiler directory from the last step, e.g. ``/usr/local/arm64/tools/gcc-linaro-5.4.1-2017.05-x86_64_aarch64-linux-gnu``
- Create a directory to store copies of files from the target machine, e.g. ``/usr/local/arm64_environment``.
- Copy the following files from the target machine including their parent directories into the directory on the host machine created in the last step.
    - ``/usr/lib/aarch64-linux-gnu/libusb-1.0.a``
    - ``/usr/lib/aarch64-linux-gnu/libusb-1.0.so``
    - ``/usr/include/libusb-1.0/libusb.h``
    - ``/lib/aarch64-linux-gnu/libudev.so.1.6.4``
    - ``/lib/aarch64-linux-gnu/libudev.so.1``

  To do this in one step execute the following command on the target machine:
  ``rsync -avR  --copy-unsafe-links /lib/aarch64-linux-gnu/libudev.so.1*  /usr/lib/aarch64-linux-gnu/libusb-1.0.*  /usr/include/libusb-1.0/libusb.h <USER>@<HOSTMACHINE>:/usr/local/arm64_environment``
- Set the environment variable ``ARM64_ENVIRONMENT_PATH`` on the host to point to the directory from the last steps, e.g. ``/usr/local/arm64_environment``
- Create a build folder
- Inside this build folder call CMake
  ``cmake -DCMAKE_TOOLCHAIN_FILE=/path_to_royale/cmake/toolchains/arm64_linux.cmake path_to_royale``
- Call make

## Mac OS X
### Requirements
- OSX >= 10.11

### Steps
- Install latest CMake (https://cmake.org/download)
- Install Xcode
- Install Freeglut (only for compiling rawviewer) (https://www.macports.org)
- Install Qt 5.5 (http://www.qt.io/download-open-source/ )
- Set the environment variable QTDIR to point to your Qt libs
- Start the CMake GUI
- Select the Royale folder and a build folder
- Hit configure and select Xcode project
- Hit generate
- Open the resulting solution file in Xcode

## Android ARM
### Requirements
- Ubuntu 14.04

### Steps
- Install latest CMake (https://cmake.org/download)
- Install the build-essential package (sudo apt-get update && sudo apt-get install build-essential)
- Install Freeglut (sudo apt-get install freeglut3-dev)
- Install Ant (sudo apt-get install ant)
- Set the environment variable ANT to point to your ant installation, e.g. /usr/bin/ant
- Install additional libusb libraries (sudo apt-get install libusb-1.0.0-dev)
- Install additional Glut libraries (sudo apt-get install libxmu-dev libxi-dev)
- Install Qt 5.5 (http://www.qt.io/download-open-source/ ) (there you also have to select the Android ARM packages!)
- Set the environment variable QTDIR to point to your Android ARM Qt libs, e.g. Qt/5.5/android_armv7/lib/cmake
- Install the Android SDK (http://developer.android.com/sdk/index.html#Other)
- Set the environment variable ANDROID_SDK to your SDK folder
- Install the desired API levels with the Android SDK manager
- Install the Android NDK (http://developer.android.com/ndk/downloads/index.html)
- Set the environment variable ANDROID_NDK to your NDK folder
- Install OpenJDK (sudo apt-get install openjdk-7-jdk)
- Set the environment variable JAVA_HOME to your OpenJDK folder, e.g. /usr/lib/jvm/java-7-openjdk-amd64
- Create a build folder
- Inside this build folder call CMake

cmake -DCMAKE_TOOLCHAIN_FILE=/path_to_royale/contrib/qt-android-cmake-master/toolchain/android.toolchain.cmake -DANDROID_ABI="armeabi-v7a" path_to_royale

- Call make

### Replacing libroyale.so in RoyaleViewer.apk without rebuilding the Qt or Java parts

Building RoyaleViewer.apk requires compatible versions of the Android SDK, Android NDK and Qt SDK;
in this case simply choosing the latest version of all of the dev kits doesn't work.  Additionally,
a build may also need network access for Gradle.

If you can build libroyale.so, the instructions in `tools/royaleviewer/misc/README_Android.md` about
replacing a library work for libroyale.so, even if you can't build the Java or Qt parts.

## Android x86_64
### Requirements
- Ubuntu 14.04

### Steps
- Install latest CMake (https://cmake.org/download)
- Install the build-essential package (sudo apt-get update && sudo apt-get install build-essential)
- Install Freeglut (sudo apt-get install freeglut3-dev)
- Install Ant (sudo apt-get install ant)
- Set the environment variable ANT to point to your ant installation, e.g. /usr/bin/ant
- Install additional Glut libraries (sudo apt-get install libxmu-dev libxi-dev)
- Install Qt 5.5 (http://www.qt.io/download-open-source/ ) (there you also have to select the Android x86 packages!)
- Set the environment variable QTDIR to point to your Android x86 Qt libs, e.g. Qt/5.5/android_x86/lib/cmake
- Install the Android SDK (http://developer.android.com/sdk/index.html#Other)
- Set the environment variable ANDROID_SDK to your SDK folder
- Install the desired API levels with the Android SDK manager
- Install the Android NDK (http://developer.android.com/ndk/downloads/index.html)
- Set the environment variable ANDROID_NDK to your NDK folder
- Install OpenJDK (sudo apt-get install openjdk-7-jdk)
- Set the environment variable JAVA_HOME to your OpenJDK folder, e.g. /usr/lib/jvm/java-7-openjdk-amd64
- Create a build folder
- Inside this build folder call CMake

cmake -DCMAKE_TOOLCHAIN_FILE=/path_to_royale/contrib/qt-android-cmake-master/toolchain/android.toolchain.cmake -DANDROID_ABI="x86_64" path_to_royale

- Call make
