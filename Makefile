###############################################################################
#	makefile
#	 by Alex Chadwick
#
#	A makefile script for generation of raspberry pi kernel images.
###############################################################################

# The toolchain to use. arm-none-eabi works, but there does exist 
# arm-bcm2708-linux-gnueabi.
# ARMGNU ?= ./crosscompiler/bin/aarch64-none-elf
# OS specific toolchain
ARMGNU ?= ./toolchain/build/bin/aarch64-elf-lxe
#ARMGNU ?= arm-none-eabi

# The intermediate directory for compiled object files.
BUILD = build/

# The directory in which source files are stored.
SOURCE = source/

# The name of the output file to generate.
TARGET = kernel.img

# The name of the assembler listing file to generate.
LIST = kernel.list

# The name of the map file to generate.
MAP = kernel.map

# The name of the linker script to use.
LINKER = kernel.ld

# Dont use glib
OPTIONS =  # -DTRACE_SYSCALLS -DDEBUG_BUDDY -DDEBUG_SD 
CFLAGS = -mgeneral-regs-only -mcmodel=large\
	-Wall -Og -g -nostdlib -nostartfiles -ffreestanding $(OPTIONS)

# Subfolders containing source files
FOLDERS := $(shell find source/* -type d -printf "%f\n")

# The names of all object files that must be generated. Deduced from the 
# assembly code files in source.
AS_SOURCES := $(wildcard $(SOURCE)*.S)
C_SOURCES := $(wildcard $(SOURCE)*.c)
AS_SOURCES += $(foreach F, $(FOLDERS), $(wildcard $(SOURCE)$(F)/*.S))
C_SOURCES += $(foreach F, $(FOLDERS), $(wildcard $(SOURCE)$(F)/*.c))
AS_OBJECTS := $(patsubst $(SOURCE)%.S, $(BUILD)%.o, $(AS_SOURCES))
C_OBJECTS := $(patsubst $(SOURCE)%.c, $(BUILD)%.o, $(C_SOURCES))

OBJECTS += $(C_OBJECTS)
OBJECTS += $(AS_OBJECTS)

BUILD_FOLDERS := $(BUILD) $(addprefix $(BUILD),$(FOLDERS))

# Rule to make everything.
all: $(TARGET) $(LIST)

# Rule to remake everything. Does not include clean.
rebuild: all

# Rule to make the listing file.
$(LIST) : $(BUILD)output.elf
	$(ARMGNU)-objdump -d $(BUILD)output.elf > $(LIST)

# Rule to make the image file.
$(TARGET) : $(BUILD)output.elf
	$(ARMGNU)-objcopy $(BUILD)output.elf -O binary $(TARGET) 

# Rule to make the elf file.
$(BUILD)output.elf : $(OBJECTS) $(LINKER)
	$(ARMGNU)-gcc $(CFLAGS) -Wl,--no-undefined $(OBJECTS) -Xlinker -Map $(MAP) -o $(BUILD)output.elf -T $(LINKER)

# MAKE OBJECT FILES
# c files
# -c option leaves linking for later
$(BUILD)%.o: $(SOURCE)%.c $(BUILD_FOLDERS)
	$(ARMGNU)-gcc $(CFLAGS) -I $(SOURCE) -I include/ -c $< -o $@

# s files
$(BUILD)%.o: $(SOURCE)%.S $(BUILD_FOLDERS)
	$(ARMGNU)-gcc  -I include/ -c $< -o $@ 

# Create rule for making build folders
$(BUILD_FOLDERS):
	mkdir $@

make-debug:
	@echo $(OBJECTS)
	@echo $(C_OBJECTS)

asm:
	$(ARMGNU)-objdump -S $(BUILD)output.elf > $(BUILD)kernel.asm

run:
	qemu-system-aarch64 -M raspi3b \
		-serial null -serial chardev:ptydev -chardev pty,id=ptydev\
		-nographic\
		-dtb "bcm2837-rpi-3-b-plus.dtb"\
		-kernel kernel.img -drive file=test.img,if=sd,format=raw

debug:
	qemu-system-aarch64 -M raspi3b \
		-serial null -serial chardev:ptydev -chardev pty,id=ptydev\
		-S -gdb tcp::9000 \
		-dtb "bcm2837-rpi-3-b-plus.dtb"\
		-kernel kernel.img -drive file=test.img,if=sd,format=raw

mount_loopfs:
	mount -o loop test.img /mnt/

# Rule to clean files.
clean : 
	-rm -rf $(BUILD)
	-rm -f $(TARGET)
	-rm -f $(LIST)
	-rm -f $(MAP)



# --------------- DEMO APPLICATIONS --------------

APPS := $(shell find applications/* -maxdepth 0 -type d -printf "%f\n")
APP_FOLDERS := $(shell find applications/* -maxdepth 0 -type d)
APP_BINARIES := $(patsubst %,applications/%/build/%,$(APPS))
APP_COPY := $(patsubst %,/mnt/%,$(APPS))

apps: $(APP_FOLDERS)

$(APP_FOLDERS): .FORCE
	make -C $@

.FORCE:

/mnt/%: applications/%/build/%
	cp -f $< $@

copyapps: $(APP_COPY)
