# validate lookup result
ifeq ($(KDIR),)
$(error No KDIR found for platform $(TARGET_PLATFORM))
endif

ifeq ($(BUILD),debug)
ccflags-y += -DDEBUG
endif

obj-m += sprdwl_ng.o
sprdwl_ng-objs += main.o
sprdwl_ng-objs += cfg80211.o
sprdwl_ng-objs += txrx.o
sprdwl_ng-objs += cmdevt.o
sprdwl_ng-objs += npi.o
sprdwl_ng-objs += wapi.o
sprdwl_ng-objs += msg.o
sprdwl_ng-objs += work.o
sprdwl_ng-objs += vendor.o
sprdwl_ng-objs += tcp_ack.o
sprdwl_ng-objs += qos.o

ifeq ($(strip $(SPRDWL_PLATFORM)),sc2331)
ccflags-y += -DSPRDWL_INTF_SDIO_BA
sprdwl_ng-objs += sdio_ba.o
else ifeq ($(strip $(SPRDWL_PLATFORM)),sc2332)
ccflags-y += -DSPRDWL_INTF_SDIO
sprdwl_ng-objs += sdio.o
sprdwl_ng-objs += external.o
else ifeq ($(strip $(SPRDWL_PLATFORM)),sp9832e)
ccflags-y += -DSPRDWL_INTF_SIPC
sprdwl_ng-objs += sipc.o
sprdwl_ng-objs += embedded.o
endif

modules:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) $@

clean:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) $@

kernelrelease:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) $@
