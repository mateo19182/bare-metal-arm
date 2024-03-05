PREFIX=arm-none-eabi-
ARCHFLAGS=-mthumb -mcpu=cortex-m0plus
COMMONFLAGS=-g3 -Og -Wall -Werror $(ARCHFLAGS)
CFLAGS=-I./includes -I./drivers $(COMMONFLAGS) -D CPU_MKL46Z128VLH4
LDFLAGS=$(COMMONFLAGS) --specs=nano.specs -Wl,--gc-sections,-Map,$(TARGET).map,-Tlink.ld
LDLIBS=

CC=$(PREFIX)gcc
LD=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy
SIZE=$(PREFIX)size
RM=rm -f

BUILD_TARGET ?= hello

ifeq ($(BUILD_TARGET),hello)
TARGET=hello_world
SRC=$(filter-out drivers/pin_mux_led.c, $(wildcard drivers/*.c includes/*.c startup.c)) $(TARGET).c

else ifeq ($(BUILD_TARGET),led)
TARGET=led_blinky
SRC=$(filter-out drivers/pin_mux_hello.c, $(wildcard drivers/*.c includes/*.c startup.c)) $(TARGET).c

else
$(error BUILD_TARGET unknown. Use BT=hello or BT=led)
endif

OBJ = $(patsubst %.c, %.o, $(SRC))

all: echo_target build size

echo_target:
	@echo "Building target: $(TARGET)"

build: $(TARGET).elf $(TARGET).srec $(TARGET).bin

clean:
	@echo "Cleaning..."
	$(RM) *.srec *.elf *.bin *.map $(OBJ)

$(TARGET).elf: $(OBJ)
	@echo "Linking $@"
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

%.srec: %.elf
	$(OBJCOPY) -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

size:
	$(SIZE) $(TARGET).elf

flash: all
	openocd -f openocd.cfg -c "program $(TARGET).elf verify reset exit"
