# GT86test2.S - test IO capabilities of
# GT86 display (cleanup and font reset)
    mov $0x0020, %r1
reset:
    iow $0x0001, $0x00AA
    iow $0x0002, $0x00AA
    mov $0x8000, %r0
loop:
    ige $0x9000, %r0
    mov $reset, %pc
    mwr %r1, %r0
    inc %r0
    inc %r1
    mov $loop, %pc
