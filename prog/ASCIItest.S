# ASCIItest.S - test GT86's charset.
# Prints characters 0..255
    mov $0x0700, %r0
    mov $0x8000, %r1
loop:
    igt $0x0FFF, %r0
    mov $end, %pc
    mwr %r0, %r1
    inc %r0
    inc %r1
    mov $loop, %pc
end:
    mov $end, %pc
