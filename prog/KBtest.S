# KBtest.S - test keyboard
mov $main, %pc

handler:
    ieq $0x000F, %r0
    mov $kb_int, %pc
    rfi

kb_int:
    ior $0x000F, %r2
    ieq $0xFF01, %r2
    mov $kb_backspace, %pc
    mov %r2, %r3
    and $0xFF00, %r3
    ieq $0xFF00, %r3
    rfi
    and $0x00FF, %r2
    bor $0x0F00, %r2
    mwr %r2, %r1
    inc %r1
    rfi

kb_backspace:
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
