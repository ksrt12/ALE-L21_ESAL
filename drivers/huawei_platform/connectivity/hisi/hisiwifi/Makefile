#Makefile for Hisilicon Wi-Fi Chip Hi1101

CFLAGS =

EXTRA_CFLAGS = $(CFLAGS)  \
           -DARP_OFFLOAD_SUPPORT \
	       -D__ROAM__ \
	       -DMAC_802_11W \
	       -D__CHECKSUM__
#EXTRA_CFLAGS += -DWLAN_PERFORM_DEBUG
EXTRA_CFLAGS += -DWLAN_POWER_MANAGEMENT -DWLAN_PATCH -DWLAN_PERFORM_OPT
EXTRA_CFLAGS += -DWLAN_ARRG_DYNAMIC_CONTROL
EXTRA_CFLAGS += -DWMM_OPT_FOR_AUTH
#EXTRA_CFLAGS += -DDEBUG
EXTRA_CFLAGS += -DHCC_DEBUG

#change wanrings to error,must warning clean!
EXTRA_CFLAGS += -Werror

#android4.2 cross-compile support module must add the follow options!
#EXTRA_CFLAGS += -DMODULE -fno-pic
#MODFLAGS = -DMODULE -fno-pic

EXTRA_CFLAGS += -DINI_CONFIG
ifneq ($(KERNELRELEASE),)
obj-$(CONFIG_HI1101_WIFI) += wifi.o

wifi-objs +=	hi110x.o
wifi-objs +=	hsdio.o
wifi-objs +=	hwifi_cfg80211.o
wifi-objs +=	station_mgmt.o
wifi-objs +=	cfg_event_rx.o
wifi-objs +=	hwifi_msg.o
wifi-objs +=	hwifi_hcc.o
wifi-objs +=	hwifi_netdev.o
wifi-objs +=	hwifi_utils.o
wifi-objs +=	hwifi_extern.o
wifi-objs +=	hwifi_regdb.o
wifi-objs +=	user_ctrl.o
wifi-objs +=	hwifi_android.o
wifi-objs +=    hwifi_wpa_ioctl.o
wifi-objs +=    hwifi_wl_config_ioctl.o
wifi-objs +=	hwifi_perform.o
wifi-objs +=	hwifi_regdomain.o
wifi-objs +=	hwifi_scan.o
wifi-objs +=	hi110x_pm.o
wifi-objs +=    hwifi_rfauth_ioctl.o

wifi-objs +=	hwifi_roam_main.o
wifi-objs +=	hwifi_roam_fsm.o
wifi-objs +=	hwifi_roam_alg.o
wifi-objs +=    hwifi_init.o

wifi-objs +=	hwifi_aggre.o
wifi-objs +=	hwifi_sfw_antijam.o
wifi-objs +=    hwifi_tps.o
wifi-objs +=    hwifi_dev_err.o
wifi-objs +=    hwifi_config.o
wifi-objs +=    hsdio_test.o
else

default:
	$(MAKE) M=$(shell pwd) modules
endif

clean:
	$(MAKE) -C  M=$(shell pwd) clean

install:default
	adb push wifi.ko /data/

.PHONY:clean
