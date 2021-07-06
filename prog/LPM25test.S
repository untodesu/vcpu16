# LPM25test.S - tests LPM25 monitor
mov $0x0000, %r1
restart:
    mov $0x8000, %r0
loop:
    ige $0x9000, %r0
    mov $restart, %pc
    mwr %r1, %r0
    inc %r0
    inc %r1
    mov $loop, %pc
