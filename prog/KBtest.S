# KBtest.S - test keyboard
start:
    mov $handler, %ia
    sti
loop:
    hlt
    mov $loop, %pc

handler:
    ieq $0x000F, %r0
    mov $kb_int, %pc
    rfi

kb_int:
    ior $0x000F, %r0
    ior $0x1F03, %r1
    ieq $0xFF01, %r0
    mov $kb_backspace, %pc
    mov %r0, %r2
    and $0xFF00, %r2
    ieq $0xFF00, %r2
    rfi
    and $0x00FF, %r0
    bor $0x0F00, %r0
    mov %r1, %r2
    add $0x8000, %r1
    mwr %r0, %r1
    inc %r2
    iow %r2, $0x1F03
    rfi

kb_backspace:
    ieq $0, %r1
    rfi
    dec %r1
    mov %r1, %r2
    add $0x8000, %r1
    mwr $0x0000, %r1
    iow %r2, $0x1F03
    rfi
