HOW TO TEST ON WINDOWS:

 * Connect the tablet via USB to your comupter (make sure both are in the same wifi)
 
 * Create a folder "libs" in the same location AndroidBDDControl.exe is located
 
 * Copy the following binaries into this libs folder: 
    * libusb_android.so
    * libroyale.so
    * libtest_royale_bdd.so
    
 * Enter the tablets IP in the designated text box
 
 * Click "Check ADB" to see if your device is connected via ADB
 
 * Click "Enable TCP/IP" to restart ADB in WiFi mode
 
 * Click "Connect TCP/IP" to connect ADB via WiFi (another "Check ADB" should show 2 devices now)
 
 * Remove the USB connection and Connect the camera to tablet
 
 * Click "Start Test" and wait until it is finished (you should read "done" in the log window)
 
 * Click "Fetch Results" -> the test results are now present as txt file in the same directory as the exe to check a new compilation just copy the .so files into the folder testing/bdd/AndroidBddRunner/libs and run again