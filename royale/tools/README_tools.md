Tools provided with the Royale source
=====================================

A few categories are separated in to their own sections.

* [Storage Tools](#storage), which read and write calibration data or boot images
* [Deprecated](#deprecated)

each\_camera
------------

Royale's tools and sample applications typically use the first camera that CameraManager returns.
`each_camera` is a test-automation tool for connecting several USB cameras to a PC, and then
selecting which camera will be opened by the other application.

It works by opening all of the cameras using using the normal Royale API, thus taking exclusive
access to all of them.  One by one it releases each camera, runs the application once, and then
takes control of the camera again.  A command-line option allows selecting a subset of the cameras.

rawviewer
---------

Replaced by RoyaleViewer, but still useful when working on the bridge (IBridgeDataReceiver)
implementations.  This shows data coming from the imager, it receives the data that would normally
be given to the processing.

This uses low-level access, without the Royale API.

royaleviewer
------------

The GUI application that is provided with the SDK (the binary-only distribution), and also
extensively used by the development team.

If you need level 2 access (or higher), the access codes are in CameraManager.cpp. There's a comment
next to each code that's easy to remember, use with `git grep words.of.comment`, and then cut &
paste for RoyaleViewer's command line.

### royaleviewerlevel2

This is an application for Android, it adds an icon to the Android Launcher which will start
RoyaleViewer with the level 2 access code.

rrfCutTool
----------

Command line application that can be used to cut Royale recordings.

This uses the rrfReader and rrfWriter C libraries.


rrftool
-------

Shows information about recorded RRF files, and can convert old recordings to a later RRF format.

This uses the rrfReader and rrfWriter C libraries.

sysInfoTool
-----------

Measures the resource usage of the processing (CPU, etc) while playing back an RRF recording.

This uses Royale's API.

# <a name="storage"></a> Storage tools (reading / writing from the flash)

Royale's tools folder includes several different tools for accessing the storage in devices. For
devices with multiple storage chips, such as a (1 attached to M2453, 1 attached to CX3) combination,
some of the tools can access one or the other.

In these summaries "read calibration data" means copying it from the device to the developer's PC,
and "write calibration data" means copying it in the other other direction.

*todo* ROYAL-3229 These tools have common functionality, can they be merged to a smaller number of
multi-purpose tools?

calibrationFlashTool (since v3.13.0)
------------------------------------

#### Abilities

* read calibration data
* write calibration if and only if Royale supports writing to these devices
* add and change product codes

#### Limitations:

* Arctic devices are limited to StorageFormatPolar (no support for Zwetschge)
* Enclustra devices are limited to StorageFormatPico
* can't write to imager-attached storage (not supported by the Royale L3 API yet)

This uses Royale's API with L3 access to read and write data, combined with directly accessing the
StorageFormat-parsing classes.

eraseFlash
----------

For specific USB devices, a tool to help with programming new firmware.  This doesn't put the new
firmware on the device, it just overwrites the existing firmware image so that the device switches
to its no-firmware fallback state (which for the supported devices allows new firmware to be sent
via USB).

#### Abilities

* make FX3 and CX3 based devices boot to the Cypress bootloader

#### Limitations

* only supports some devices
* Cypress FX3 development kit required to put new firmware on the device (it's a software kit, no
  hardware required except a standard laptop)

This uses low-level access, without the Royale API.

monstarFirmwareUpdate
-------------------------

This tool should only be used to update pico monstar glass devices to the newest firmware.

#### Abilities

* write new firmware to pico monstar devices
* does not require the Cypress FX3 dev kit

#### Limitations

* only supports specific devices

spiFlashTool
------------

On certain M2453 and M2452\_B1x devices (such as Skylla, Daedalus or Alea), the calibration data can
be accessed by using the imager itself as an SPI master. This tool handles this access, including
loading the M2452's SpiBusMaster firmware.

zwetschgeFlashTool
------------------

This tool can be used to transfer Zwetschge files onto devices with an M2453 or M2455 imager.

#### Limitations:

* limited to devices using the M2453 or M2455 imagers.

This is implemented similar to the spiFlashTool and uses Royale's API with L4 access.

#### Abilities

* read calibration data
* write calibration
* add and change product codes

#### Limitations:

* limited set of supported devices, hardcoded in to the tool

This uses Royale's API with L4 access, and with parts implemented in the
spiFlashTool itself.

zwetschgeTool
-------------

Creates .zwetschge files, containing the data for flash-defined imagers (these files contain
register maps and part of the module config).

*Note:* this is in source/components/storage/python, not the tools folder.


# <a name="deprecated"></a> Deprecated tools

det
---

Captures an image and reports basic statistics about the average depth values.  This provides a
plausibility test for the depth values.

*todo* Is this still used? I can't see a reason to use it instead of RoyaleViewer or the R&D team's
calibration scripts.

releaseanalyzer
---------------

This was an automated testing tool.  Although it is still in the source it is neither used nor
maintained.
