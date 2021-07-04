# V16
A small portable DCPU-like virtual machine. 

# Building
## V16 library
The library is fully dependency-free (except for libc) and should be build-able by any compiler.

## V16 utilities
v16asm, v16dasm and v16exec require `getopt.h` header to be present in your system so watch out! MSVC would definitely fail to compile!

## V16 system
v16sys (which is basically a fictional machine emulator) requires SDL2 to be present in your system in order to compile. No getopt (yet) is required!
