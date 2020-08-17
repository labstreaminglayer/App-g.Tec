# LabStreamingLayer application for g.Tec g.NEEDaccess Client

## Build Instructions

### Requirements

* [CMake](https://cmake.org/download/)
* [Qt5](https://www.qt.io/download-open-source/)
* g.Tec g.NEEDaccess SDK (from manufacturer)
* [liblsl](https://github.com/sccn/liblsl/releases)
* Build environment
    * Tested with MSVC 2019 Win64

### Configure Project

These instructions assume Visual Studio 2019.

1. Open file explorer to the g.NEEDaccess folder.
2. Right click on empty space and select "Open in Visual Studio".
    * Alternatively, open visual studio first then File > Open the folder.
3. It will detect the CMakeLists.txt file. The initial configuration will fail.
4. Use the Project > CMakeSettings menu to open up a dialog window to edit the cmake options.
    * `Qt5_DIR`: `path/to/Qt5Config.cmake`
    * `GDS_ROOT`: `path/to/gneedaccess sdk/c` if it is not installed into the default directory.
        * Default is defined in ./cmake/FindgNEEDaccessSDK.cmake
    * `LSL_DIR`: `path/to/extracted/liblsl`
5. You may want to add an x64-Release configuration.

## Usage Instructions

* Use g.NEEDaccessDemo client app to check impedances.
    * The gNEEDaccess LSL app can refresh impedance values for some of the supported devices, but the g.NEEDaccessDemo client has a nicer user interface for this
* Quit the g.NEEDaccessDemo client before running this LSL app.

1. Run gNEEDaccess.exe
1. Set the server IP and port, client port, then click Scan.
1. Select your device(s) from the list and check appropriate boxes, then click Connect.
    * Use ctrl + click to select more than one g.USBamp
    * The 'master' g.USBamp should be first in the list of selected devices. You can drag and drop it up.
1. Click on "Config" and configure your device.
1. Change your settings
    * Use the load/save menu to load/save configs.
1. Click OK to set the device config and close the config Window.
1. Click Go! when ready to start streaming over LSL.
1. When finished, click Stop!, then Disconnect, then close the App.

If the application encounters an error (e.g., GDS server lost) then the task bar will alert you and in the application the status bar will update to tell you to look at the log file.

## Known Issues

* Devices use the hardware-provided defaults upon initialization. Loading the config screen will then always use the <devicename>_default.cfg file provided (currently not available for g.USBamp). You can save yourself some effort by backing up the original config, then saving your custom config over the _default.cfg file so this is automatically loaded next time you load the config screen.
