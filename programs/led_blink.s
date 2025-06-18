    movz x1, #0x0000, lsl #0
    movk x1, #0x3f20, lsl #16

    ldr  w3, [x1, #8]

    movz w2, #56
    bic  w3, w3, w2

    movz w2, #8
    orr  w3, w3, w2

    str  w3, [x1, #8]

    movz w2, #0x0000
    movk w2, #0x20, lsl #16

main_loop:

    str w2, [x1, #28]

    movz x4, #0xFFFF
    movk x4, #0x10, lsl #16
delay1:
    subs x4, x4, #1
    b.ne delay1

    str  w2, [x1, #40]

    movz x4, #0xFFFF
    movk x4, #0x10, lsl #16
delay2:
    subs x4, x4, #1
    b.ne delay2
    b main_loop