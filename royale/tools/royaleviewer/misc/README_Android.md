Installing RoyaleViewer
-----------------------

To install the application on your Android device, use adb:

adb install -r ./RoyaleViewer.apk

For information: http://developer.android.com/tools/help/adb.html

Replacing a library
-------------------

If you do not know what this section is about, you don't need it.

The APK includes libraries, including the Gnu C++ STL runtime, Qt, LibUSB and (in some versions)
LibUVC.  These libraries can be replaced without needing to rebuild the other components, provided
that the replacement is ABI-compatible.

Modifying the APK file doesn't need root access, and it can then easily be installed on multiple
devices.  If you only want to use the replacement library on a few devices, and those devices have
been rooted, it may be easier to overwrite the library's .so file on the devices' filesystems.

Android APKs are specially-formatted and signed Zip files. The signing part must be done with your
own keystore (which is used by the Android SDK for normal APK builds).

* Unzip the original APK to a temporary directory (preserving the subdirectory structure)
* Find the .so file to replace, in `libs/armeabi-v7a` (or the 64-bit equivalent)
* Replace that library with its new version
* Build the new APK with jar and jarsigner

Example arguments for jar and jarsigner, assuming a command shell in the root of the unzip'd
original APK:

    jar cf ../rezipped.apk *
    jarsigner -keystore KEYSTORE -storepass PASSWORD ../rezipped.apk ALIAS
