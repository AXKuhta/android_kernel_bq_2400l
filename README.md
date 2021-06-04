BQ 2400L Kernel
===============

Building:
 - Make sure you have the neccessary packages installed: `build-essential bc bison flex libncurses-dev libssl-dev libelf-dev`
 - Download the appropriate toolchain: https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/+archive/refs/heads/ndk-release-r21.tar.gz
 - Unpack it in a folder somewhere: `tar -xf ndk-release-r22.tar.gz`
 - Apply a small fix:

   ```
   cd *TOOLCHAIN_DIR*
   cd bin
   rm arm-linux-androideabi-gcc
   ln -s arm-linux-androideabi-gcc-4.9.x arm-linux-androideabi-gcc
   ```
 - Edit `build_kernel.sh` to have the right path
 - Run `build_kernel.sh`

This should result in `out/arch/arm/boot/Image`

Rename to `boot.img-kernel`, repack your boot.img with it, and flash it with fastboot.
