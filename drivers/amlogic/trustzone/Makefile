##########################################
########## Amlogic Drivers ###############
##########################################

# Makefile for TRUSTZONE

TARGET_ASMFLAGS = -DCONFIG_CORTEX_A9=1 -mcpu=cortex-a9
EXTRA_CFLAGS = -g -Wall -nostdlib
EXTRA_CFLAGS += -msoft-float -mno-thumb-interwork -marm -fno-short-enums $(TARGET_CCFLAGS)

#ccflags-y:= -I../include

obj-$(CONFIG_MESON_TRUSTZONE) += otz_client_main.o




