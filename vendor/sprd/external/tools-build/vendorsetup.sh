#
# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

########################################################################
# Spreadtrum defined quick commands for Android Building
########################################################################

function get_arch_name()
{
	kernel_arch=$(get_build_var TARGET_KERNEL_ARCH)

	if [ ! "$kernel_arch" ]; then
		if [ "true" = "$(get_build_var CONFIG_64KERNEL_32FRAMEWORK)" ]; then
			echo "arm64"
		else
			echo $(get_build_var TARGET_ARCH)
		fi
	else
		echo $kernel_arch
	fi
}

# a quick command to install kernel header files for userspace building
function kheader()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi
	mkdir -p $OUT/obj/KERNEL
	make -C $T/kernel O=$OUT/obj/KERNEL ARCH=$(get_arch_name) headers_install
}

# a quick command to launch kernel menuconfig
function kmconfig()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi
	mkdir -p $OUT/obj/KERNEL
	make -C $T/kernel O=$OUT/obj/KERNEL ARCH=$(get_arch_name) menuconfig
}

# a quick command to launch kernel menuconfig and update .config to kernel
function kuconfig()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi
	KERNEL_DEFCONFIG=$(get_build_var KERNEL_DEFCONFIG)
	_ARCH=$(get_arch_name)
	mkdir -p $OUT/obj/KERNEL
	make -C $T/kernel O=$OUT/obj/KERNEL ARCH=$_ARCH $KERNEL_DEFCONFIG
	make -C $T/kernel O=$OUT/obj/KERNEL ARCH=$_ARCH menuconfig
	cp $OUT/obj/KERNEL/.config $T/kernel/arch/$_ARCH/configs/$KERNEL_DEFCONFIG
}

# a quick command to update kernel config to the corresponding defconfig
function kdconfig()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi
	KERNEL_DEFCONFIG=$(get_build_var KERNEL_DEFCONFIG)
	mkdir -p $OUT/obj/KERNEL
	make -C $T/kernel O=$OUT/obj/KERNEL ARCH=$(get_arch_name) $KERNEL_DEFCONFIG
}

# a quick command to check if the contents of two files are same
function kcconfig()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi
	KERNEL_DEFCONFIG=$(get_build_var KERNEL_DEFCONFIG)
	_ARCH=$(get_arch_name)
	mkdir -p $OUT/obj/KERNEL
	make -C $T/kernel O=$OUT/obj/KERNEL ARCH=$_ARCH $KERNEL_DEFCONFIG
	cr=`diff $OUT/obj/KERNEL/.config $T/kernel/arch/$_ARCH/configs/$KERNEL_DEFCONFIG`
	if [ "$cr" == "" ];then
		echo "$KERNEL_DEFCONFIG is up-to-date!"
		return 0
	else
		echo "WARNING:$KERNEL_DEFCONFIG is not up-to-date!"
		echo "Be sure you already update code to the latest point"
		echo "before using kuconfig to update your defconfig"
		echo "and try again, please."
		return 1
	fi
}

# a quick command to clean kernel building files
function kclean()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi
	make -C $T/kernel O=$OUT/obj/KERNEL ARCH=$(get_arch_name) mrproper
}

# a quick command to run kernel make
function kmk()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi
	make -C $T/kernel \
		O=$OUT/obj/KERNEL \
		ARCH=$(get_arch_name) \
		CROSS_COMPILE=$(get_build_var KERNEL_CROSS_COMPILE) \
		-j4 \
		$*
	UNCOMPRESS=$(get_build_var USES_UNCOMPRESSED_KERNEL)
	if [ "$UNCOMPRESS" = "true" ]; then
		KIMG=$OUT/obj/KERNEL/arch/$(get_arch_name)/boot/Image
	else
		KIMG=$OUT/obj/KERNEL/arch/$(get_arch_name)/boot/bzImage
	fi
	cp $KIMG $OUT/kernel
}

# a quick command to clean u-boot building files
function uclean()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi
	make -C $T/u-boot O=$OUT/obj/u-boot mrproper
}

# a quick command to clean u-boot building files
function umk()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi
	make -C $T/u-boot O=$OUT/obj/u-boot $*
	cp $OUT/obj/u-boot/u-boot.bin $OUT/
	cp $OUT/obj/u-boot/nand_spl/u-boot-spl-16k.bin $OUT/
}

# a quick command to config u-boot
function uconfig()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi
	make -C $T/u-boot O=$OUT/obj/u-boot $(get_build_var UBOOT_DEFCONFIG)_config
}

function packboot()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi
	rm -f $OUT/ramdisk.img
	$ANDROID_HOST_OUT/bin/mkbootfs $OUT/root | $ANDROID_HOST_OUT/bin/minigzip > $OUT/ramdisk.img
	DT_SUPPORT=$(get_build_var BOARD_KERNEL_SEPARATED_DT)
	if [ "$DT_SUPPORT" = "true" ]; then
	O_ARGS="--dt $OUT/dt.img"
	fi
	$ANDROID_HOST_OUT/bin/mkbootimg  --kernel $OUT/kernel --ramdisk $OUT/ramdisk.img --cmdline "console=ttyS1,115200n8" --base 0x00000000 $O_ARGS --output $OUT/boot.img
}
# arg is the target ramdisk
function packboot_ramdisk()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi

	if [ ! "$1" ]; then
		echo "you must give a target ramdisk."
		return
	fi

	cp $1 $OUT/ramdisk.img

	DT_SUPPORT=$(get_build_var BOARD_KERNEL_SEPARATED_DT)
	if [ "$DT_SUPPORT" = "true" ]; then
	O_ARGS="--dt $OUT/dt.img"
	fi
	$ANDROID_HOST_OUT/bin/mkbootimg  --kernel $OUT/kernel --ramdisk $OUT/ramdisk.img --cmdline "console=ttyS1,115200n8" --base 0x00000000 $O_ARGS --output $OUT/boot.img
}

function packsys()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi

	SYSIMG_SIZE=$(get_build_var BOARD_SYSTEMIMAGE_PARTITION_SIZE)
	SPARSE_DIS=$(get_build_var TARGET_SYSTEMIMAGES_SPARSE_EXT_DISABLED)
	if [ "$SPARSE_DIS" = "true" ]; then
		SOPT=""
	else
		SOPT="-s"
	fi
	$ANDROID_HOST_OUT/bin/make_ext4fs $SOPT -S $OUT/root/file_contexts -l $SYSIMG_SIZE -a system $OUT/system.img $OUT/system
}

# get dts file from dtb
# use example1: source build/envsetup.sh; lunch xxx; make bootimage; getdts
# use example2: source build/envsetup.sh; lunch xxx; make bootimage; getdts ~/abc.dts
function getdts()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP."
		return
	fi

	dtc=$OUT/obj/KERNEL/scripts/dtc/dtc

	if [ ! "$1" ]; then
		$dtc -I dtb $OUT/obj/KERNEL/arch/$(get_arch_name)/boot/dts/$(get_build_var TARGET_DTB).dtb -O dts > ~/$(get_build_var TARGET_DTB).dts
	else

		$dtc -I dtb $OUT/obj/KERNEL/arch/$(get_arch_name)/boot/dts/$(get_build_var TARGET_DTB).dtb -O dts > $1
	fi
}
