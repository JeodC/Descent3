![d3 (1)](https://github.com/DescentDevelopers/Descent3/assets/47716344/82ba0911-ee32-4565-84ee-b432c215ab95)
This is a fork of the Descent 3 open source engine, tailored to retro handhelds using gl4es and PortMaster. Binaries built from this fork are used in [PortMaster-Descent3](https://github.com/JeodC/PortMaster-Descent3). This fork contains several commits that deviate from the parent repository like QoL commits for controller-only setups.

Examples:

- Adjust viewport and scissor to automatically scale properly among various resolutions and aspect ratios (no need for `-f`, `-w`, `-width <w>`, `-height <h>`, or `-aspect`)
- Make mouse cursor invisible in menus in Release builds (still visible and usable in Debug builds)
- Change level select menu to a listbox with pretty names for core levels
- Prefill savegame dialog with current level number if blank slot or different level than existing slot

You are of course free to cherry-pick commits and/or use them as inspiration for your own.

## Building
Run these commands:

```sh
sudo apt update
sudo apt install -y --no-install-recommends git ninja-build cmake g++ libsdl2-dev zlib1g-dev
git clone https://github.com/JeodC/Descent3
cd Descent3
cmake --preset linux
cmake --build --preset linux --config [Debug|Release]
```

You can use the following commands during the `cmake --preset linux` step:

`-D ENABLE_LOGGER=[ON|OFF]` - This enables mprintf statements in Debug builds, using the -logfile argument. Defaults to ON.  
`-D ENABLE_NETGAMES=[ON|OFF]` - This enables building the netgames, which go in the `netgames` directory. Defaults to OFF.  
`-D ENABLE_NETGAMES=[ON|OFF]` - This enables building the level scripts (`d3-linux.hog`). You usually don't have to build this very often. Defaults to OFF.  


Once CMake finishes, the built files will be put in `builds/linux/Descent3/Debug` or `builds/linux/Descent3/Release`.
