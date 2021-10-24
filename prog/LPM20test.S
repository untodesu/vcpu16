# LPM20test.S
# Tests the capabilities of LPM-25

# REGISTER LAYOUT:
#   R0 - LPM-25 text buffer start.
#   R1 - upper limit. upon reaching this limit the program resets the pointer.
#   R2 - scratch pointer.
#   R3 - scratch character.

ior $0x1F01, %R0
mov $0x1000, %R1
add %R0, %R1
xor %R3, %R3

restart:
    mov %R0, %R2

loop:
    ige %R1, %R2
    mov $restart, %PC

    mwr %R3, %R2
    inc %R2
    inc %R3

    mov $loop, %PC
