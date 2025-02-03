;
; Copyright (C) 2024 Rhys Weatherley
;
; Permission is hereby granted, free of charge, to any person obtaining a
; copy of this software and associated documentation files (the "Software"),
; to deal in the Software without restriction, including without limitation
; the rights to use, copy, modify, merge, publish, distribute, sublicense,
; and/or sell copies of the Software, and to permit persons to whom the
; Software is furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included
; in all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
; OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
; DEALINGS IN THE SOFTWARE.
;

    org $0000

;
; Address of the screen buffer in RAM.
;
vz_screen equ $7000

;
; Scratch area for global variables.
;
scratch             equ $7200
key_name            equ scratch
highest_good        equ scratch+64
ram_size            equ scratch+64+8
interrupt_counter   equ scratch+64+10

;
; Macro to NOP out a vector at the start of ROM.
;
macro rst_nop
    jp  reset
    nop
    nop
    nop
    nop
    nop
endm

;
; RST $00 - Reset Vector.
;
reset:
    di                  ; Disable interrupts until we are ready for them.
    xor a
    ld  ($6800),a       ; Set the video to text mode with a green background.
    jp  start
;
; RST $08
;
    rst_nop
;
; RST $10
;
    rst_nop
;
; RST $18
;
    rst_nop
;
; RST $20
;
    rst_nop
;
; RST $28
;
    rst_nop
;
; RST $30
;
    rst_nop
;
; RST $38 - IRQ vector for the VSYNC interrupt.
;
;
; Save the registers we will be using.
;
    push af
    push de
    push hl
;
; Count the number of interrupts that we have seen.
;
    ld  hl,(interrupt_counter)
    inc hl
    ld  (interrupt_counter),hl
;
; The interrupt is caused by VSYNC which is now low.  We need to wait
; for it to go high again before returning.  Otherwise we immediately
; end up back here.
;
    ld  hl,$68fe
    ld  de,0
wait_for_vsync_high:
    dec de
    ld  a,d
    or  e
    jr  z,vsync_int_fail
    bit 7,(hl)
    jr  z,wait_for_vsync_high
;
; Pop the saved registers and return from the interrupt.
;
    pop hl
    pop de
    pop af
    ei
    reti
;
; We timed out waiting for VSYNC to go high again.  Disable interrupts
; and return with an error state in "interrupt_counter".
;
vsync_int_fail:
    ld  hl,$ffff
    ld  (interrupt_counter),hl
    pop hl
    pop de
    pop af
    di
    reti

;
; Start of the dead test.
;
start:
;
; Set the stack to start at the end of video RAM.  We haven't tested
; program RAM yet, so we cannot use it for stack or scratch.
;
    ld  sp,$7800
;
; Copy the initial test screen from ROM to video RAM.  We also fill up the
; rest of video RAM so that we can check if we can read it back later.
;
    ld  hl,test_screen
    ld  de,vz_screen
    ld  ix,vz_screen+2047
    ld  bc,$0200
copy_test_screen:
    ld  a,(hl)
    cp  $80
    jr  nc,copy_test_screen_2
    and $3f     ; Convert ASCII into inverse characters for the screen.
copy_test_screen_2:
    ld  (de),a
    ld  (ix),a
    inc hl
    inc de
    dec ix
    dec bc
    ld  a,b
    or  c
    jr  nz,copy_test_screen
    ld  bc,$0200
copy_test_screen_3:
    ld  a,(hl)
    ld  (de),a
    ld  (ix),a
    inc hl
    inc de
    dec ix
    dec bc
    ld  a,b
    or  c
    jr  nz,copy_test_screen_3

;
; Beep the speaker.  If nothing is displayed on the screen, then this
; will at least tell us that code is running out of ROM.
;
; This code is based on the sound output routine in the VZ ROM's at $345C.
;
    ld  hl,259          ; Pitch (middle C).
    ld  de,87           ; Duration (1/3rd of a second).
beep_loop:
    ld  a,$01           ; Speaker A on, Speaker B off.
    ld  ($6800),a
    ld  b,h
    ld  c,l
click_delay_1:
    dec bc
    ld  a,b
    or  c
    jr  nz,click_delay_1
;
    ld  a,$20           ; Speaker B on, Speaker A off.
    ld  ($6800),a
    ld  b,h
    ld  c,l
click_delay_2:
    dec bc
    ld  a,b
    or  c
    jr  nz,click_delay_2
    dec de
    ld  a,d
    or  e
    jr  nz,beep_loop
;
; Make sure both sides of the speaker are off at the end.
;
    ld  a,0
    ld  ($6800),a

;
; Test that we can read back what we just put in video RAM.
;
    ld  de,test_screen
    ld  hl,vz_screen
    ld  ix,vz_screen+2047
    ld  bc,$0200
compare_test_screen:
    ld  a,(de)
    cp  $80
    jr  nc,compare_test_screen_2
    and $3f     ; Convert ASCII into inverse characters for the screen.
compare_test_screen_2:
    cp  (hl)
    jr  nz,video_ram_failed
    cp  (ix)
    jr  nz,video_ram_failed
    inc hl
    inc de
    dec ix
    dec bc
    ld  a,b
    or  c
    jr  nz,compare_test_screen
    ld  bc,$0200
compare_test_screen_3:
    ld  a,(de)
    cp  (hl)
    jr  nz,video_ram_failed
    cp  (ix)
    jr  nz,video_ram_failed
    inc hl
    inc de
    dec ix
    dec bc
    ld  a,b
    or  c
    jr  nz,compare_test_screen_3
    ld  ix,vz_screen+3*32+27
    ld  (ix),$20        ; ' '
    ld  (ix+1),$0f      ; 'O'
    ld  (ix+2),$0b      ; 'K'
    jp  video_ram_done
;
video_ram_failed:
;
; Fill the text screen buffer in video RAM with the character set,
; forwards in the top half of the screen and in reverse in the bottom
; half of the screen.  Hopefully this will reveal which bytes or address
; lines are bad.
;
; To be done in the future: Maybe switch to graphics mode so that any
; bad blocks in the rest of video RAM are also visible on-screen.
;
    ld  hl,vz_screen
    ld  ix,vz_screen+511
    ld  a,0
video_ram_fill:
    ld  (hl),a
    ld  (ix),a
    inc hl
    dec ix
    inc a
    jr  nz,video_ram_fill
;
; Write "VRAM BAD" into the top-left corner of the screen.
;
    ld  ix,vz_screen
    ld  (ix),$56        ; 'V'
    ld  (ix+1),$52      ; 'R'
    ld  (ix+2),$41      ; 'A'
    ld  (ix+3),$4D      ; 'M'
    ld  (ix+4),$60      ; ' '
    ld  (ix+5),$42      ; 'B'
    ld  (ix+6),$41      ; 'A'
    ld  (ix+7),$44      ; 'D'
    ld  (ix+8),$60      ; ' '
;
; Switch back and forth between the green and orange background colors to
; indicate that something is wrong with VRAM.  Beeping whenever the screen
; has an orange background color.  No other tests are possible.
;
video_blink_background:
    ld  hl,259          ; Pitch (middle C).
    ld  de,87           ; Duration (1/3rd of a second).
vram_fail1_beep_loop:
    ld  a,$11           ; Speaker A on, Speaker B off, orange background.
    ld  ($6800),a
    ld  b,h
    ld  c,l
vram_fail1_click_delay_1:
    dec bc
    ld  a,b
    or  c
    jr  nz,vram_fail1_click_delay_1
;
    ld  a,$30           ; Speaker B on, Speaker A off, orange background.
    ld  ($6800),a
    ld  b,h
    ld  c,l
vram_fail1_click_delay_2:
    dec bc
    ld  a,b
    or  c
    jr  nz,vram_fail1_click_delay_2
    dec de
    ld  a,d
    or  e
    jr  nz,vram_fail1_beep_loop
;
; Switch to a green background.  We do the same beep loop but this time
; we don't actually beep the speaker.  This keeps the same timing as above.
;
    ld  hl,259          ; Pitch (middle C).
    ld  de,87           ; Duration (1/3rd of a second).
vram_fail2_beep_loop:
    ld  a,$00           ; Speaker A off, Speaker B off, green background.
    ld  ($6800),a
    ld  b,h
    ld  c,l
vram_fail2_click_delay_1:
    dec bc
    ld  a,b
    or  c
    jr  nz,vram_fail2_click_delay_1
;
    ld  a,$00           ; Speaker B off, Speaker A off, green background.
    ld  ($6800),a
    ld  b,h
    ld  c,l
vram_fail2_click_delay_2:
    dec bc
    ld  a,b
    or  c
    jr  nz,vram_fail2_click_delay_2
    dec de
    ld  a,d
    or  e
    jr  nz,vram_fail2_beep_loop
;
; No more tests are possible if video RAM is bad.  Go around again.
;
    jr  video_blink_background
;
video_ram_done:

;
; Zero all of video RAM except the first 512 bytes for the text screen buffer.
; This clears our stack space and scratch memory.
;
    ld  hl,vz_screen+512
    ld  de,vz_screen+513
    ld  bc,$05ff
    ld  (hl),0
    ldir

;
; Check that the VSYNC signal is being delivered regularly via I/O lines.
;
    ld  hl,$68fe
    ld  b,5                 ; Look for 5 pulses and then stop.
wait_for_vsync:
    ld  de,0
wait_for_vsync_1:           ; Wait for a rising edge.
    dec de
    ld  a,d
    or  e
    jr  z,wait_for_vsync_timeout
    bit 7,(hl)
    jr  z,wait_for_vsync_1
    ld  de,0
wait_for_vsync_2:           ; Wait for a falling edge.
    dec de
    ld  a,d
    or  e
    jr  z,wait_for_vsync_timeout
    bit 7,(hl)
    jr  nz,wait_for_vsync_2
    djnz wait_for_vsync
;
; We appear to have regular pulses on the VSYNC line.
;
    ld  ix,vz_screen+4*32+27
    ld  (ix),$20        ; ' '
    ld  (ix+1),$0f      ; 'O'
    ld  (ix+2),$0b      ; 'K'
    jr  test_vsync_int

;
; If we timeout waiting for VSYNC to transition, then we don't have VSYNC.
; Show both VSYNC POLL and VSYNC INTERRUPT as BAD.
;
wait_for_vsync_timeout:
    ld  ix,vz_screen+4*32+27
    ld  (ix),$42        ; 'B'
    ld  (ix+1),$41      ; 'A'
    ld  (ix+2),$44      ; 'D'
    ld  (ix+32),$42     ; 'B'
    ld  (ix+33),$41     ; 'A'
    ld  (ix+34),$44     ; 'D'
    jr  test_ram

;
; Check that the VSYNC signal is being delivered via the IRQ vector.
;
test_vsync_int:
    ld  hl,0
    ld  (interrupt_counter),hl
;
; Enable interrupt mode 1 and turn on interrupts for VSYNC.
;
    im  1
    ei
;
; Wait for some time for interrupts to arrive.  If none arrive, or they
; are too far apart, then VSYNC interrupts are not working correctly.
;
    ld  b,2
wait_for_vsync_int_1:
    ld  de,0
wait_for_vsync_int_2:
    dec de
    ld  a,d
    or  e
    jr  nz,wait_for_vsync_int_2
    djnz wait_for_vsync_int_1
;
; Disable interrupts - we don't need them for the remaining tests.
;
    di
;
; So, what happened?
;
    ld  ix,vz_screen+5*32+27
    ld  hl,(interrupt_counter)
    ld  a,h
    or  l
    jr  z,vsync_int_bad     ; No interrupts occurred at all.
    ld  a,h
    cp  $ff
    jr  z,vsync_int_bad     ; Interrupt pulses were too wide for a VSYNC signal.
;
; Check the counter.  From testing, it was about 59 for 50Hz which should
; make it about 71 for 60Hz.  Allow some wiggle around these values and
; report the actual frequency.
;
    ld  (ix+1),$08          ; 'H'
    ld  (ix+2),$1a          ; 'Z'
    ld  a,l
    cp  57
    jr  c,vsync_freq_incorrect
    cp  61
    jr  c,vsync_freq_50hz
    cp  69
    jr  c,vsync_freq_incorrect
    cp  73
    jr  nc,vsync_freq_incorrect
    ld  (ix-1),$36          ; '6'
    ld  (ix),$30            ; '0'
    jr  test_ram
vsync_freq_50hz:
    ld  (ix-1),$35          ; '5'
    ld  (ix),$30            ; '0'
    jr  test_ram
vsync_freq_incorrect:
    ld  (ix-1),$7f          ; '?'
    ld  (ix),$7f            ; '?'
    jr  test_ram
vsync_int_bad:
    ld  (ix),$42            ; 'B'
    ld  (ix+1),$41          ; 'A'
    ld  (ix+2),$44          ; 'D'

;
; Test the RAM and figure out how much RAM we have.
;
; Start by filling RAM between $7800 and $FFFF with psuedorandom data.
;
; Data is copied from "test_data" 256 bytes at a time.  For each new page,
; the starting position in "test_data" is shifted up by 1.  This seeks to
; avoid correlation between the same bytes on different pages in case there
; is address aliasing going on.
;
test_ram:
    ld  ix,$7800
    ld  de,0            ; Page shift amount.
    ld  c,$78
next_page:
    ld  hl,test_data
    add hl,de
    inc de
    ld  b,0
fill_page:
    ld  a,(hl)
    ld  (ix),a
    inc hl
    inc ix
    djnz fill_page
    inc c
    jr  nz,next_page
;
; Read back the data and compare with the original.  We do this bit by bit
; so that we can figure out which bits are working and which ones are not.
;
macro test_ram,pass,bit_mask
    ld  ix,$7800
    ld  de,0            ; Page shift amount.
    ld  c,$78
test_next_page##pass:
    ld  hl,test_data
    add hl,de
    inc de
    ld  b,0
test_fill_page##pass:
    ld  a,(ix)
    xor (hl)
    and bit_mask
    jr  nz,test_page_done##pass
    inc hl
    inc ix
    djnz test_fill_page##pass
    inc c
    jr  nz,test_next_page##pass
test_page_done##pass:
;
; C is now the highest address that is good on this bit.  If C is still
; $78 then the bit is completely dead across the entire memory range.
;
    ld  a,c
    sub $78
    ld  (highest_good+pass),a
    ld  de,(ram_size)
    cp  e
    jr  c,test_ram_bit_done##pass
    ld  e,a
    ld  (ram_size),de
test_ram_bit_done##pass:
endm
    ld  de,0
    ld  (ram_size),de
    test_ram 0,$01
    test_ram 1,$02
    test_ram 2,$04
    test_ram 3,$08
    test_ram 4,$10
    test_ram 5,$20
    test_ram 6,$40
    test_ram 7,$80
;
; Report which bits are alive or dead.
;
macro report_ram,pass,result
    ld  ix,result
    ld  a,(highest_good+pass)
    or  a
    jr  z,report_ram_bad##pass
    cp  (hl)
    jr  c,report_ram_bad##pass
    ld  (ix),$20        ; ' '
    ld  (ix+1),$0f      ; 'O'
    ld  (ix+2),$0b      ; 'K'
    jr  report_ram_done##pass
report_ram_bad##pass:
    ld  (ix),$42        ; 'B'
    ld  (ix+1),$41      ; 'A'
    ld  (ix+2),$44      ; 'D'
report_ram_done##pass:
endm
    ld  hl,ram_size
    report_ram 0,vz_screen+7*32+11
    report_ram 1,vz_screen+8*32+11
    report_ram 2,vz_screen+9*32+11
    report_ram 3,vz_screen+10*32+11
    report_ram 4,vz_screen+7*32+27
    report_ram 5,vz_screen+8*32+27
    report_ram 6,vz_screen+9*32+27
    report_ram 7,vz_screen+10*32+27
;
; Report the size of the RAM that tested good.
;
    ld  a,(ram_size)
    and $fc
    srl a
    ld  e,a
    ld  d,0
    ld  ix,ram_sizes
    add ix,de
    ld  iy,vz_screen+6*32+27
    ld  (iy),$20
    ld  a,(ix)
    ld  (iy+1),a
    ld  a,(ix+1)
    ld  (iy+2),a

;
; Loop forever until the device is powered off while waiting for keys.
;
; If the user presses a key, then print that key to the screen.
; If the user presses CTRL+BREAK, then restart the tests.
;
    ld  de,$ffff                ; Previous key and CTRL/SHIFT flags.
key_loop:
;
; Get the key that is pressed on the keyboard.
;
;   A   - ASCII code of the base key or 0 if none pressed.
;   B   - Bit 0 is non-zero if CTRL is pressed.  Bit 1 is non-zero for SHIFT.
;
; This routine isn't very space-efficient, but it is easy to understand.
;
    ld  b,0
    ld  a,($68fd)               ; Is CTRL pressed?
    bit 2,a
    jr  nz,get_key_no_ctrl
    set 0,b
get_key_no_ctrl:
    ld  a,($68fb)               ; Is SHIFT pressed?
    bit 2,a
    jr  nz,get_key_no_shift
    set 1,b
get_key_no_shift:
    ld  a,($68fe)               ; Row 1 of the keyboard.
    bit 0,a
    jp  z,key_T
    bit 1,a
    jp  z,key_W
    bit 3,a
    jp  z,key_E
    bit 4,a
    jp  z,key_Q
    bit 5,a
    jp  z,key_R
    ld  a,($68fd)               ; Row 2 of the keyboard.
    bit 0,a
    jp  z,key_G
    bit 1,a
    jp  z,key_S
    bit 3,a
    jp  z,key_D
    bit 4,a
    jp  z,key_A
    bit 5,a
    jp  z,key_F
    ld  a,($68fb)               ; Row 3 of the keyboard.
    bit 0,a
    jp  z,key_B
    bit 1,a
    jp  z,key_X
    bit 3,a
    jp  z,key_C
    bit 4,a
    jp  z,key_Z
    bit 5,a
    jp  z,key_V
    ld  a,($68f7)               ; Row 4 of the keyboard.
    bit 0,a
    jp  z,key_5
    bit 1,a
    jp  z,key_2
    bit 3,a
    jp  z,key_3
    bit 4,a
    jp  z,key_1
    bit 5,a
    jp  z,key_4
    ld  a,($68ef)               ; Row 5 of the keyboard.
    bit 0,a
    jp  z,key_N
    bit 1,a
    jp  z,key_dot
    bit 3,a
    jp  z,key_comma
    bit 4,a
    jp  z,key_space
    bit 5,a
    jp  z,key_M
    ld  a,($68df)               ; Row 6 of the keyboard.
    bit 0,a
    jp  z,key_6
    bit 1,a
    jp  z,key_9
    bit 2,a
    jp  z,key_minus
    bit 3,a
    jp  z,key_8
    bit 4,a
    jp  z,key_0
    bit 5,a
    jp  z,key_7
    ld  a,($68bf)               ; Row 7 of the keyboard.
    bit 0,a
    jp  z,key_Y
    bit 1,a
    jp  z,key_O
    bit 2,a
    jp  z,key_return
    bit 3,a
    jp  z,key_I
    bit 4,a
    jp  z,key_P
    bit 5,a
    jp  z,key_U
    ld  a,($687f)               ; Row 8 of the keyboard.
    bit 0,a
    jp  z,key_H
    bit 1,a
    jp  z,key_L
    bit 2,a
    jp  z,key_colon
    bit 3,a
    jp  z,key_K
    bit 4,a
    jp  z,key_semi
    bit 5,a
    jp  z,key_J
    xor a
    jp  got_key
key_T:
    ld  a,'T'
    jp  got_key
key_W:
    ld  a,'W'
    jp  got_key
key_E:
    ld  a,'E'
    jp  got_key
key_Q:
    ld  a,'Q'
    jp  got_key
key_R:
    ld  a,'R'
    jp  got_key
key_G:
    ld  a,'G'
    jp  got_key
key_S:
    ld  a,'S'
    jp  got_key
key_D:
    ld  a,'D'
    jp  got_key
key_A:
    ld  a,'A'
    jp  got_key
key_F:
    ld  a,'F'
    jp  got_key
key_B:
    ld  a,'B'
    jp  got_key
key_X:
    ld  a,'X'
    jp  got_key
key_C:
    ld  a,'C'
    jp  got_key
key_Z:
    ld  a,'Z'
    jp  got_key
key_V:
    ld  a,'V'
    jp  got_key
key_5:
    ld  a,'5'
    jp  got_key
key_2:
    ld  a,'2'
    jp  got_key
key_3:
    ld  a,'3'
    jp  got_key
key_1:
    ld  a,'1'
    jp  got_key
key_4:
    ld  a,'4'
    jp  got_key
key_N:
    ld  a,'N'
    jp  got_key
key_dot:
    ld  a,'.'
    jp  got_key
key_comma:
    ld  a,','
    jp  got_key
key_space:
    ld  a,' '
    jp  got_key
key_M:
    ld  a,'M'
    jp  got_key
key_6:
    ld  a,'6'
    jp  got_key
key_9:
    ld  a,'9'
    jp  got_key
key_minus:
    ld  a,'-'
    jp  got_key
key_8:
    ld  a,'8'
    jp  got_key
key_0:
    ld  a,'0'
    jp  got_key
key_7:
    ld  a,'7'
    jp  got_key
key_Y:
    ld  a,'Y'
    jp  got_key
key_O:
    ld  a,'O'
    jp  got_key
key_return:
    ld  a,$0d
    jp  got_key
key_I:
    ld  a,'I'
    jp  got_key
key_P:
    ld  a,'P'
    jp  got_key
key_U:
    ld  a,'U'
    jp  got_key
key_H:
    ld  a,'H'
    jp  got_key
key_L:
    ld  a,'L'
    jp  got_key
key_colon:
    ld  a,':'
    jp  got_key
key_K:
    ld  a,'K'
    jp  got_key
key_semi:
    ld  a,';'
    jp  got_key
key_J:
    ld  a,'J'
;
; We now have a key.
;
got_key:
    cp  d               ; Is this the same key as last time?
    jr  nz,print_key
    ld  l,a
    ld  a,b
    cp  e
    jp  z,key_loop      ; Yes, then nothing to do - go back for another key.
    ld  a,l
print_key:
    ld  d,a             ; Save the key that was just pressed / released in DE.
    ld  e,b
    ld  ix,key_name     ; Point to the buffer to fill with the name.
    cp  '-'             ; Is the key CTRL+BREAK?
    jr  nz,not_break
    bit 0,b
    jp  z,not_break
restart:                ; If it is, then restart the test.
    ld  a,($68df)       ; Wait for the BREAK key to be released,
    bit 2,a             ; or we might get stuck in a loop restarting
    jr  z,restart       ; the dead test over and over and over.
    jp  reset
not_break:
;
; Output the base key name.
;
    or  a
    jr  z,no_base_char
    cp  $0d
    jr  nz,not_return
    call copy_string
    db  "NRUTER",0      ; "RETURN" backwards.
    jr  no_base_char
not_return:
    cp  ' '
    jr  nz,not_space
    call copy_string
    db  "ECAPS",0       ; "SPACE" backwards.
    jr  no_base_char
not_space:
    and $3f             ; Convert from ASCII into a graphic character.
    ld  (ix),a
    inc ix
no_base_char:
    bit 1,b
    jr  z,no_shift
    call copy_string
    db  "+TFIHS",0      ; "SHIFT+" backwards.
no_shift:
    bit 0,b
    jr  z,pad
    call copy_string
    db  "+LRTC",0       ; "CTRL+" backwards.
;
; Pad the name to the maximum possible size: "CTRL+SHIFT+RETURN".
;
pad:
    ld  b,17
    ld  a,' '
pad_loop:
    ld  (ix),a
    inc ix
    djnz pad_loop
;
; Show the name of the key on the screen.
;
    ld  ix,key_name
    ld  hl,vz_screen+12*32+29
    ld  b,17
show_key_name_loop:
    ld  a,(ix)
    ld  (hl),a
    inc ix
    dec hl
    djnz show_key_name_loop
;
; Got around again for the next key.
;
    jp  key_loop

;
; Copy a string to (IX) that is pointed to by the return address.
;
copy_string:
    pop hl
copy_string_loop:
    ld  a,(hl)
    inc hl
    and $3f
    jr  z,end_copy_string
    ld  (ix),a
    inc ix
    jr  copy_string_loop
end_copy_string:
    push hl
    ret

;
; Dead test failed in a manner that no further progress is possible.
;
failed:
    jr  failed

;
; Initial test screen image with a blue border and black background.
;
test_screen:
    db  $ae                     ; Line 0
    ds  30,$ac
    db  $ad
    db  $aa                     ; Line 1
    db  "    VZ200/VZ300 DEAD TEST     "
    db  $a5
    db  $aa                     ; Line 2
    db  "                              "
    db  $a5
    db  $aa                     ; Line 3
    db  " VIDEO RAM                ??? "
    db  $a5
    db  $aa                     ; Line 4
    db  " VSYNC SIGNAL             ??? "
    db  $a5
    db  $aa                     ; Line 5
    db  " VSYNC FREQUENCY          ??? "
    db  $a5
    db  $aa                     ; Line 6
    db  " RAM SIZE (KB)            ??? "
    db  $a5
    db  $aa                     ; Line 7
    db  " RAM D0   ???    RAM D4   ??? "
    db  $a5
    db  $aa                     ; Line 8
    db  " RAM D1   ???    RAM D5   ??? "
    db  $a5
    db  $aa                     ; Line 9
    db  " RAM D2   ???    RAM D6   ??? "
    db  $a5
    db  $aa                     ; Line 10
    db  " RAM D3   ???    RAM D7   ??? "
    db  $a5
    db  $aa                     ; Line 11
    db  "                              "
    db  $a5
    db  $aa                     ; Line 12
    db  " KEY                      ??? "
    db  $a5
    db  $aa                     ; Line 13
    db  "                              "
    db  $a5
    db  $aa                     ; Line 14
    db  $20,$20,$20
    db  $8f,$8f,$8f             ; Show all text colours in a row.
    db  $9f,$9f,$9f
    db  $af,$af,$af
    db  $bf,$bf,$bf
    db  $cf,$cf,$cf
    db  $df,$df,$df
    db  $ef,$ef,$ef
    db  $ff,$ff,$ff
    db  $20,$20,$20
    db  $a5
    db  $ab                     ; Line 15
    ds  30,$a3
    db  $a7

;
; Random-ish data for testing RAM contents.
;
test_data:
    ;
    ; 256 bytes from the AES S-box.
    ;
    db $63, $7c, $77, $7b, $f2, $6b, $6f, $c5, $30, $01, $67, $2b, $fe, $d7, $ab, $76
    db $ca, $82, $c9, $7d, $fa, $59, $47, $f0, $ad, $d4, $a2, $af, $9c, $a4, $72, $c0
    db $b7, $fd, $93, $26, $36, $3f, $f7, $cc, $34, $a5, $e5, $f1, $71, $d8, $31, $15
    db $04, $c7, $23, $c3, $18, $96, $05, $9a, $07, $12, $80, $e2, $eb, $27, $b2, $75
    db $09, $83, $2c, $1a, $1b, $6e, $5a, $a0, $52, $3b, $d6, $b3, $29, $e3, $2f, $84
    db $53, $d1, $00, $ed, $20, $fc, $b1, $5b, $6a, $cb, $be, $39, $4a, $4c, $58, $cf
    db $d0, $ef, $aa, $fb, $43, $4d, $33, $85, $45, $f9, $02, $7f, $50, $3c, $9f, $a8
    db $51, $a3, $40, $8f, $92, $9d, $38, $f5, $bc, $b6, $da, $21, $10, $ff, $f3, $d2
    db $cd, $0c, $13, $ec, $5f, $97, $44, $17, $c4, $a7, $7e, $3d, $64, $5d, $19, $73
    db $60, $81, $4f, $dc, $22, $2a, $90, $88, $46, $ee, $b8, $14, $de, $5e, $0b, $db
    db $e0, $32, $3a, $0a, $49, $06, $24, $5c, $c2, $d3, $ac, $62, $91, $95, $e4, $79
    db $e7, $c8, $37, $6d, $8d, $d5, $4e, $a9, $6c, $56, $f4, $ea, $65, $7a, $ae, $08
    db $ba, $78, $25, $2e, $1c, $a6, $b4, $c6, $e8, $dd, $74, $1f, $4b, $bd, $8b, $8a
    db $70, $3e, $b5, $66, $48, $03, $f6, $0e, $61, $35, $57, $b9, $86, $c1, $1d, $9e
    db $e1, $f8, $98, $11, $69, $d9, $8e, $94, $9b, $1e, $87, $e9, $ce, $55, $28, $df
    db $8c, $a1, $89, $0d, $bf, $e6, $42, $68, $41, $99, $2d, $0f, $b0, $54, $bb, $16
    ;
    ; 256 bytes from the AES inverse S-box.
    ;
    db $52, $09, $6a, $d5, $30, $36, $a5, $38, $bf, $40, $a3, $9e, $81, $f3, $d7, $fb
    db $7c, $e3, $39, $82, $9b, $2f, $ff, $87, $34, $8e, $43, $44, $c4, $de, $e9, $cb
    db $54, $7b, $94, $32, $a6, $c2, $23, $3d, $ee, $4c, $95, $0b, $42, $fa, $c3, $4e
    db $08, $2e, $a1, $66, $28, $d9, $24, $b2, $76, $5b, $a2, $49, $6d, $8b, $d1, $25
    db $72, $f8, $f6, $64, $86, $68, $98, $16, $d4, $a4, $5c, $cc, $5d, $65, $b6, $92
    db $6c, $70, $48, $50, $fd, $ed, $b9, $da, $5e, $15, $46, $57, $a7, $8d, $9d, $84
    db $90, $d8, $ab, $00, $8c, $bc, $d3, $0a, $f7, $e4, $58, $05, $b8, $b3, $45, $06
    db $d0, $2c, $1e, $8f, $ca, $3f, $0f, $02, $c1, $af, $bd, $03, $01, $13, $8a, $6b
    db $3a, $91, $11, $41, $4f, $67, $dc, $ea, $97, $f2, $cf, $ce, $f0, $b4, $e6, $73
    db $96, $ac, $74, $22, $e7, $ad, $35, $85, $e2, $f9, $37, $e8, $1c, $75, $df, $6e
    db $47, $f1, $1a, $71, $1d, $29, $c5, $89, $6f, $b7, $62, $0e, $aa, $18, $be, $1b
    db $fc, $56, $3e, $4b, $c6, $d2, $79, $20, $9a, $db, $c0, $fe, $78, $cd, $5a, $f4
    db $1f, $dd, $a8, $33, $88, $07, $c7, $31, $b1, $12, $10, $59, $27, $80, $ec, $5f
    db $60, $51, $7f, $a9, $19, $b5, $4a, $0d, $2d, $e5, $7a, $9f, $93, $c9, $9c, $ef
    db $a0, $e0, $3b, $4d, $ae, $2a, $f5, $b0, $c8, $eb, $bb, $3c, $83, $53, $99, $61
    db $17, $2b, $04, $7e, $ba, $77, $d6, $26, $e1, $69, $14, $63, $55, $21, $0c, $7d 

;
; Strings for showing the RAM size.
;
ram_sizes:
    db  " 0 1 2 3 4 5 6 7"
    db  " 8 9101112131415"
    db  "1617181920212223"
    db  "2425262728293031"
    db  "323334"

;
; Pad the image to 8K.
;
    org $1fff
    nop
    end
