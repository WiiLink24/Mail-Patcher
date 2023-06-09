# Mail-Patcher
Patcher for WiiLink Mail. Work in Progress.

CMake support is from Dolphin Emulator's [hwtest repo](https://github.com/dolphin-emu/hwtests)

## How to compile
- CMake
- devkitPPC
- libogc (if not installed into ${DEVKITPRO}/libogc, a CMake variable called LIBOGCDIR needs to be specified)
- DEVKITPRO and DEVKITPPC environment variables (see devkitPPC documentation)

## How it works
Unlike the [RiiConnect24 Mail Patcher](https://github.com/RiiConnect24/Mail-Patcher), this patcher utilizes the built-in 
registration function of WiiConnect24. It then updates the URL's in `/shared2/wc24/nwc24msg.cfg` to point to WiiLink's servers.
