PREFIX=arm-none-eabi-

hello: hello_target all

hello_target:
	$(eval TARGET=hello_world)

led: led_target all

led_target:
	$(eval TARGET=led_blinky)

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

#TARGET=led_blinky
TARGET=hello_world

#SRC=$(filter-out drivers/pin_mux_hello.c, $(wildcard drivers/*.c includes/*.c startup.c)) $(TARGET).c
SRC=$(filter-out drivers/pin_mux_led.c, $(wildcard drivers/*.c includes/*.c startup.c)) $(TARGET).c
OBJ = $(patsubst %.c, %.o, $(SRC))


all: echo_target build size
build: elf srec bin
elf: $(TARGET).elf
srec: $(TARGET).srec
bin: $(TARGET).bin

echo_target:
	@echo "Building target: $(TARGET)"

clean:
	@echo "Cleaning all targets..."
	$(RM) led_blinky.srec led_blinky.elf led_blinky.bin led_blinky.map $(OBJ)
	$(RM) hello_world.srec hello_world.elf hello_world.bin hello_world.map $(OBJ)

$(TARGET).elf: $(OBJ)
	@echo "Linking target: $(TARGET)"
	$(LD) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@

%.srec: %.elf
	$(OBJCOPY) -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

size:
	$(SIZE) $(TARGET).elf

flash_hello: hello_target all
	openocd -f openocd.cfg -c "program $(TARGET).elf verify reset exit"

flash_led: led_target all
	openocd -f openocd.cfg -c "program $(TARGET).elf verify reset exit"
