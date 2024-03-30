# 3D Camera DLL for SimCity 4

Experience your cities from completely new angles.

## Features

Activates two cheat codes that allow you to freely change the camera angles of the game.

## System requirements

- SimCity 4, version 1.1.641 (the digital release)
  ([more info](https://community.simtropolis.com/forums/topic/762980-the-future-of-sc4-modding-the-matter-of-digital-vs-disc-and-windows-vs-macos-in-the-dll-era/))
- Windows 10+ or Linux

## Installation

Copy the DLL into the top-level directory of either Plugins folder
(place it directly in `<Documents>\SimCity 4\Plugins` or `<SC4 install folder>\Plugins`, not in a subfolder).

## Usage

Open the cheat console with `CTRL + x` and enter the `CameraYaw` or `CameraPitch` cheats:

- `CameraYaw <angle>` - set the azimuth angle (counter-clockwise)
- `CameraYaw` - reset the angle to the default of 22.5 degree
- `CameraPitch <angle>` - set the altitude angle for all zoom levels (range: 0.0 to 90.0 degree)
- `CameraPitch <angle 1> .. <angle 5>` - set the altitude angle for zoom levels 1 to 5 individually
- `CameraPitch` - reset the angles to the default for each zoom: 30°, 35°, 40°, 45°, 45°

**Examples:**
- top down view: `CameraPitch 85` and `CameraYaw 0`
- isometric SC3k-style view: `CameraYaw 45`

Sometimes the effect only fully applies after changing zoom level or rotating the map, particularly on zooms 5 and 6.

Graphical glitches may occur, especially with the more extreme angles.

To avoid rendering the region view of a city from a non-standard angle,
use Fast-Save (hold ALT + press Save button) or reset the angles to the default values before saving.

## Troubleshooting

The plugin should write a `.log` file in the folder containing the plugin.
The log contains status information for the most recent run of the plugin.

------------------------------------------------------------
## Information for developers

### Building the plugin

The DLL is compiled using `clang` as a cross-compiler.
Check the [Makefile](Makefile) for details.
```
git submodule update --init
make
```
The source code is mostly compatible with the MSVC compiler as well, but some tweaks may be needed for that.

### License

This project is licensed under the terms of the GNU Lesser General Public License version 3.0.
See [LICENSE.txt](LICENSE.txt) for more information.

#### 3rd party code

- [gzcom-dll](https://github.com/nsgomez/gzcom-dll/tree/master) Located in the vendor folder, MIT License.
- [Windows Implementation Library](https://github.com/microsoft/wil) MIT License
- [SC4Fix](https://github.com/nsgomez/sc4fix) MIT License
- [NAM-dll](https://github.com/NAMTeam/nam-dll) LGPL 3.0
- [sc4-growify](https://github.com/0xC0000054/sc4-growify) MIT License
