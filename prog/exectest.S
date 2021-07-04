# exectest.S - test v16exec IO port
# The V16Exec utility provides an IO port
# which wraps getchar() and putchar() in C
loop:
    ior $0x00FF, %r0
    ieq $0xFFFF, %r0
    hlt
    iow %r0, $0x00FF
    mov $loop, %pc
