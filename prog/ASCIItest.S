# ASCIItest.S - prints the ASCII table
# in seven colors, than freezes.
mov $0x8000, %r0
mov $0x0800, %r1
loop:
    ige $0x0FFF, %r1
    mov $end, %pc
    mwr %r1, %r0
    inc %r0
    inc %r1
    mov $loop, %pc
end:
    mov $end, %pc
