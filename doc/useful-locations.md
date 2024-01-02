VZ200 Useful Locations
======================

This file lists useful memory locations and subroutines on the VZ200.
It is a summary of the data from the manuals, combined with other
information I gleaned along the way.

## 64K Memory Map

* `$0000 - $1FFF` - ROM 0, U9, 8K
* `$2000 - $3FFF` - ROM 1, U10, 8K
* `$4000 - $67FF` - Cartridge ROM, 10K
* `$6800 - $6FFF` - I/O space, U1 and U2, 2K
* `$7000 - $77FF` - Video RAM, U7, 2K
* `$7800 - $8FFF` - User Program RAM, 6K
* `$9000 - $FFFF` - Unused in the base model VZ200, used by the 16K
  memory expansion module.

## I/O Space

Due to how address decoding works, writing to any address in the range
`$6800 - $6FFF` will write the same I/O location.  The bits are
arranged as follows:

* Bit 0 - Speaker A
* Bit 1 - Cassette out A.
* Bit 2 - Cassette out B.
* Bit 3 - VDG display mode; 0 = text mode, 1 = graphics mode.
* Bit 4 - VDG background colour.  In text mode, 0 = green, 1 = orange.
  In graphics mode, 0 = green, 1 = buff.
* Bit 5 - Speaker B.  Setting Speaker A and B to opposite bits, and
  then swapping the two bits, will cause the speaker to emit a single click.
  Rapidly swapping back and forth will play a tone.

Any value that is written to `$6800` should also be written to `$783B` in
RAM to let the BASIC interpreter know the most recent modes that were set.

When reading from addresses in the range `$6800 - $6FFF`, the keyboard
matrix is accessible on bits 0 to 5.  The address selects the specific
row of the keyboard matrix to read:

* `$68FE` - Row 0
* `$68FD` - Row 1
* `$68FB` - Row 2
* `$68F7` - Row 3
* `$68EF` - Row 4
* `$68DF` - Row 5
* `$68BF` - Row 6
* `$687F` - Row 7

A bit that reads as 0 corresponds to a pressed key, and a bit that
reads as 1 corresponds to an unpressed key.  Note that
[key ghosting](https://en.wikipedia.org/wiki/Key_rollover#Key_jamming_and_ghosting)
may occur if 3 or more keys are held down at once.

The following bits have a special meaning when I/O addresses are read:

* Bit 6 - State of the cassette input.
* Bit 7 - State of the frame sync signal from the VDG.

## Peripheral Port

The peripheral port on the back of the device makes the 256-byte I/O
bus of the Z-80 CPU available to peripherals.  The manual reserves the
following regions:

* `$00 - $0F` - Printer, output data on `$0E`, read status on `$00`.
* `$10 - $1F` - Floppy Disk Controller
* `$20 - $2F` - Joystick Interface
* `$30 - $3F` - Communications Modem
* `$70 - $7F` - Memory Bank Switch

The ROM's seem to also make use of I/O ports in the range `$40 - $5F`.
Not sure what they are for.

Usually a peripheral also needs ROM support but the peripheral port does
not make the memory bus accessible.  Only peripherals that have support in
the standard ROM's can be connected to the peripheral port.  If the
peripheral needs custom drivers, then it must be connected to the
memory expansion / cartridge port, with the driver in external ROM.

## System Variables in RAM

* `$7800 - $7802` - Handler for `RST $08`, usually a `JP` or `RET` instruction.
* `$7803 - $7805` - Handler for `RST $10`, usually a `JP` or `RET` instruction.
* `$7806 - $7808` - Handler for `RST $18`, usually a `JP` or `RET` instruction.
* `$7809 - $780B` - Handler for `RST $20`, usually a `JP` or `RET` instruction.
* `$780C - $780E` - Handler for `RST $28`, usually a `JP` or `RET` instruction.
* `$780F - $7811` - Handler for `RST $30`, usually a `JP` or `RET` instruction.
* `$783B` - Copy of the output latch; the last value that was written to
  `$6800` in I/O space.
* `$787D - $787F` - Interrupt handler.
* `$788E, $788F` - 16-bit address of the `USR` routine.
* `$789C` - Output device; 0 = screen, 1 = printer, -1 = cassette.
* `$78A0, $78A1` - 16-bit address of the end of BASIC's stack and the
  start of the string variable storage area.
* `$78A4, $78A5` - 16-bit address of the start of the BASIC program.
* `$78A6` - Cursor position.
* `$78B1, $78B2` - 16-bit address of the top of memory.
* `$78F9, $78FA` - 16-bit address of the end of the BASIC program and the
  start of the simple variables table.
* `$78FB, $78FC` - 16-bit address of the start of the dimensioned
  variables table.
* `$7921, $7922` - 16-bit value that was passed to and from the `USR` routine.
* `$79E8` - 8-bit index into the 64-byte BASIC input line buffer.
* `$7AE9` - Default starting location for BASIC programs.

## Useful subroutines in ROM

### 0049 - Keyboard Get

Waits for a character on the keyboard and returns it in A.
Destroys F, BC, DE, and HL.

### 01C9 - Clear Screen

Clears the screen and moves the cursor to the home position.
Destroys AF, BC, DE, and HL.

### 033A - Character Output

Outputs the character in A to the screen.  All registers are preserved.

### 058D - Print Character

Prints the character in C to the printer.  Returns the printed character
in A and C, and destroys F, B, DE, and HL.

Calling this routine with C = 0 will check the printer status in A only.
Carry will be set if BREAK has been pressed.

### 05C4 - Check Printer Status

Returns the printer status byte in A.  If bit 0 is set, the printer is busy.
If bit 0 is not set, the printer is ready.  BC, DE, and HL are preserved.

### 1A19 - BASIC Entry Point

Entry point to the BASIC interpreter.  ROM cartridges can jump to this
location to re-enter BASIC when the cartridge program exits.

### 28A7 - String Output

Writes the NUL-terminated string that is pointed to by HL to the screen.
Destroys AF, BC, DE, and HL.

### 2EF4 - Keyboard Scan

Scans the keyboard matrix and returns the key that is pressed in A.
Or zero if no key is pressed.  Destroys F, BC, DE, and HL.

### 2EFD - Keyboard Scan Once

Another keyboard scanning routine.  No debouncing?  No echo?  TBD

### 3450 - Beep

Beep the speaker.  Destroys AF, BC, and DE.  HL is preserved.

### 345C - Sound Output

Plays a sound on the speaker.  HL contains the pitch and BC contains the
duration.  Destroys AF, BC, DE, and HL.

### 3AE2 - Print CRLF

Sends a CRLF sequence to the printer.  Destroys AF, BC, DE, and HL.
If BREAK was pressed, then the carry flag will be set.

## Memory Bank Switching

[Andrew Laird's 256K RAM Expansion](http://wiki.vz200.org/index.php?title=Andrew_Laird%27s_256K_RAM_Expansion)
writes the bank number 0..15 to output port `$70` to select which 16K
memory bank to make active.
