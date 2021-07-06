# KBtest.S - test keyboard
mov $main, %pc
handler:
    ine $0x000F, %r0
    rfi
    ior $0x000F, %r0
    ieq $0xFF01, %r0
    mov $backspace, %pc
    and $0x00FF, %r0
    bor $0x0F00, %r0
    mwr %r0, %r1
    inc %r1
    rfi

backspace:
    dec %r1
    ilt $0x8000, %r1
    mov $0x8000, %r1
    mwr $0x0000, %r1
    rfi

main:
    mov $0x8000, %r1
    mov $handler, %ia
    sti
end:
    hlt
    mov $end, %pc
