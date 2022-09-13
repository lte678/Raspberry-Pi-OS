ARMGNU ?= ../../crosscompiler/bin/aarch64-none-elf

OPTIONS =
CFLAGS = -Wall -Og -g --specs=nosys.specs -Wl,-z,max-page-size=4096 $(OPTIONS)

TARGET = demo
BUILD = build/
OBJS = main.o syscalls.o
HEADERS = 

OBJS := $(addprefix $(BUILD),$(OBJS))
TARGET := $(BUILD)$(TARGET)

# Rule to make everything.
all: $(TARGET)

$(BUILD)%.o: %.c $(HEADERS)
	$(ARMGNU)-gcc -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(ARMGNU)-gcc -o $@ $^ $(CFLAGS) 

asm:
	$(ARMGNU)-objdump -S $(TARGET) > $(TARGET).asm
