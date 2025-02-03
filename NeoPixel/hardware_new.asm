PIO_A_DATA:     EQU 0x00
PIO_A_CTRL:     EQU 0x01
PIO_B_DATA:     EQU 0x02
PIO_B_CTRL:     EQU 0x03
CTC_C0:         EQU 0x04
CTC_C1:         EQU 0x05
CTC_C2:         EQU 0x06
CTC_C3:         EQU 0x07
SIO_A_DATA:     EQU 0x08
SIO_A_CTRL:     EQU 0x09
SIO_B_DATA:     EQU 0x0a
SIO_B_CTRL:     EQU 0x0b
OUT_A:          EQU 0x0C
IN_A:           EQU 0x0C
OUT_B:          EQU 0x10
IN_B:           EQU 0x10
NEOPIXEL:       EQU 0x1C

SECTION code_user

PUBLIC _neopixel_hw_send
PUBLIC _neopixel_hw_clear
PUBLIC _input_a
PUBLIC _init_hw
PUBLIC _get_elapsed_time
PUBLIC fputc_cons_native
PUBLIC _fputc_cons_native


_input_a:
    in A, (IN_A)
    ld L, A
    ret

_neopixel_hw_send:
    pop     BC  ;return address
    pop     DE  ; lunghezza in E
    pop     HL  ; indirizzo della matrice
    push    HL
    push    DE
    push    BC

    ld A, E

loop_send:
    di

    ld D, (HL)
    inc HL
    ld C, (HL)
    inc HL
    ld E, (HL)
    inc HL

    call send_out

    dec A
    jr NZ, loop_send

    ei

    ret

_neopixel_hw_clear:
    pop     BC  ;return address
    pop     DE  ; lunghezza in E
    push    DE
    push    BC

    ld A, E

loop_clear:
    di

    ld D, 0
    ld C, 0
    ld E, 0

    call send_out

    dec A
    jr NZ, loop_clear

    ei
    ret


; Red   in D
; Green in C
; Blue  in E
; NEOPIXEL IO address
send_out:
    ex AF, AF'

; send green
    ld B, 8
loop_green:
    sla C
    jr NC, green_bit_val_0
    out (NEOPIXEL), A     ; Invia un 1
    jr green_out_bit_val
green_bit_val_0:
    in A, (NEOPIXEL)     ; Invia uno 0
green_out_bit_val:
    djnz loop_green

; send red
    ld B, 8
loop_red:
    sla D
    jr NC, red_bit_val_0
    out (NEOPIXEL), A     ; Invia un 1
    jr red_out_bit_val
red_bit_val_0:
    in A, (NEOPIXEL)     ; Invia uno 0
red_out_bit_val:
    djnz loop_red

; send blue
      ld B, 8
loop_blue:
    sla E
    jr NC, blue_bit_val_0
    out (NEOPIXEL), A     ; Invia un 1
    jr blue_out_bit_val
blue_bit_val_0:
    in A, (NEOPIXEL)     ; Invia uno 0
blue_out_bit_val:
    djnz loop_blue

    ex AF, AF'
    ret


fputc_cons_native:
_fputc_cons_native:
    pop     bc  ;return address
    pop     hl  ;character to print in l
    push    hl
    push    bc
    ld      a,l
    out (SIO_A_DATA), a     
    call tx_emp
    ret

fgetc_cons:
_fgetc_cons:
    ld      l,0     ;Return the result in hl
    ld      h,0
    ret

_init_hw:
    ld a, int_start / 256    ;  Prende il byte più significativo
    ld i, a

    ; Azzera il contatore
    ; ld hl, run_time_counter
    ; ld (hl), 0
    ; inc hl
    ; ld (hl), 0
    ; inc hl
    ; ld (hl), 0
    ; inc hl
    ; ld (hl), 0

    call init_ctc
    call init_sio
    ret

; Inizializzazione SIO
init_sio:
    ; Impostiamo TX and RX per il Canale A

    ; Queste impostazioni sono inviate al canale A
    ld a,00110000b      ; scrittura in WR0: error reset, seleziona WR0
    out (SIO_A_CTRL),a
    ld a,00011000b      ; scrittura in WR0: channel reset
    out (SIO_A_CTRL),a
    ld a,00000100b      ; scrittura in WR0: seleziona WR4
    out (SIO_A_CTRL),a
    ld a,01000100b      ; scrittura in WR4: presc. 16x, 1 stop bit, no parity
    ;ld a,00000100b      ; scrittura in WR4: presc. 1x, 1 stop bit, no parity
    out (SIO_A_CTRL),a
    ld a,00000101b      ; scrittura in WR0: seleziona WR5
    out (SIO_A_CTRL),a
    ld a,11101000b      ; scrittura in WR5: DTR on, TX 8 bits, BREAK off, TX on, RTS off
    out (SIO_A_CTRL),a

    ; Queste impostazioni sono inviate al canale B (valgono per entrambe i canali)
    ld a,00000001b      ; scrittura in WR0: seleziona WR1
    out (SIO_B_CTRL),a
    ld a,00000100b      ; scrittura in WR1: lo stato influisce sull'interrupt vector
    out (SIO_B_CTRL),a
    ld a,00000010b      ; scrittura in WR0: seleziona WR2
    out (SIO_B_CTRL),a
    ld a,0h             ; scrittura in WR2: imposta l'interrupt vector, i bit D3/D2/D1 
                        ; saranno impostati a seconda del canale e delle condizioni che hanno causato l'interrupt stesso
                        ; qui consideriamo:
                        ;    0x0C per ricezione di un carattere sul Canale A
                        ;    0x0E per condizioni speciali (buffer overrun)
    out (SIO_B_CTRL),a
    
    ; Queste impostazioni sono inviate al canale A
    ld a,01h            ; scrittura in WR0: seleziona WR1
    out (SIO_A_CTRL),a
    ld a,00011000b      ; interrupt ad ogni carattere ricevuto; la parità non è una condizione speciale
                        ; buffer overrun è una condizione speciale
    out (SIO_A_CTRL),a

    ; Abilita RX del SIO canale A
    ld a,00000011b      ; scrittura in WR0: seleziona WR3
    out (SIO_A_CTRL),a
    ld a,11000001b      ; 8 bits/RX char; auto enable OFF; RX enable
    out (SIO_A_CTRL),a

    ei

    ret

init_ctc:
    ; Canale 1 per SIO
    ld A, 4Dh  ;  Channel control word   (Counter Mode, CLK pulse starts timer, Time Constant Follows)
    out (CTC_C1), A
    ;ld A, 12     ; Prescaler SIO 16x  9600
    ld A, 6     ; Prescaler SIO 16x  19200
    ;ld A, 2     ; Prescaler SIO 16x  57600
    
    ;ld A, 16     ; Prescaler SIO 1x  115200
    ;ld A, 96     ; Prescaler SIO 1x  19200
    
    out (CTC_C1), A

    ; Canale 0 per interrupt ogni 10 ms
    ld A, 10100101b  ;  Channel control word  Interrupt, prescaler 256    
    out (CTC_C0), A
    ld A, 144     ; Costante di tempo
    out (CTC_C0), A
    ld A, 10h    ; Interrupt control word
    out (CTC_C0), A

    ret

a_ds_rts:
    ex af, af'
    ld a,00000101b      ; scrittura in WR0: seleziona WR5
    out (SIO_A_CTRL),a
    ld a,11101000b      ; 8 bits/TX char; TX enable; RTS disable
    out (SIO_A_CTRL),a
    ex af, af'
    ret

a_en_rts:
    ex af, af'
    ld a,00000101b      ; scrittura in WR0: seleziona WR5
    out (SIO_A_CTRL),a
    ld a,11101010b      ; 8 bits/TX char; TX enable; RTS enable
    out (SIO_A_CTRL),a
    ex af, af'
    ret

tx_emp:
    ; Verifica se il buffer di trasmissione è vuoto
    ex af, af'
    ;sub a
    ;inc a
    ld a, 1
    out (SIO_A_CTRL),a   ; Seleziona il RR1
    in a,(SIO_A_CTRL)
    bit 0,a
    jp z,tx_emp
    ex af, af'
    ret

increment_time:
    exx

    ld    hl, run_time_counter   ; HL punta al primo byte (LSB)
    
    inc   (hl)        ; incrementa il LSB
    jr    nz, increment_time_done    ; se non c’è stato overflow (Z=0) salta all’uscita
    
    inc   hl          ; altrimenti spostati al byte successivo
    inc   (hl)        
    jr    nz, increment_time_done    ; se questo byte non diventa 0, fine
    
    inc   hl
    inc   (hl)
    jr    nz, increment_time_done
    
    inc   hl
    inc   (hl)        ; ultimo byte (MSB)

increment_time_done:
    exx
    ret           

_get_elapsed_time:
    ; Carichiamo in HL i primi 2 byte (LSW)
    ld hl, (run_time_counter)

    ; Carichiamo in DE i byte successivi (MSW)
    ld de, (run_time_counter+2)

    ; Ritorniamo con HL:DE = valore a 32 bit
    ret

run_time_counter:
    defb 0 ; Byte meno significativo (LSB)
    defb 0
    defb 0
    defb 0 ; Byte più significativo (MSB)

; Posiziona il codice che gestisce gli interrupt all'indrizzo allineato in modo da avere
; le tabelle di interrupt agli indirizzi giusti per CTC e SIO
SECTION INTERRUPT
;    ALIGN 0xA000
    ALIGN 256    ; Allinea alla pagina in modo che le due ultime cifre siano 00

int_start:
ctc_time:
    jr ctc_time_routine
cha_rx_int:
    jr cha_rx_int_routine
cha_overrun:
    jr cha_overrun_routine

    defw    0               ; 06
    defw    0               ; 08
    defw    0               ; 0A
    defw    cha_rx_int      ; 0C
    defw    cha_overrun     ; 0E
    defw    ctc_time        ; 10  CTC 0
    defw    0               ; 12  CTC 1
    defw    0               ; 14  CTC 2
    defw    0               ; 16  CTC 3

cha_rx_int_routine:
    ex af, af'     

    call a_ds_rts
    in a,(SIO_A_DATA)    
    out (SIO_A_DATA), a     
    call tx_emp         ; attende l'invio del carattere

    call a_en_rts  

    ex af, af'
    ei
    reti

cha_overrun_routine:
    ei
    reti ; Se c'è un buffer overrrun ignora

ctc_time_routine:
    call increment_time
    ei
    reti