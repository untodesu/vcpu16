mov $main, %pc

# r0: the number
# r1: screen offset
# r2: screen pointer
# r3: internal offset [0-5]
# r4: digit character
# r5: internal pointer
print_number:
    pts %r0
    pts %r1
    pts %r2
    pts %r3
    pts %r4
    pts %r5
    ior $0x1F01, %r2
    add %r1, %r2
    xor %r3, %r3
print_number_L1:
    ieq $0, %r0
        mov $print_number_L1_end, %pc
    mov %r0, %r4
    mod $10, %r4
    add $'0', %r4
    and $0x00FF, %r4
    bor $0x0F00, %r4
    mov $print_number_A1, %r5
    add %r3, %r5
    inc %r3
    mwr %r4, %r5
    div $10, %r0
    mov $print_number_L1, %pc
print_number_L1_end:
    dec %r3 # we point to the end+1
    mov %r3, %r5
    add %r1, %r5
    inc %r5
    iow %r5, $0x1F02
print_number_L2:
    mov $print_number_A1, %r5
    add %r3, %r5
    mrd %r5, %r4
    mwr %r4, %r2
    inc %r2
    ieq $0, %r3
        mov $print_number_L2_end, %pc
    dec %r3
    mov $print_number_L2, %pc
print_number_L2_end:
    pfs %r5
    pfs %r4
    pfs %r3
    pfs %r2
    pfs %r1
    pfs %r0
    ret
print_number_A1:
    .skip 5

main:
    # the code goes here
hang:
    mov $hang, %pc
