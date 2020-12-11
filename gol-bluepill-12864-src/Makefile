PROJECT = gol-bluepill-12864
BUILD_DIR = bin

CFILES = gol_main.c

DEVICE=stm32f103c8
OOCD_FILE = target/stm32f1x.cfg

VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk