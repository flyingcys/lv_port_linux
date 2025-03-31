#
# Makefile
#
# CC ?= gcc
CC              = /home/share/samba/risc-v/sg200x/duo-buildroot-sdk/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc
CFLAGS          ?= -O3 -g0 -march=rv64imafdcvxthead -mcpu=c906fdv -mcmodel=medany -mabi=lp64d -Wall -Wshadow -I$(LVGL_DIR)/ $(WARNINGS)
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}
# CFLAGS ?= -O3 -g0 -I$(LVGL_DIR)/ -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare
LDFLAGS ?= -lm
BIN = demo


#Collect the files to compile
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

CSRCS +=$(LVGL_DIR)/mouse_cursor_icon.c 

CSRCS +=$(LVGL_DIR)/cubegame.c 

# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_0.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_1.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_2.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_3.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_4.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_5.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_6.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_7.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_8.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_9.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_10.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_11.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_0_12.c 
# CSRCS +=$(LVGL_DIR)/icon_replace_2/icon_replace_2.c 

CSRCS += $(LVGL_DIR)/qieshuiguo/apple_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/banana_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/boom_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/dragon_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/fruit_bg_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/green_blood_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/pear_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/pink_blood_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/qieshuiguo.c
CSRCS += $(LVGL_DIR)/qieshuiguo/red_blood_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/split_apple_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/split_banana_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/split_dragon_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/split_pear_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/split_xigua_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/start_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/xigua_img.c
CSRCS += $(LVGL_DIR)/qieshuiguo/yellow_blood_img.c


CSRCS += $(LVGL_DIR)/fly_game/boom.c
CSRCS += $(LVGL_DIR)/fly_game/enemyboss.c
CSRCS += $(LVGL_DIR)/fly_game/enemyfly1.c
CSRCS += $(LVGL_DIR)/fly_game/enemyfly2.c
CSRCS += $(LVGL_DIR)/fly_game/enemyfly3.c
CSRCS += $(LVGL_DIR)/fly_game/fly.c
CSRCS += $(LVGL_DIR)/fly_game/flygame.c

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)

