# Tarkov KD Dropper

Tarkov KD Dropper is a small Windows desktop tool for automating a simple death-loop workflow in Escape from Tarkov. It lets you choose a map, pick day or night, configure keybinds, set a death limit, and watch live stats while it runs.

If you want a better user experience and more advanced automation, we also provide other bots at https://tarkov.bot/.

![Tarkov KD Dropper UI](UI.png)

## Features

- Win32 desktop UI
- Configurable map selection
- Optional random map selection
- Configurable day or night selection
- Optional random time selection
- Stop key and grenade key configuration
- Max deaths limit
- Live status, loop count, death count, and average loop time

## Requirements

- Windows
- A 1920x1080 display, or matching coordinate adjustments
- Escape from Tarkov already installed and running
- In-game keybinds aligned with the app settings
- Visual Studio C++ build tools
- OpenCV configured for the project, for example through `vcpkg`

## Build

1. Open the project in Visual Studio.
2. Make sure OpenCV include and library paths are configured.
3. Select `Release | x64` or your preferred build configuration.
4. Build and run the project.

## Usage

1. Launch the app.
2. Open Escape from Tarkov.
3. Prepare your inventory for the grenade loop.
4. Choose a map or enable random map.
5. Choose day or night or enable random time.
6. Set stop key, grenade key, and max deaths.
7. Click `Start Bot`.
8. Press the stop key or click `Stop Bot` to stop the loop.

## Notes

- The project uses fixed screen coordinates and pixel checks, so UI changes, display scaling, or game updates can break behavior.
- Random map selection chooses from the currently enabled working maps (automatically detected).

## Files

- `Tarkov_KD-Dropper_public.cpp`: Main window and UI
- `KdDropper.cpp`: Bot loop logic
- `KdDropper.h`: Core state and configuration
- `utils.cpp` and `utils.h`: Screenshot, pixel, timing, and UI helpers

## Support

If you run into issues, open an issue in the repository. If you want a better user experience and more advanced automation, check out the other bots at https://tarkov.bot/.