# Memory management

These files and modules are concerned with all aspects of memory management, except for allocation.
This includes virtual memory and process address spaces for example.

## Virtual Address Space

* `0x0000'0000'0000'0000` -  `0x0000'0000'4000'0000`
1GiB identity mapping during early boot
* `0x0000'8000'0000'0000` -  `0x0000'8000'4000'0000`
1GiB mapping of physical ram
* `0x0000'9000'0000'0000` -  `0x0000'A000'0000'0000`
16 TiB of vmalloc mappings (each individual allocation is up to 4 GiB in size)
* `0x0000'A000'0000'0000` -  `0x0000'B000'0000'0000`
16 TiB of mmap mappings (each individual allocation is up to 4 GiB in size)

## Page Table Structs
To resolve page table entries to virtual kernel addresses, each page table contains 512 * 2 entries. Entry `i + 512` contains the virtual kernel address.