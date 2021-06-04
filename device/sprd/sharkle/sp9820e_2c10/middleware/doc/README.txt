List of files to install:
=========================

>> For XPCOM module:
 - ewxpcom/EwXPCOMComponent.js
 - ewxpcom/chrome.manifest

>> For MSP Server daemon:
 - bin/EwMSPServer
 - lib/libcrypto.so.1.0.0_ew.so
 - lib/libEwEMBMSAL.so
 - lib/libEwMSPServer.so
 - lib/libssl.so.1.0.0_ew.so
 - workdir/Bootstrap.xml
 - workdir/EwMSPConfig.xml
 - workdir/MMWConfigurationFile.xml
 - src/init.rc (this file shall be integrated in the phone image)

To install MSP Server daemon and XPCOM module on Spreadtrum 9820E device, please follow the steps below:
========================================================================================================

In a command windows, go to Middleware directory of the KaiOS delivery before starting the install

1) ADB commands to activate Root on the device

	|> adb root
	|> adb remount /system

2) ADB commands to install the MSP Server daemon

	|> adb push bin/EwMSPServer /system/bin/.

3) ADB commands to install all the Middleware libraries

	|> adb push lib/libcrypto.so.1.0.0_ew.so /system/lib/.
	|> adb push lib/libEwEMBMSAL.so /system/lib/.
	|> adb push lib/libEwMSPServer.so /system/lib/.
	|> adb push lib/libssl.so.1.0.0_ew.so /system/lib/.

4) ADB commands to install the MSP working directory

	|> adb shell rm -Rf /data/ewworkdir
	|> adb shell mkdir /data/ewworkdir
	|> adb push workdir/Bootstrap.xml /data/ewworkdir/.
	|> adb push workdir/EwMSPConfig.xml /data/ewworkdir/.
	|> adb push workdir/MMWConfigurationFile.xml /data/ewworkdir/.

5) ADB commands to install the XPCOM module

	|> adb shell mkdir -p /system/b2g/distribution/bundles/ewxpcom
	|> adb push ewxpcom/EwXPCOMComponent.js /system/b2g/distribution/bundles/ewxpcom/.
	|> adb push ewxpcom/chrome.manifest /system/b2g/distribution/bundles/ewxpcom/.
	|> adb reboot

WARNING: The Ew XPCOM module is not hot-reloaded after file push install
		 Whenever this XPCOM module is installed or updated, the phone has to be reboot to be taken into account
		 and for the MSP Server to be installed correctly.
		 In final release, the Ew XPCOM module will be integrated in the phone image.

6) ADB commands to create FIFO to manage MSP Server daemon

WARNING: FIFO creation is not persistant : it is deleted on phone reboot.
		 Hence this step needs to be done after each reboot.
		 In final release, the FIFO will be integrated in the phone image.

	|> adb root
	|> adb remount
	|> adb shell
	|> mount -o remount,rw rootfs /
	|> mkdir -p /opt/expway
	|> mknod /opt/expway/ew_embms_fifo_command p
	|> mknod /opt/expway/ew_embms_fifo_response p

To start/stop MSP Server daemon on Spreadtrum 9820E device, please follow the steps below:
========================================================================================================

WARNING: After install or after each phone boot/reboot, the MSP server daemon has to be started manually
		 The start of the MSP Server daemon will be automatic at device boot in the final release.
		 Meanwhile, it needs to be started manually as described in below steps.
		 Remind that after each phone boot/reboot, the FIFO creation (described in step 6) have to be done again before the MSP Server daemon start.

ADB command to start the MSP Server daemon

	|> adb shell /system/bin/EwMSPServer -start /data/ewworkdir

ADB command to stop the MSP Server daemon

	|> adb shell /system/bin/EwMSPServer -stop

