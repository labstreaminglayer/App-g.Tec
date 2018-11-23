# gUSBamp App

## Build

It is expected that this folder location is labstreaminglayer/Apps/g.Tec/g.USBamp.
You must also have a g.tec API installed (see below).

This app depends on Qt. Please see the parent labstreaminglayer [build instructions](https://github.com/labstreaminglayer/labstreaminglayer/blob/master/doc/BUILD.md)
for how information on how to obtain Qt and instruct cmake to use it.

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 14 2015 Win64" -DQt5_DIR=C:\Qt\5.11.1\msvc2015_64\lib\cmake\Qt5 -DBOOST_ROOT=C:\local\boost_1_65_0
```

### gUSBamp API

#### Windows 

The gUSBampCAPI has been deprecated in favour of the gNEEDaccess CAPI. There is a g.NEEDaccess LSL app available.
This application still uses the legacy API, but our ability to support it is limited, and g.tec will not be able to support it either.
Please contact g.Tec support directly support@gtec.at to determine if you are eligible for a free upgrade to the g.NEEDaccess API.

Using the deprecated API...

The cmake build system will search for the gUSBampCAPI in C:\Program Files\gtec\gUSBampCAPI\API\Win32.
This folder is expected to have gUSBamp.h, gUSBamp.dll, and gUSBamp.bin
If your amp serial number is of the form UA-XXXX.XX.XX, you may need to replace your gUSBamp.dll file with a gUSBamp-for-2.0.dll file and rename to gUSBamp.dll.

#### Linux

There is a g.USBamp Linux CAPI. I haven't added support for this yet. I intend to. If this is urgent to you then please let Chadwick know.

## Usage
  * Start the gUSBamp app. You should see a window like the following.
> ![gUSBamp.png](gUSBamp.png)

  * Make sure that you have correctly installed the drivers for your amplifier, and that the amplifier is plugged in and ready to use (see also official brochure).

  * If you have only one device plugged in, you can leave the Device Port or Serial setting at its default value. i.e., "(search)". If you have multiple devices you can either enter the device USB port number (starting from 0) or, better, the serial number of the device, which is a string of the form UX-XXXX.XX.XX.

  * Select the desired number of channels to record, and enter your channel labels (make sure that the number of provided labels matches the number of channels setting).

  * Use whatever sampling rate is appropriate for your experiment; you can use a lower rate if necessary to save network bandwidth (when transmitting on a slow network) or to reduce the disk space taken up by your recordings. Some analyses will require higher sampling rates than the default (e.g., measuring auditory brainstem responses). If you choose a high sampling rate, you should also increase the chunk size (the official recommendation is to use a power of two that is closest to sampling rate / 32 (e.g., if your sampling rate is 9600, 9600/32=300, so you pick 256 as the chunk size). You can pick a lower chunk size within some reasonable tolerance if you need particularly low latency access to the data (e.g., for real-time P300 spelling applications).

  * The typical setting for Common Ground and Common Reference is to have them both checked -- unchecking Common Ground means that each block of 4 channels has its own ground contact, which can be placed anywhere on the subject(s), and the same holds for Common Reference.

  * The Act as Slave setting is currently untested -- in theory it allows you to synchronize the clocks of multiple devices in hardware, but it requires some precautions. You need to declare one device the "master", and check Act as Slave in all other devices. Also, the master must be the first to be linked, and the last to be unlinked. If this is not checked, the synchronization will be done in software based on time stamps.

  * Click the "Link" button. If all goes well you should now have a stream on your lab network that has name "gUSBamp-0" (if you used device 0) and type "EEG", and a second one named "gUSBamp-0-Markers" with type "Markers" that holds the event markers. Note that you cannot close the app while it is linked.

## License

I don't know who wrote the original version of the App. If you wrote the app then please reach out to Chad.
Most LSL Apps are released under MIT, so this app is now being released under that license,
and Chadwick Boulay is taking over the copyright (exclusively, for now, but happy to share with original author).
