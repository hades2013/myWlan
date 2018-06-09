ATH_BLD_DIR := $(call my-dir)

ifeq ($(BOARD_HAS_ATH_WLAN_AR6004), true)
        include $(ATH_BLD_DIR)/ath6kl_fw/AR6004/Android.mk
        include $(ATH_BLD_DIR)/btfilter/Android.mk
endif
ifeq ($(BOARD_HAS_ATH_WLAN_AR6320), true)
        include $(ATH_BLD_DIR)/libtcmd/Android.mk
        include $(ATH_BLD_DIR)/libathtestcmd/libtlvutil/Android.mk
        include $(ATH_BLD_DIR)/libathtestcmd/Android.mk
        include $(ATH_BLD_DIR)/myftm/Android.mk
endif

#Build/Package FTM Module in case of 8994 target variants
ifeq ($(call is-board-platform,msm8994),true)
        ifeq ($(BOARD_HAS_ATH_WLAN_AR6320), true)
                include $(ATH_BLD_DIR)/libtcmd/Android.mk
        endif
endif
