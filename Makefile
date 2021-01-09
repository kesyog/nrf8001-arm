ROOT_DIR = .
BUILDDIR = ./_build
TARGET=$(BUILDDIR)/libnrf8001.a

MKDIR=mkdir -p
ECHO=echo

CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OD=arm-none-eabi-objdump

DEFS =

MCU = cortex-m4
MCFLAGS = -mcpu=$(MCU) -mthumb -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb-interwork -std=c99

STM32DIR=./libraries/STM32F4_PeriphLib/
STM32INC = \
	$(STM32DIR)/inc/ \
	./libraries/CMSIS/Device/ST/STM32F4xx/Include/ \
	./libraries/CMSIS/Include/ \

OPTIMIZE       = -Os

INC = \
	./libraries/BLE/ \
	./src/ \

SRC = \
	./src/ble_assert.c \
	./libraries/BLE/acilib.c \
	./libraries/BLE/aci_queue.c \
	./libraries/BLE/aci_setup.c \
	./libraries/BLE/hal_aci_tl.c \
	./libraries/BLE/lib_aci.c \

CFLAGS	= $(MCFLAGS)  $(OPTIMIZE)  $(DEFS)  -Wall -fPIE
AFLAGS	= rcs
#-mapcs-float use float regs. small increase in code size

OBJDIR = $(BUILDDIR)/obj
OBJ := $(patsubst $(ROOT_DIR)/%, $(OBJDIR)/%, $(addsuffix .o, $(basename $(SRC))))

all: $(TARGET)

$(TARGET): $(OBJ)
	@$(ECHO) "AR: $(subst $(ROOT_DIR)/,,$@)"
	@$(AR) $(AFLAGS) $@ $^

$(OBJDIR)/%.o: $(ROOT_DIR)/%.c
	@$(MKDIR) $(dir $@)
	@$(ECHO) "CC: $(subst $(ROOT_DIR)/,,$<)"
	@$(CC) $(CFLAGS) $(addprefix -I,$(INC)) -c -MMD -MP -o $@ $<

clean:
	@rm -r $(BUILDDIR)
