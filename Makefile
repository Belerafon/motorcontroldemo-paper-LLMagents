VECTORIDE ?= C:/VectorIDEv1.3.3
TOOLCHAIN_BIN ?= $(VECTORIDE)/CodeBench/bin
CROSS_COMPILE ?= $(TOOLCHAIN_BIN)/arm-none-eabi-

CC := $(CROSS_COMPILE)gcc.exe
OBJCOPY := $(CROSS_COMPILE)objcopy.exe
SIZE := $(CROSS_COMPILE)size.exe
RM := $(TOOLCHAIN_BIN)/cs-rm.exe -f

BUILD_DIR := EclipseIDE/Debug
TARGET := $(BUILD_DIR)/MotorControlDemo
ELF := $(TARGET).elf
BIN := $(TARGET).bin
MAP := $(TARGET).map

SOURCES := $(wildcard src/*.c) \
           $(wildcard Vsrc/*.c) \
           $(wildcard Vsrc/USBlib/*.c)
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))
DEPS := $(OBJECTS:.o=.d)
LISTINGS := $(OBJECTS:%=%.lst)

CPU_FLAGS := -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
COMMON_FLAGS := $(CPU_FLAGS) -O2 -fmessage-length=0 -fsigned-char \
                -ffunction-sections -fdata-sections -g3
CFLAGS := $(COMMON_FLAGS) -Iinclude -IVinclude -std=gnu11 \
          -fsingle-precision-constant
LDFLAGS := $(COMMON_FLAGS) -T EclipseIDE/cmd/build.ld \
           -nodefaultlibs -nostdlib -Xlinker --gc-sections \
           -LEclipseIDE/lib -Xlinker -LEclipseIDE/cmd -Wl,-Map,$(MAP)
LIBS := EclipseIDE/lib/libc.a \
        EclipseIDE/lib/libgcc.a \
        EclipseIDE/lib/libm.a \
        EclipseIDE/lib/libCANOpen_drv.a

.DEFAULT_GOAL := all

.PHONY: all clean size

all: $(ELF) $(BIN) size

$(ELF): $(OBJECTS)
	$(CC) $(LDFLAGS) -o "$@" $(OBJECTS) $(LIBS)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary "$<" "$@"

size: $(ELF)
	$(SIZE) --format=berkeley "$(ELF)"

$(BUILD_DIR)/%.o: %.c
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(CFLAGS) -Wa,-adhlns="$@.lst" -MMD -MP \
		-MF"$(@:.o=.d)" -MT"$@" -c -o "$@" "$<"

clean:
	$(RM) $(OBJECTS) $(DEPS) $(LISTINGS) "$(ELF)" "$(BIN)" "$(MAP)"

-include $(DEPS)
