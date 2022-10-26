ARMGNU ?= ../../crosscompiler/bin/aarch64-none-elf

OPTIONS =
CFLAGS = -Wall -Og -g -Wl,-z,max-page-size=4096 $(OPTIONS)
LINKER = lxe.ld

TARGET = demo
BUILD = build/
OBJS = main.o syscalls.o
HEADERS = 

OBJS := $(addprefix $(BUILD),$(OBJS))
TARGET := $(BUILD)$(TARGET)

# Rule to make everything.
all: $(TARGET) 

clean:
	rm -r build

$(BUILD):
	mkdir $@

$(BUILD)%.o: %.c $(HEADERS) $(BUILD) 
	$(ARMGNU)-gcc -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(ARMGNU)-gcc -o $@ $^ $(CFLAGS) -T $(LINKER)

asm:
	$(ARMGNU)-objdump -S $(TARGET) > $(TARGET).asm
