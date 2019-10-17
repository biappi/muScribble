CROSS_COMPILE := arm-none-eabi-

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy

CFLAGS  := -mcpu=cortex-m3 -mthumb -nostartfiles -mfloat-abi=soft -ffreestanding -nostdlib
LDFLAGS := -mcpu=cortex-m3 -mthumb -nostartfiles -Wl,-gc-sections

# - #

PROJ_NAME  := test

BUILD_DIR  := build
SRC_DIR    := src
UCMX_DIR   := unicore-mx

ELF_PATH   := $(BUILD_DIR)/$(PROJ_NAME).elf
BIN_PATH   := $(BUILD_DIR)/$(PROJ_NAME).bin

# = #

UCMX_A     := $(UCMX_DIR)/lib/libucmx_stm32f4.a
LD_SCRIPT  := $(SRC_DIR)/target.ld

CFLAGS     += -I$(UCMX_DIR)/include -DSTM32F4
LDFLAGS    += -L$(UCMX_DIR)/lib -lucmx_stm32f4

SRC_FILES  := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES  := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))


all: $(ELF_PATH) $(BIN_PATH)


$(BUILD_DIR)/.build:
	@mkdir -p $(BUILD_DIR)
	@touch $(BUILD_DIR)/.build


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $^


$(UCMX_A):
	make -C $(UCMX_DIR) TARGETS=stm32/f4 FP_FLAGS="-mfloat-abi=soft"


$(ELF_PATH): $(BUILD_DIR)/.build $(OBJ_FILES) $(UCMX_A)
	$(LD) -o $@ $(OBJ_FILES) $(LDFLAGS) -T$(LD_SCRIPT)


$(BIN_PATH): $(ELF_PATH)
	$(OBJCOPY) -O binary $^ $@

