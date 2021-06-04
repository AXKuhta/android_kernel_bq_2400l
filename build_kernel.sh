export PATH="$HOME/google_armv7_gcc/bin:$PATH"
export ARCH=arm
export CROSS_COMPILE=arm-linux-androideabi-

mkdir out
make O=out -j$(nproc) bq2400l_defconfig
make O=out -j$(nproc)

