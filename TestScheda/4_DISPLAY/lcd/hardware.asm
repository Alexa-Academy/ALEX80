PORT_A_DATA: EQU 0xc
PORT_B_DATA: EQU 0x10

SECTION code_user

PUBLIC _pio_b_out
PUBLIC fputc_cons_native
PUBLIC _fputc_cons_native

PUBLIC fgetc_cons
PUBLIC _fgetc_cons

_pio_b_out:
fputc_cons_native:
_fputc_cons_native:
    pop     bc  ;return address
    pop     hl  ;character to print in l
    push    hl
    push    bc
    ld      a,l
    out     (PORT_B_DATA),a
    ret

fgetc_cons:
_fgetc_cons:
    in      a,(PORT_A_DATA)
    ld      l,a     ;Return the result in hl
    ld      h,0
    ret