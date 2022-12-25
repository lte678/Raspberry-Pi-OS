ARMGNU ?= ../../toolchain/build/bin/aarch64-elf-lxe

OPTIONS = 
CFLAGS = -Wall -Wl,-z,max-page-size=4096 $(OPTIONS)

TARGET = demo
BUILD = build/
OBJS = main.o
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
	$(ARMGNU)-gcc -o $@ $^ $(CFLAGS)
asm:
	$(ARMGNU)-objdump -S $(TARGET) > $(TARGET).asm
