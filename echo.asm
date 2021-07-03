# echo.asm: copy data using V16EXEC's IO port.
# V16EXEC provides an IO port 0x00FF which
# basically is a fgetc() and fputc() wrapper
IOR $0x00FF, %R0
IEQ $0xFFFF, %R0    # if reached EOF (Ctrl+D)
HLT                 # then die
IOW %R0, $0x00FF
MOV $0x0000, %PC
