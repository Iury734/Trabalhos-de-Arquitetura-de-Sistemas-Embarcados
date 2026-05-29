CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
CPU = -mcpu=cortex-m0plus -mthumb
CFLAGS = $(CPU) -O0 -g

all: main.bin

main.elf: main.c
	$(CC) $(CFLAGS) -nostartfiles -Ttext 0x0 main.c -o main.elf

main.bin: main.elf
	$(OBJCOPY) -O binary main.elf main.bin

clean:
	rm -f *.elf *.bin