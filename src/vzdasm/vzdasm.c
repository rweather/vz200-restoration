/*
 * Copyright (C) 2024 Rhys Weatherley
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>

/* Opcode list from https://clrhome.org/table/ */
/*
 * N  - Single-byte operand.
 * NN - Two-byte operand.
 * R  - Single-byte relative operand.
 * D  - Single-byte displacement operand.
 */
static const char * const opcodes_main[256] = {
/*          0               1               2               3               4               5               6               7 */
/* 0x00 */  "nop",          "ld bc,NN",     "ld (bc),a",    "inc bc",       "inc b",        "dec b",        "ld b,N",       "rlca",
/* 0x08 */  "ex af,af'",    "add hl,bc",    "ld a,(bc)",    "dec bc",       "inc c",        "dec c",        "ld c,N",       "rrca",
/* 0x10 */  "djnz R",       "ld de,NN",     "ld (de),a",    "inc de",       "inc d",        "dec d",        "ld d,N",       "rla",
/* 0x18 */  "jr R",         "add hl,de",    "ld a,(de)",    "dec de",       "inc e",        "dec e",        "ld e,N",       "rra",
/* 0x20 */  "jr nz,R",      "ld hl,NN",     "ld (NN),hl",   "inc hl",       "inc h",        "dec h",        "ld h,N",       "daa",
/* 0x28 */  "jr z,R",       "add hl,hl",    "ld hl,(NN)",   "dec hl",       "inc l",        "dec l",        "ld l,N",       "cpl",
/* 0x30 */  "jr nc,R",      "ld sp,NN",     "ld (NN),a",    "inc sp",       "inc (hl)",     "dec (hl)",     "ld (hl),N",    "scf",
/* 0x38 */  "jr c,R",       "add hl,sp",    "ld a,(NN)",    "dec sp",       "inc a",        "dec a",        "ld a,N",       "ccf",
/* 0x40 */  "ld b,b",       "ld b,c",       "ld b,d",       "ld b,e",       "ld b,h",       "ld b,l",       "ld b,(hl)",    "ld b,a",
/* 0x48 */  "ld c,b",       "ld c,c",       "ld c,d",       "ld c,e",       "ld c,h",       "ld c,l",       "ld c,(hl)",    "ld c,a",
/* 0x50 */  "ld d,b",       "ld d,c",       "ld d,d",       "ld d,e",       "ld d,h",       "ld d,l",       "ld d,(hl)",    "ld d,a",
/* 0x58 */  "ld e,b",       "ld e,c",       "ld e,d",       "ld e,e",       "ld e,h",       "ld e,l",       "ld e,(hl)",    "ld e,a",
/* 0x60 */  "ld h,b",       "ld h,c",       "ld h,d",       "ld h,e",       "ld h,h",       "ld h,l",       "ld h,(hl)",    "ld h,a",
/* 0x68 */  "ld l,b",       "ld l,c",       "ld l,d",       "ld l,e",       "ld l,h",       "ld l,l",       "ld l,(hl)",    "ld l,a",
/* 0x70 */  "ld (hl),b",    "ld (hl),c",    "ld (hl),d",    "ld (hl),e",    "ld (hl),h",    "ld (hl),l",    "halt",         "ld (hl),a",
/* 0x78 */  "ld a,b",       "ld a,c",       "ld a,d",       "ld a,e",       "ld a,h",       "ld a,l",       "ld a,(hl)",    "ld a,a",
/* 0x80 */  "add a,b",      "add a,c",      "add a,d",      "add a,e",      "add a,h",      "add a,l",      "add a,(hl)",   "add a,a",
/* 0x88 */  "adc a,b",      "adc a,c",      "adc a,d",      "adc a,e",      "adc a,h",      "adc a,l",      "adc a,(hl)",   "adc a,a",
/* 0x90 */  "sub b",        "sub c",        "sub d",        "sub e",        "sub h",        "sub l",        "sub (hl)",     "sub a",
/* 0x98 */  "sbc a,b",      "sbc a,c",      "sbc a,d",      "sbc a,e",      "sbc a,h",      "sbc a,l",      "sbc a,(hl)",   "sbc a,a",
/* 0xA0 */  "and b",        "and c",        "and d",        "and e",        "and h",        "and l",        "and (hl)",     "and a",
/* 0xA8 */  "xor b",        "xor c",        "xor d",        "xor e",        "xor h",        "xor l",        "xor (hl)",     "xor a",
/* 0xB0 */  "or b",         "or c",         "or d",         "or e",         "or h",         "or l",         "or (hl)",      "or a",
/* 0xB8 */  "cp b",         "cp c",         "cp d",         "cp e",         "cp h",         "cp l",         "cp (hl)",      "cp a",
/* 0xC0 */  "ret nz",       "pop bc",       "jp nz,NN",     "jp NN",        "call nz,NN",   "push bc",      "add a,N",      "rst $00",
/* 0xC8 */  "ret z",        "ret",          "jp z,NN",      NULL,           "call z,NN",    "call NN",      "adc a,N",      "rst $08",
/* 0xD0 */  "ret nc",       "pop de",       "jp nc,NN",     "out (N),a",    "call nc,NN",   "push de",      "sub N",        "rst $10",
/* 0xD8 */  "ret c",        "exx",          "jp c,NN",      "in a,(N)",     "call c,NN",    NULL,           "sbc a,N",      "rst $18",
/* 0xE0 */  "ret po",       "pop hl",       "jp po,NN",     "ex (sp),hl",   "call po,NN",   "push hl",      "and N",        "rst $20",
/* 0xE8 */  "ret pe",       "jp (hl)",      "jp pe,NN",     "ex de,hl",     "call pe,NN",   NULL,           "xor N",        "rst $28",
/* 0xF0 */  "ret p",        "pop af",       "jp p,NN",      "di",           "call p,NN",    "push af",      "or N",         "rst $30",
/* 0xF8 */  "ret m",        "ld sp,hl",     "jp m,NN",      "ei",           "call m,NN",    NULL,           "cp N",         "rst $38",
};

/* Bit instruction opcodes that are prefixed by 0xCB */
static const char * const opcodes_cb[256] = {
/*          0               1               2               3               4               5               6               7 */
/* 0x00 */  "rlc b",        "rlc c",        "rlc d",        "rlc e",        "rlc h",        "rlc l",        "rlc (hl)",     "rlc a",
/* 0x08 */  "rrc b",        "rrc c",        "rrc d",        "rrc e",        "rrc h",        "rrc l",        "rrc (hl)",     "rrc a",
/* 0x10 */  "rl b",         "rl c",         "rl d",         "rl e",         "rl h",         "rl l",         "rl (hl)",      "rl a",
/* 0x18 */  "rr b",         "rr c",         "rr d",         "rr e",         "rr h",         "rr l",         "rr (hl)",      "rr a",
/* 0x20 */  "sla b",        "sla c",        "sla d",        "sla e",        "sla h",        "sla l",        "sla (hl)",     "sla a",
/* 0x28 */  "sra b",        "sra c",        "sra d",        "sra e",        "sra h",        "sra l",        "sra (hl)",     "sra a",
/* 0x30 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x38 */  "srl b",        "srl c",        "srl d",        "srl e",        "srl h",        "srl l",        "srl (hl)",     "srl a",
/* 0x40 */  "bit 0,b",      "bit 0,c",      "bit 0,d",      "bit 0,e",      "bit 0,h",      "bit 0,l",      "bit 0,(hl)",   "bit 0,a",
/* 0x48 */  "bit 1,b",      "bit 1,c",      "bit 1,d",      "bit 1,e",      "bit 1,h",      "bit 1,l",      "bit 1,(hl)",   "bit 1,a",
/* 0x50 */  "bit 2,b",      "bit 2,c",      "bit 2,d",      "bit 2,e",      "bit 2,h",      "bit 2,l",      "bit 2,(hl)",   "bit 2,a",
/* 0x58 */  "bit 3,b",      "bit 3,c",      "bit 3,d",      "bit 3,e",      "bit 3,h",      "bit 3,l",      "bit 3,(hl)",   "bit 3,a",
/* 0x60 */  "bit 4,b",      "bit 4,c",      "bit 4,d",      "bit 4,e",      "bit 4,h",      "bit 4,l",      "bit 4,(hl)",   "bit 4,a",
/* 0x68 */  "bit 5,b",      "bit 5,c",      "bit 5,d",      "bit 5,e",      "bit 5,h",      "bit 5,l",      "bit 5,(hl)",   "bit 5,a",
/* 0x70 */  "bit 6,b",      "bit 6,c",      "bit 6,d",      "bit 6,e",      "bit 6,h",      "bit 6,l",      "bit 6,(hl)",   "bit 6,a",
/* 0x78 */  "bit 7,b",      "bit 7,c",      "bit 7,d",      "bit 7,e",      "bit 7,h",      "bit 7,l",      "bit 7,(hl)",   "bit 7,a",
/* 0x80 */  "res 0,b",      "res 0,c",      "res 0,d",      "res 0,e",      "res 0,h",      "res 0,l",      "res 0,(hl)",   "res 0,a",
/* 0x88 */  "res 1,b",      "res 1,c",      "res 1,d",      "res 1,e",      "res 1,h",      "res 1,l",      "res 1,(hl)",   "res 1,a",
/* 0x90 */  "res 2,b",      "res 2,c",      "res 2,d",      "res 2,e",      "res 2,h",      "res 2,l",      "res 2,(hl)",   "res 2,a",
/* 0x98 */  "res 3,b",      "res 3,c",      "res 3,d",      "res 3,e",      "res 3,h",      "res 3,l",      "res 3,(hl)",   "res 3,a",
/* 0xA0 */  "res 4,b",      "res 4,c",      "res 4,d",      "res 4,e",      "res 4,h",      "res 4,l",      "res 4,(hl)",   "res 4,a",
/* 0xA8 */  "res 5,b",      "res 5,c",      "res 5,d",      "res 5,e",      "res 5,h",      "res 5,l",      "res 5,(hl)",   "res 5,a",
/* 0xB0 */  "res 6,b",      "res 6,c",      "res 6,d",      "res 6,e",      "res 6,h",      "res 6,l",      "res 6,(hl)",   "res 6,a",
/* 0xB8 */  "res 7,b",      "res 7,c",      "res 7,d",      "res 7,e",      "res 7,h",      "res 7,l",      "res 7,(hl)",   "res 7,a",
/* 0xC0 */  "set 0,b",      "set 0,c",      "set 0,d",      "set 0,e",      "set 0,h",      "set 0,l",      "set 0,(hl)",   "set 0,a",
/* 0xC8 */  "set 1,b",      "set 1,c",      "set 1,d",      "set 1,e",      "set 1,h",      "set 1,l",      "set 1,(hl)",   "set 1,a",
/* 0xD0 */  "set 2,b",      "set 2,c",      "set 2,d",      "set 2,e",      "set 2,h",      "set 2,l",      "set 2,(hl)",   "set 2,a",
/* 0xD8 */  "set 3,b",      "set 3,c",      "set 3,d",      "set 3,e",      "set 3,h",      "set 3,l",      "set 3,(hl)",   "set 3,a",
/* 0xE0 */  "set 4,b",      "set 4,c",      "set 4,d",      "set 4,e",      "set 4,h",      "set 4,l",      "set 4,(hl)",   "set 4,a",
/* 0xE8 */  "set 5,b",      "set 5,c",      "set 5,d",      "set 5,e",      "set 5,h",      "set 5,l",      "set 5,(hl)",   "set 5,a",
/* 0xF0 */  "set 6,b",      "set 6,c",      "set 6,d",      "set 6,e",      "set 6,h",      "set 6,l",      "set 6,(hl)",   "set 6,a",
/* 0xF8 */  "set 7,b",      "set 7,c",      "set 7,d",      "set 7,e",      "set 7,h",      "set 7,l",      "set 7,(hl)",   "set 7,a",
};

/* IX instruction opcodes that are prefixed by 0xDD */
static const char * const opcodes_dd[256] = {
/*          0               1               2               3               4               5               6               7 */
/* 0x00 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x08 */  "",             "add ix,bc",    "",             "",             "",             "",             "",             "",
/* 0x10 */  "",             "",             "",             "",             "",             "",             "ld d,N",       "",
/* 0x18 */  "",             "add ix,de",    "",             "",             "",             "",             "",             "",
/* 0x20 */  "",             "ld ix,NN",     "ld (NN),ix",   "inc ix",       "",             "",             "",             "",
/* 0x28 */  "",             "add ix,ix",    "ld ix,(NN)",   "dec ix",       "",             "",             "",             "",
/* 0x30 */  "",             "",             "",             "",             "inc (ix+D)",   "dec (ix+D)",   "ld (ix+D),N",  "",
/* 0x38 */  "",             "add ix,sp",    "",             "",             "",             "",             "",             "",
/* 0x40 */  "",             "",             "",             "",             "",             "",             "ld b,(ix+D)",  "",
/* 0x48 */  "",             "",             "",             "",             "",             "",             "ld c,(ix+D)",  "",
/* 0x50 */  "",             "",             "",             "",             "",             "",             "ld d,(ix+D)",  "",
/* 0x58 */  "",             "",             "",             "",             "",             "",             "ld e,(ix+D)",  "",
/* 0x60 */  "",             "",             "",             "",             "",             "",             "ld h,(ix+D)",  "",
/* 0x68 */  "",             "",             "",             "",             "",             "",             "ld l,(ix+D)",  "",
/* 0x70 */  "ld (ix+D),b",  "ld (ix+D),c",  "ld (ix+D),d",  "ld (ix+D),e",  "ld (ix+D),h",  "ld (ix+D),l",  "",             "ld (ix+D),a",
/* 0x78 */  "",             "",             "",             "",             "",             "",             "ld a,(ix+D)",  "",
/* 0x80 */  "",             "",             "",             "",             "",             "",             "add a,(ix+D)", "",
/* 0x88 */  "",             "",             "",             "",             "",             "",             "adc a,(ix+D)", "",
/* 0x90 */  "",             "",             "",             "",             "",             "",             "sub (ix+D)",   "",
/* 0x98 */  "",             "",             "",             "",             "",             "",             "sbc a,(ix+D)", "",
/* 0xA0 */  "",             "",             "",             "",             "",             "",             "and (ix+D)",   "",
/* 0xA8 */  "",             "",             "",             "",             "",             "",             "xor (ix+D)",   "",
/* 0xB0 */  "",             "",             "",             "",             "",             "",             "or (ix+D)",    "",
/* 0xB8 */  "",             "",             "",             "",             "",             "",             "cp (ix+D)",    "",
/* 0xC0 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xC8 */  "",             "",             "",             NULL,           "",             "",             "",             "",
/* 0xD0 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xD8 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xE0 */  "",             "pop ix",       "",             "ex (sp),ix",   "push ix",      "",             "",             "",
/* 0xE8 */  "",             "jp (ix)",      "",             "",             "",             "",             "",             "",
/* 0xF0 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xF8 */  "",             "ld sp,ix",     "",             "",             "",             "",             "",             "",
};

/* IX bit instruction opcodes that are prefixed by 0xDDCB */
static const char * const opcodes_ddcb[256] = {
/*          0               1               2               3               4               5               6               7 */
/* 0x00 */  "",             "",             "",             "",             "",             "",             "rlc (ix+D)",   "",
/* 0x08 */  "",             "",             "",             "",             "",             "",             "rrc (ix+D)",   "",
/* 0x10 */  "",             "",             "",             "",             "",             "",             "rl (ix+D)",    "",
/* 0x18 */  "",             "",             "",             "",             "",             "",             "rr (ix+D)",    "",
/* 0x20 */  "",             "",             "",             "",             "",             "",             "sla (ix+D)",   "",
/* 0x28 */  "",             "",             "",             "",             "",             "",             "sra (ix+D)",   "",
/* 0x30 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x38 */  "",             "",             "",             "",             "",             "",             "srl (ix+D)",   "",
/* 0x40 */  "",             "",             "",             "",             "",             "",             "bit 0,(ix+D)", "",
/* 0x48 */  "",             "",             "",             "",             "",             "",             "bit 1,(ix+D)", "",
/* 0x50 */  "",             "",             "",             "",             "",             "",             "bit 2,(ix+D)", "",
/* 0x58 */  "",             "",             "",             "",             "",             "",             "bit 3,(ix+D)", "",
/* 0x60 */  "",             "",             "",             "",             "",             "",             "bit 4,(ix+D)", "",
/* 0x68 */  "",             "",             "",             "",             "",             "",             "bit 5,(ix+D)", "",
/* 0x70 */  "",             "",             "",             "",             "",             "",             "bit 6,(ix+D)", "",
/* 0x78 */  "",             "",             "",             "",             "",             "",             "bit 7,(ix+D)", "",
/* 0x80 */  "",             "",             "",             "",             "",             "",             "res 0,(ix+D)", "",
/* 0x88 */  "",             "",             "",             "",             "",             "",             "res 1,(ix+D)", "",
/* 0x90 */  "",             "",             "",             "",             "",             "",             "res 2,(ix+D)", "",
/* 0x98 */  "",             "",             "",             "",             "",             "",             "res 3,(ix+D)", "",
/* 0xA0 */  "",             "",             "",             "",             "",             "",             "res 4,(ix+D)", "",
/* 0xA8 */  "",             "",             "",             "",             "",             "",             "res 5,(ix+D)", "",
/* 0xB0 */  "",             "",             "",             "",             "",             "",             "res 6,(ix+D)", "",
/* 0xB8 */  "",             "",             "",             "",             "",             "",             "res 7,(ix+D)", "",
/* 0xC0 */  "",             "",             "",             "",             "",             "",             "set 0,(ix+D)", "",
/* 0xC8 */  "",             "",             "",             "",             "",             "",             "set 1,(ix+D)", "",
/* 0xD0 */  "",             "",             "",             "",             "",             "",             "set 2,(ix+D)", "",
/* 0xD8 */  "",             "",             "",             "",             "",             "",             "set 3,(ix+D)", "",
/* 0xE0 */  "",             "",             "",             "",             "",             "",             "set 4,(ix+D)", "",
/* 0xE8 */  "",             "",             "",             "",             "",             "",             "set 5,(ix+D)", "",
/* 0xF0 */  "",             "",             "",             "",             "",             "",             "set 6,(ix+D)", "",
/* 0xF8 */  "",             "",             "",             "",             "",             "",             "set 7,(ix+D)", "",
};

/* Misc instruction opcodes that are prefixed by 0xED */
static const char * const opcodes_ed[256] = {
/*          0               1               2               3               4               5               6               7 */
/* 0x00 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x08 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x10 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x18 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x20 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x28 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x30 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x38 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x40 */  "in b,(c)",     "out (c),b",    "sbc hl,bc",    "ld (NN),bc",   "neg",          "retn",         "im 0",         "ld i,a",
/* 0x48 */  "in c,(c)",     "out (c),c",    "adc hl,bc",    "ld bc,(NN)",   "",             "reti",         "",             "ld r,a",
/* 0x50 */  "in d,(c)",     "out (c),d",    "sbc hl,de",    "ld (NN),de",   "",             "",             "im 1",         "ld a,i",
/* 0x58 */  "in e,(c)",     "out (c),e",    "adc hl,de",    "ld de,(NN)",   "",             "",             "im 2",         "ld a,r",
/* 0x60 */  "in h,(c)",     "out (c),h",    "sbc hl,hl",    "",             "",             "",             "",             "rrd",
/* 0x68 */  "in l,(c)",     "out (c),l",    "adc hl,hl",    "",             "",             "",             "",             "rld",
/* 0x70 */  "",             "",             "sbc hl,sp",    "ld (NN),sp",   "",             "",             "",             "",
/* 0x78 */  "in a,(c)",     "out (c),a",    "adc hl,sp",    "ld sp,(NN)",   "",             "",             "",             "",
/* 0x80 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x88 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x90 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x98 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xA0 */  "ldi",          "cpi",          "ini",          "outi",         "",             "",             "",             "",
/* 0xA8 */  "ldd",          "cpd",          "ind",          "outd",         "",             "",             "",             "",
/* 0xB0 */  "ldir",         "cpir",         "inir",         "otir",         "",             "",             "",             "",
/* 0xB8 */  "lddr",         "cpdr",         "indr",         "otdr",         "",             "",             "",             "",
/* 0xC0 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xC8 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xD0 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xD8 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xE0 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xE8 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xF0 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xF8 */  "",             "",             "",             "",             "",             "",             "",             "",
};

/* IY instruction opcodes that are prefixed by 0xFD */
static const char * const opcodes_fd[256] = {
/*          0               1               2               3               4               5               6               7 */
/* 0x00 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x08 */  "",             "add iy,bc",    "",             "",             "",             "",             "",             "",
/* 0x10 */  "",             "",             "",             "",             "",             "",             "ld d,N",       "",
/* 0x18 */  "",             "add iy,de",    "",             "",             "",             "",             "",             "",
/* 0x20 */  "",             "ld iy,NN",     "ld (NN),iy",   "inc iy",       "",             "",             "",             "",
/* 0x28 */  "",             "add iy,iy",    "ld iy,(NN)",   "dec iy",       "",             "",             "",             "",
/* 0x30 */  "",             "",             "",             "",             "inc (iy+D)",   "dec (iy+D)",   "ld (iy+D),N",  "",
/* 0x38 */  "",             "add iy,sp",    "",             "",             "",             "",             "",             "",
/* 0x40 */  "",             "",             "",             "",             "",             "",             "ld b,(iy+D)",  "",
/* 0x48 */  "",             "",             "",             "",             "",             "",             "ld c,(iy+D)",  "",
/* 0x50 */  "",             "",             "",             "",             "",             "",             "ld d,(iy+D)",  "",
/* 0x58 */  "",             "",             "",             "",             "",             "",             "ld e,(iy+D)",  "",
/* 0x60 */  "",             "",             "",             "",             "",             "",             "ld h,(iy+D)",  "",
/* 0x68 */  "",             "",             "",             "",             "",             "",             "ld l,(iy+D)",  "",
/* 0x70 */  "ld (iy+D),b",  "ld (iy+D),c",  "ld (iy+D),d",  "ld (iy+D),e",  "ld (iy+D),h",  "ld (iy+D),l",  "",             "ld (iy+D),a",
/* 0x78 */  "",             "",             "",             "",             "",             "",             "ld a,(iy+D)",  "",
/* 0x80 */  "",             "",             "",             "",             "",             "",             "add a,(iy+D)", "",
/* 0x88 */  "",             "",             "",             "",             "",             "",             "adc a,(iy+D)", "",
/* 0x90 */  "",             "",             "",             "",             "",             "",             "sub (iy+D)",   "",
/* 0x98 */  "",             "",             "",             "",             "",             "",             "sbc a,(iy+D)", "",
/* 0xA0 */  "",             "",             "",             "",             "",             "",             "and (iy+D)",   "",
/* 0xA8 */  "",             "",             "",             "",             "",             "",             "xor (iy+D)",   "",
/* 0xB0 */  "",             "",             "",             "",             "",             "",             "or (iy+D)",    "",
/* 0xB8 */  "",             "",             "",             "",             "",             "",             "cp (iy+D)",    "",
/* 0xC0 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xC8 */  "",             "",             "",             NULL,           "",             "",             "",             "",
/* 0xD0 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xD8 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xE0 */  "",             "pop iy",       "",             "ex (sp),iy",   "push iy",      "",             "",             "",
/* 0xE8 */  "",             "jp (iy)",      "",             "",             "",             "",             "",             "",
/* 0xF0 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0xF8 */  "",             "ld sp,iy",     "",             "",             "",             "",             "",             "",
};

/* IY bit instruction opcodes that are prefixed by 0xFDCB */
static const char * const opcodes_fdcb[256] = {
/*          0               1               2               3               4               5               6               7 */
/* 0x00 */  "",             "",             "",             "",             "",             "",             "rlc (iy+D)",   "",
/* 0x08 */  "",             "",             "",             "",             "",             "",             "rrc (iy+D)",   "",
/* 0x10 */  "",             "",             "",             "",             "",             "",             "rl (iy+D)",    "",
/* 0x18 */  "",             "",             "",             "",             "",             "",             "rr (iy+D)",    "",
/* 0x20 */  "",             "",             "",             "",             "",             "",             "sla (iy+D)",   "",
/* 0x28 */  "",             "",             "",             "",             "",             "",             "sra (iy+D)",   "",
/* 0x30 */  "",             "",             "",             "",             "",             "",             "",             "",
/* 0x38 */  "",             "",             "",             "",             "",             "",             "srl (iy+D)",   "",
/* 0x40 */  "",             "",             "",             "",             "",             "",             "bit 0,(iy+D)", "",
/* 0x48 */  "",             "",             "",             "",             "",             "",             "bit 1,(iy+D)", "",
/* 0x50 */  "",             "",             "",             "",             "",             "",             "bit 2,(iy+D)", "",
/* 0x58 */  "",             "",             "",             "",             "",             "",             "bit 3,(iy+D)", "",
/* 0x60 */  "",             "",             "",             "",             "",             "",             "bit 4,(iy+D)", "",
/* 0x68 */  "",             "",             "",             "",             "",             "",             "bit 5,(iy+D)", "",
/* 0x70 */  "",             "",             "",             "",             "",             "",             "bit 6,(iy+D)", "",
/* 0x78 */  "",             "",             "",             "",             "",             "",             "bit 7,(iy+D)", "",
/* 0x80 */  "",             "",             "",             "",             "",             "",             "res 0,(iy+D)", "",
/* 0x88 */  "",             "",             "",             "",             "",             "",             "res 1,(iy+D)", "",
/* 0x90 */  "",             "",             "",             "",             "",             "",             "res 2,(iy+D)", "",
/* 0x98 */  "",             "",             "",             "",             "",             "",             "res 3,(iy+D)", "",
/* 0xA0 */  "",             "",             "",             "",             "",             "",             "res 4,(iy+D)", "",
/* 0xA8 */  "",             "",             "",             "",             "",             "",             "res 5,(iy+D)", "",
/* 0xB0 */  "",             "",             "",             "",             "",             "",             "res 6,(iy+D)", "",
/* 0xB8 */  "",             "",             "",             "",             "",             "",             "res 7,(iy+D)", "",
/* 0xC0 */  "",             "",             "",             "",             "",             "",             "set 0,(iy+D)", "",
/* 0xC8 */  "",             "",             "",             "",             "",             "",             "set 1,(iy+D)", "",
/* 0xD0 */  "",             "",             "",             "",             "",             "",             "set 2,(iy+D)", "",
/* 0xD8 */  "",             "",             "",             "",             "",             "",             "set 3,(iy+D)", "",
/* 0xE0 */  "",             "",             "",             "",             "",             "",             "set 4,(iy+D)", "",
/* 0xE8 */  "",             "",             "",             "",             "",             "",             "set 5,(iy+D)", "",
/* 0xF0 */  "",             "",             "",             "",             "",             "",             "set 6,(iy+D)", "",
/* 0xF8 */  "",             "",             "",             "",             "",             "",             "set 7,(iy+D)", "",
};

/* BASIC token names */
static const char * const basic_token_names[128] = {
/*          0               1               2               3               4               5               6               7 */
/* 0x80 */  "END",          "FOR",          "RESET",        "SET",          "CLS",          "CMD",          "RANDOM",       "NEXT",
/* 0x88 */  "DATA",         "INPUT",        "DIM",          "READ",         "LET",          "GOTO",         "RUN",          "IF",
/* 0x90 */  "RESTORE",      "GOSUB",        "RETURN",       "REM",          "STOP",         "ELSE",         "COPY",         "COLOR",
/* 0x98 */  "VERIFY",       "DEFINT",       "DEFSNG",       "DEFDBL",       "CRUN",         "MODE",         "SOUND",        "RESUME",
/* 0xA0 */  "OUT",          "ON",           "OPEN",         "FIELD",        "GET",          "PUT",          "CLOSE",        "LOAD",
/* 0xA8 */  "MERGE",        "NAME",         "KILL",         "LSET",         "RSET",         "SAVE",         "SYSTEM",       "LPRINT",
/* 0xB0 */  "DEF",          "POKE",         "PRINT",        "CONT",         "LIST",         "LLIST",        "DELETE",       "AUTO",
/* 0xB8 */  "CLEAR",        "CLOAD",        "CSAVE",        "NEW",          "TAB(",         "TO",           "FN",           "USING",
/* 0xC0 */  "VARPTR",       "USR",          "ERL",          "ERR",          "STRING$",      "INSTR",        "POINT",        "TIME$",
/* 0xC8 */  "MEM",          "INKEY$",       "THEN",         "NOT",          "STEP",         "+",            "-",            "*",
/* 0xD0 */  "/",            "^",            "AND",          "OR",           ">",            "=",            "<",            "SGN",
/* 0xD8 */  "INT",          "ABS",          "FRE",          "INP",          "POS",          "SQR",          "RND",          "LOG",
/* 0xE0 */  "EXP",          "COS",          "SIN",          "TAN",          "ATN",          "PEEK",         "CVI",          "CVS",
/* 0xE8 */  "CVD",          "EOF",          "LOC",          "LOF",          "MKI$",         "MKS$",         "MKD$",         "CINT",
/* 0xF0 */  "CSNG",         "CDBL",         "FIX",          "LEN",          "STR$",         "VAL",          "ASC",          "CHR$",
/* 0xF8 */  "LEFT$",        "RIGHT$",       "MID$",         "'",            "",             "",             "",             "",
};

static unsigned char data[65536];
static size_t data_len = 0;

/* Addresses of token handling routines in the BASIC ROM */
static unsigned token_handler[128];

static unsigned char const header_cartridge[] = {0xAA, 0x55, 0xE7, 0x18};
static unsigned char const header_rom0[] = {0xF3, 0xAF, 0x32, 0x00, 0x68};
static unsigned char const header_rom1[] = {0x2d, 0x38, 0x02, 0x1E};
static unsigned char const header_vzfile1[] = {0x56, 0x5a, 0x46, 0x30};
static unsigned char const header_vzfile2[] = {0x20, 0x20, 0x00, 0x00};

static void disassemble(FILE *out, unsigned address, const unsigned char *data, size_t len);
static void load_basic_tokens(const unsigned char *data);
static void list_basic(FILE *out, const unsigned char *data, size_t len);

static int is_magic(const unsigned char *magic, size_t len)
{
    if (data_len < len) {
        return 0;
    }
    return memcmp(data, magic, len) == 0;
}

int main(int argc, char *argv[])
{
    const char *input;
    unsigned address = 0;
    int unknown = 0;
    FILE *in;

    /* Parse the command-line arguments */
    if (argc < 2) {
        return 1;
    }
    input = argv[1];

    /* Read the input file into memory */
    if ((in = fopen(input, "rb")) == NULL) {
        perror(input);
        return 1;
    }
    data_len = fread(data, 1, sizeof(data), in);
    fclose(in);

    /* Determine what format the file is in from the header */
    if (is_magic(header_cartridge, sizeof(header_cartridge))) {
        /* Looks like a ROM cartridge image */
        address = 0x4000 + sizeof(header_cartridge);
        while (data_len > sizeof(header_cartridge)) {
            /* Strip 0xFF bytes from the end of the ROM image */
            if (data[data_len - 1] != 0xFF)
                break;
            --data_len;
        }
        disassemble(stdout, address, data + sizeof(header_cartridge), data_len - sizeof(header_cartridge));
    } else if (is_magic(header_rom0, sizeof(header_rom0))) {
        /* Looks like the standard VZ U9 ROM at address 0x0000 */
        address = 0x0000;
        load_basic_tokens(data);
        disassemble(stdout, address, data, data_len);
    } else if (is_magic(header_rom1, sizeof(header_rom1))) {
        /* Looks like the standard VZ U10 ROM at address 0x2000 */
        address = 0x2000;
        disassemble(stdout, address, data, data_len);
    } else if ((is_magic(header_vzfile1, sizeof(header_vzfile1)) ||
                is_magic(header_vzfile2, sizeof(header_vzfile2))) && data_len > 24) {
        /* Looks like a ".VZ" file, which is either a BASIC program
         * or an encapsulated machine code program. */
        address = data[22] + (((unsigned)(data[23])) << 8);
        if (data[21] == 0xF0) {
            /* BASIC program */
            list_basic(stdout, data + 24, data_len - 24);
        } else if (data[21] == 0xF1) {
            /* Machine code program */
            disassemble(stdout, address, data + 24, data_len - 24);
        } else {
            unknown = 1;
        }
    } else {
        unknown = 1;
    }
    if (unknown) {
        fprintf(stderr, "%s: unknown file format\n", input);
        return 1;
    }
    return 0;
}

/* Determine the number of operand bytes on the instruction */
static size_t operand_size(const char *insn)
{
    if (strstr(insn, "NN")) {
        return 2;
    }
    if (strstr(insn, "D")) {
        if (strstr(insn, "N")) {
            return 2;
        } else {
            return 1;
        }
    }
    if (strstr(insn, "N")) {
        return 1;
    }
    if (strstr(insn, "R")) {
        return 1;
    }
    return 0;
}

/* Get information about a Z-80 instruction */
static const char *get_insn_info(const unsigned char *data, size_t *op_size, size_t *size)
{
    const char *insn;
    unsigned char opcode;
    size_t opcode_size = 1;
    opcode = data[0];
    insn = opcodes_main[opcode];
    if (!insn) {
        if (opcode == 0xCB) {
            ++opcode_size;
            insn = opcodes_cb[data[1]];
        } else if (opcode == 0xDD) {
            ++opcode_size;
            insn = opcodes_dd[data[1]];
            if (!insn && data[1] == 0xCB) {
                ++opcode_size;
                insn = opcodes_ddcb[data[1]];
            }
        } else if (opcode == 0xFD) {
            ++opcode_size;
            insn = opcodes_fd[data[1]];
            if (!insn && data[1] == 0xCB) {
                ++opcode_size;
                insn = opcodes_fdcb[data[1]];
            }
        } else if (opcode == 0xED) {
            ++opcode_size;
            insn = opcodes_ed[data[1]];
        }
    }
    *op_size = opcode_size;
    if (insn) {
        *size = opcode_size + operand_size(insn);
        return insn;
    } else {
        *size = 1;
        return 0;
    }
}

/* Dump a Z80 instruction */
static void dump_instruction(FILE *out, unsigned address, const char *insn, const unsigned char *data)
{
    size_t count = 0;
    char ch;
    while ((ch = *insn++) != '\0') {
        if (ch == ' ') {
            while (count < 5) {
                fputc(' ', out);
                ++count;
            }
        } else if (ch == 'N' && insn[0] == 'N') {
            fprintf(out, "$%02X%02X", data[1], data[0]);
            data += 2;
            ++insn;
        } else if (ch == 'N') {
            fprintf(out, "$%02X", data[0]);
            data += 1;
        } else if (ch == 'D') {
            fprintf(out, "$%02X", data[0]);
            data += 1;
        } else if (ch == 'R') {
            fprintf(out, "$%04X", address + (int)(signed char)(data[0]));
            data += 1;
        } else if (ch >= 'a' && ch <= 'z') {
            fputc(ch - 'a' + 'A', out);
        } else {
            fputc(ch, out);
        }
        ++count;
    }
}

/* Map a ROM address to the name of the function that is being called */
static const char *map_rom_routine(unsigned address)
{
    switch (address) {
    case 0x0000: return "Reset Vector";
    case 0x0008: return "RST $08";
    case 0x0010: return "RST $10";
    case 0x0018: return "RST $18";
    case 0x0020: return "RST $20";
    case 0x0028: return "RST $28";
    case 0x0030: return "RST $30";
    case 0x0038: return "RST $38 - Interrupt Vector";
    case 0x0049: return "Keyboard Get";
    case 0x0050: return "Screen Get Character";
    case 0x01C9: return "Clear Screen";
    case 0x033A: return "Character Output";
    case 0x058D: return "Print Character";
    case 0x05C4: return "Check Printer Status";
    case 0x06A0: return "Enable Interrupts and Enter BASIC";
    case 0x1A19: return "Enter BASIC";
    case 0x1C90: return "Compare HL and DE";
    case 0x1C96: return "Examine String";
    case 0x1D78: return "Check Next Character";
    case 0x28A7: return "String Output";
    case 0x2EB8: return "Interrupt Handler";
    case 0x2EF4: return "Keyboard Scan";
    case 0x2EFD: return "Keyboard Scan Once";
    case 0x308B: return "Character Output 2";
    case 0x3450: return "Beep";
    case 0x345C: return "Sound Output";
    case 0x3AE2: return "Print CRLF";
    }
    return NULL;
}

/* Gets a BASIC token name from an address */
static const char *get_token_from_address(unsigned address)
{
    int token;
    if (token_handler[0] == 0) {
        return NULL;
    }
    for (token = 128; token < 256; ++token) {
        if (address == token_handler[token - 128]) {
            return basic_token_names[token - 128];
        }
    }
    return NULL;
}

/* Disassemble a block of Z80 instructions */
static void disassemble(FILE *out, unsigned address, const unsigned char *data, size_t len)
{
    const char *insn;
    size_t size, index;
    size_t opcode_size;
    const char *name;
    int prev_was_jump = 0;
    while (len > 0) {
        name = map_rom_routine(address);
        if (!name) {
            name = get_token_from_address(address);
        }
        if (name) {
            printf(";\n");
            printf("; %s\n", name);
            printf(";\n");
        } else if (prev_was_jump) {
            printf(";\n");
        }
        prev_was_jump = 0;
        fprintf(out, "%04X:", address);
        insn = get_insn_info(data, &opcode_size, &size);
        for (index = 0; index < size; ++index) {
            fprintf(out, " %02X", data[index]);
        }
        while (index < 4) {
            fprintf(out, "   ");
            ++index;
        }
        fprintf(out, "     ");
        if ((data[0] == 0xCD || data[0] == 0xC3) && data[2] < 0x40) {
            /* Probably a call into a ROM routine.  Is it one we recognize? */
            unsigned dest = data[1] + (((unsigned)(data[2])) << 8);
            name = map_rom_routine(dest);
            if (!name) {
                name = get_token_from_address(dest);
            }
            dump_instruction(out, address + size, insn, data + opcode_size);
            if (name) {
                fprintf(out, "            ; %s", name);
            }
        } else if (insn && insn[0] != '\0') {
            dump_instruction(out, address + size, insn, data + opcode_size);
        } else {
            fprintf(out, "DB   $%02X", data[0]);
        }
        if (data[0] == 0x18 || data[0] == 0xC3 || data[0] == 0xC9 || data[0] == 0xE9) {
            /* Unconditional jump instruction: "JR", "JP", "RET", or "JP (HL)" */
            prev_was_jump = 1;
        }
        fprintf(out, "\n");
        if (size >= len) {
            break;
        }
        address += size;
        len -= size;
        data += size;
    }
}

/* Loads the addresses of the BASIC token handling routines */
static void load_basic_tokens_inner(unsigned address, int first, int last, const unsigned char *data)
{
    int token;
    const unsigned char *d;
    for (token = first; token <= last; ++token) {
        d = data + address + (token - first) * 2;
        token_handler[token - 128] = d[0] + (((unsigned)(d[1])) << 8);
    }
}
static void load_basic_tokens(const unsigned char *data)
{
    /* Load the token handlers from the two routine pointer tables */
    load_basic_tokens_inner(0x1822, 128, 187, data);
    load_basic_tokens_inner(0x1608, 215, 250, data);
}

/* Lists the contents of a BASIC program */
static void list_basic(FILE *out, const unsigned char *data, size_t len)
{
    unsigned line;
    unsigned next_address = 0;
    unsigned char ch;
    int quoted;
    int last_was_token;
    int first_token;
    while (len >= 4) {
        /* If the address of the next line is zero, then we're at the end
         * of the BASIC program.  There may be machine code following. */
        if (data[0] == 0 && data[1] == 0) {
            data += 2;
            len -= 2;
            if (len > 0) {
                printf(";\n");
                disassemble(out, next_address + 2, data, len);
            }
            break;
        }
        next_address = data[0] + (((unsigned)(data[1])) << 8);

        /* Print the line number */
        line = data[2] + (((unsigned)(data[3])) << 8);
        fprintf(out, "%d ", line);
        data += 4;
        len -= 4;

        /* Dump the tokens on this line */
        quoted = 0;
        last_was_token = 0;
        first_token = 1;
        while (len > 0 && (ch = data[0]) != 0) {
            if (ch == '"') {
                quoted = !quoted;
            }
            if (ch < 0x80) {
                if (last_was_token && ch != 0x20) {
                    fputc(' ', out);
                }
                fputc(ch, out);
                last_was_token = 0;
            } else if (quoted) {
                if (ch < 0xC0) {
                    fputc('~', out);
                } else if (ch < 0xE0) {
                    fputc(ch - 0x80, out);
                } else {
                    fputc(ch - 0xC0, out);
                }
            } else {
                if (!last_was_token && !first_token) {
                    fputc(' ', out);
                } else if (last_was_token && !first_token) {
                    fputc(' ', out);
                }
                fputs(basic_token_names[ch - 0x80], out);
                last_was_token = 1;
            }
            first_token = 0;
            ++data;
            --len;
        }
        if (len > 0) {
            /* Skip the NUL at the end of the line */
            ++data;
            --len;
        }

        /* Terminate the current line */
        fprintf(out, "\n");
    }
}
