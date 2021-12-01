# KBtest.S
# Test the capabilities of the keyboard.

start:
    ior $0x1F01, %R4
    mov $on_int, %IA
    sti
hang:
    hlt
    mov $hang, %PC

on_int:
    ieq $0x000F, %R0
    mov $kb_int, %PC
    rfi

kb_int:
    ior $0x000F, %R0
    ior $0x1F02, %R1

    # backspace
    ieq $0xFF01, %R0
    mov $backsp, %PC

    # any other special character
    mov %R0, %R2
    and $0xFF00, %R2
    ieq $0xFF00, %R2
    rfi

    # apply color
    and $0x00FF, %R0
    bor $0x0700, %R0

    # write to the buffer
    mov %R1, %R2
    add %R4, %R1
    mwr %R0, %R1

    # update the cursor
    inc %R2
    iow %R2, $0x1F02

    rfi

backsp:
    xor %R3, %R3

    # nothing to do
    ieq %R3, %R1
    rfi

    dec %R1

    # write null to the buffer
    mov %R1, %R2
    add %R4, %R1
    mwr %R3, %R1

    # update the cursor
    iow %R2, $0x1F02

    rfi
