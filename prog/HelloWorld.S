# HelloWorld.S - duh
# Tests LPM20 and assembler directives

ior $0x1F01, %r0
mov $text, %r1

loop:
    mrd %r1, %r2
    and $0x00FF, %r2
    ieq $0x0000, %r2
    mov $end, %pc

    bor $0x0700, %r2
    mwr %r2, %r0

    inc %r0
    inc %r1

    mov $loop, %pc

end:
    # move the cursor
    mov $text_end, %r1
    sub $text, %r1
    iow %r1, $0x1F02
hang:
    mov $hang, %pc
    
text:
    # NOTE: this doesn't support escape sequences
    # so you have to manually put them via .dw or .dat
    .ascii "Hello, wonderful world!"
text_end:
    .dw 0
