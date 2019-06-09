# Sign instructions
------------------------

## Sign installers

To sign the installer you need the following parts:

1. The etoken usb stick with our certificate
2. The driver software software for the etoken. You can find the driver in our
   intranet under `P:\Royale\4_Entwicklung\Tools\Sign\etokenDrivers`, or you can
   download it under https://support.globalsign.com/customer/portal/articles/1698654-safenet-drivers
3. A Windows SDK that contains the signtool.exe. **Attention, if you use Windows 7,
   you must not use the Windows 10 SDK, because signtool.exe from Win 10 does not work
   properly on Windows 7.** You can find a suitable Windows 8 SDK under `P:\Royale\4_Entwicklung\Tools\Sign\Windows Kits\8.0\StandaloneSDK`.

To prepare a machine for signing do:

1. Install the Windows SDK.
2. Install the etoken drivers.
3. Connect the etoken.
4. Configure a Royale build with `cmake`
5. Check if `cmake` found the `signtool.exe`. It should print a message that it found `signtool.exe`

Now you can build the Royale package. After the package build just call SignSetup.bat. The script
will call signtool with corerct parameters. The script will create 2 signatures for the Installer,
so it will ask you for the key password 2 times. If you want get rid of the frequent passwort
questions, you can open `SafeNet Authentication Client` through a double click on the symbol in the
task bar. Click on "Advanced View" (gear-wheel symbol) and choose "Client Settings". Go to "Advanced",
select "Enable single logon" and click on "Save". Log off and log on your user account. You will now
be asked for the password only once per session.

## Sign drivers

To sign the driver you need the same parts neccessary for installer signing, and: 

1. The Windows 10 driver development kit (WDK). You can find the Windows 10 WDK under
   `P:\Royale\4_Entwicklung\Tools\Sign\Windows Kits\10\WDK`.
2. The "R1 cross signing certificate". You can find the certificate in the Royale repository under `cmake\sign\r1cross.cer`

To prepare a machine for driver signing do:

1. Install the Windows SDK.
2. Install the Windows 10 WDK
3.  Install the R1 cross signing certificate.
4. Install the etoken drivers.
5. Connect the etoken.
6. Configure a Royale build with `cmake`
7. Check if `cmake` found the `signtool.exe`. It should print a message that it found `signtool.exe`
8. Check if `cmake` found the `inf2cat.exe`. It should print a message that it found `inf2cat.exe`

To create and sign new drivers do:

1. Adapt the driver .inf files to your needs.
2. Call `CreateCatFiles.bat` from the Royale build directory. This will create new .cat files for
   each windows platform.
3. Call `SignDriver.bat` from the Royale build directory. This will sign the drivers.
4. Add and commit the .cat and .inf files to git.
