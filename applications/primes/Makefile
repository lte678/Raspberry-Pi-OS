ARMGNU ?= ../../toolchain/build/bin/aarch64-elf-lxe

CXXFLAGS = -Wall -g

TARGET = primes
BUILD = build/
OBJS = primes.o
HEADERS = 

OBJS := $(addprefix $(BUILD),$(OBJS))
TARGET := $(BUILD)$(TARGET)

# Rule to make everything.
all: $(TARGET) 

clean:
	rm -r build

$(BUILD):
	mkdir $@

$(BUILD)%.o: %.cpp $(HEADERS) $(BUILD) 
	$(ARMGNU)-g++ -c -o $@ $< $(CXXFLAGS)

$(TARGET): $(OBJS)
	$(ARMGNU)-g++ -o $@ $^ $(CXXFLAGS)
asm:
	$(ARMGNU)-objdump -S $(TARGET) > $(TARGET).asm
