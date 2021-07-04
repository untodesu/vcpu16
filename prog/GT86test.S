# GT86test.S - test for GT86 display
# capabilities (rapid flashing at 50 Hz)
        mov $0x0E20, %r0
reset:  mov $0x8000, %r1
loop:   ige $0x9000, %r1
        mov $reset, %pc
        mwr %r0, %r1
        add $1, %r0
        add $1, %r1
        mov $loop, %pc
        