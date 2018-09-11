# gUSBamp App

## Build

It is expected that this folder location is labstreaminglayer/Apps/g.Tec/g.USBamp.
You must also have a g.tec API installed (see below).

This app depends on Qt. Please see the parent labstreaminglayer [build instructions](https://github.com/labstreaminglayer/labstreaminglayer/blob/master/doc/BUILD.md)
for how information on how to obtain Qt and instruct cmake to use it.

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 14 2015 Win64" -DQt5_DIR=C:\Qt\5.10.0\msvc2015_64\lib\cmake\Qt5 -DBOOST_ROOT=C:\local\boost_1_65_1
```

### gUSBamp API

#### Windows 

The gUSBampCAPI has been deprecated in favour of the gNEEDaccess CAPI. There is a g.NEEDaccess LSL app available.
This application still uses the legacy API, but our ability to support it is limited, and g.tec will not be able to support it either.
Please contact g.Tec support directly support@gtec.at to determine if you are eligible for a free upgrade to the g.NEEDaccess API.

Using the deprecated API...

The cmake build system will search for the gUSBampCAPI in C:\Program Files\gtec\gUSBampCAPI\API.
This folder is expected to have gUSBamp.h, gUSBamp.dll, and gUSBamp.bin
If your amp serial number is of the form UA-XXXX.XX.XX, you may need to replace your gUSBamp.dll file with a gUSBamp-for-2.0.dll file and rename to gUSBamp.dll.

#### Linux

There is a g.USBamp Linux CAPI. I haven't added support for this yet. I intend to. If this is urgent to you then please let Chadwick know.

## Use

Before you can use this program you must have installed the drivers for your g.USBamp.
The configuration settings can be saved to a .cfg file (see File / Save Configuration)
and subsequently loaded from such a file (via File / Load Configuration).
Importantly, the program can be started with a command-line argument of the form "gUSBamp.exe -c myconfig.cfg",
which allows to load the config automatically at start-up.
The recommended procedure to use the app in production experiments is to make a shortcut on the experimenter's desktop
which points to a previously saved configuration customized to the study being recorded to minimize the chance of operator error.

## License

I don't know who wrote the original version of the App. If you wrote the app then please reach out to Chad.
Most LSL Apps are released under MIT, so this app is now being released under that license,
and Chadwick Boulay is taking over the copyright (exclusively, for now, but happy to share with original author).
