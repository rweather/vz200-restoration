VZ200 Restoration
=================

This repository chronicles my exploits in restoring a Dick Smith VZ200
that I got off eBay.  The VZ200 is a rebadged VTech Laser 210, which was
sold in Australia from 1983 onwards.

The eBay seller Ben had this to say about the unit: "It was an op-shop
pickup from my Nan and had been in storage in a plastic container for who
knows how long.  As it did not have any leads I never tested it."

## Technical specifications

* Z80A 8-bit CPU, running at 3.58MHz.
* 16K of ROM containing BASIC.
* 2K of video RAM for either 32 x 16 text mode or 128 x 64 graphics mode.
* 6K of user program RAM, expandable to 22K using a 16K RAM expansion module.
* 45 key chiclet keyboard organized in a 6x8 matrix.
* Motorola 6847 video processor.
* RF and composite video output.

Note: The VZ300 is essentially the same computer except that it has 16K of
user program RAM on-board and a mechanical keyboard.

Outside of Australia and New Zealand, the VZ200 and VZ300 were known as the
Laser 210 and Laser 310 respectively.

## Resources

* [Wikipedia article](https://en.wikipedia.org/wiki/VTech_Laser_200)
* [vz200.org](http://vz200.org/)
* [Bushy's VZ200 website](http://www.vz200.org/bushy/)
* [VZ200 Technical Reference Manual](http://www.vz200.org/bushy/VZ200_technical_reference_manual.pdf) from Bushy's website.

## Schematics

I have reproduced the schematics from the VZ200 Technical Reference Manual in
KiCad and generated PDF versions.  The original schematics can be hard to read.

* [VZ200 Motherboard](schematics/VZ200/PDF/VZ200.pdf)
* [Printer Interface](schematics/VZ200_Printer_Interface/PDF/VZ200_Printer_Interface.pdf)
* [16K Memory Expansion Module](schematics/VZ200_16K_RAM_Expansion/PDF/VZ200_16K_RAM_Expansion.pdf)

Note: The motherboard schematic includes components for the PAL encoder
and 6K of Program RAM that were on separate PCB's in the original design.

There is a lot of missing detail in the manual.  Many of the components do
not have designators, so there is no way to know if a resistor was originally
R1, R63, or R128.  The PCB does not have a silkscreen marking the components.
In some cases, the component value is not provided.

For components without a designator, I made up numbers starting at 900;
e.g. R906, C912, U904, etc.  For components without a known value, I listed
the value as "???".  I was able to determine some of the values by inspecting
the actual PCB.

## Restoration

### Initial condition of the unit

There were no power or video cables.  The unit had not been powered on
for a long time.  The power input is 9V DC with a standard DC barrel jack,
so power can be easily supplied with a modern plugpack / wall wart.

The outside of the case was dirty, but nothing that some water, Windex,
and elbow grease wouldn't fix.  The plastic was a little yellowed.

<img alt="Initial Condition Top" src="photos/initial-condition-top.jpg" width="860"/>

<img alt="Initial Condition Bottom" src="photos/initial-condition-bottom.jpg" width="860"/>

Serial number 075172.

<img alt="Serial Number" src="photos/serial-number.png" width="860"/>

The covers for the memory expansion and peripheral ports were still in place,
so I was hopeful that it had never been opened.

<img alt="Expansion Port Covers" src="photos/expansion-port-covers.jpg" width="860"/>

However, once inside I discovered some dirt and the braids connecting the
RF shield to ground had been cut:

<img alt="Cut Braids" src="photos/cut-braids.jpg" width="860"/>

It looks like someone had tried to remove the RF shield before and had
not succeeded.  It is soldered down to the motherboard, but some of the
legs were loose indicating partial success.

### Checking rail-to-rail resistance

I measured the resistance between 5V and ground - 284 ohms.  The same
between 9V and ground.  Probably OK.  Not a dead short anyway.

The resistance between 5V and 9V was 34 ohms, due to an actual 32 ohm
resistor between the two:

<img alt="Power Resistor" src="photos/power-resistor.png" width="860"/>

### Removing the keyboard

I had to remove the keyboard from the top half of the case to clean the
case properly.  It looks like the original keyboard was in a module that
was pushed in from the top and then snapped into place.  Removing the
screws was easy, but I couldn't slide out the keyboard PCB without
cutting the snaps or the cross-piece.  I elected to cut the cross-piece:

<img alt="Keyboard Removal" src="photos/keyboard-removal.jpg" width="860"/>

I'll figure out a replacement later.

The keyboard PCB has scratched-on letters indicating which key is which.
That's nice.

<img alt="Keyboard Letters" src="photos/keyboard-letters.jpg" width="860"/>

After that, it was easy to remove the rubber membrane for cleaning.
The key caps themselves are molded into the membrane, so there are no
individual keys to get lost.

<img alt="Keyboard Membrane Before Cleaning" src="photos/keyboard-membrane-filthy.jpg" width="860"/>

### Removing the RF shield

The person who tried to remove the RF shield didn't do a very good job.
Solder had bridged tracks, and one of the tracks was ripped right off the PCB:

<img alt="Ripped Track" src="photos/ripped-track.jpg" width="860"/>

Using a solder sucker, solder wick, flux, heat, and some patience,
I managed to clean up the excess solder and get the shield off:

<img alt="PCB Image" src="photos/pcb-image.jpg" width="860"/>

Here a version with the major components marked up:

<img alt="PCB Image With Markings" src="photos/pcb-marked-up.jpg" width="860"/>

### Cleaning the case and keyboard membrane

I cleaned the top and bottom of the case and the keyboard membrane
with Windex and water.  Some stubborn stains on the back were cleaned
off with isopropyl alchohol.

<img alt="Top and Keyboard Membrane After Cleaning" src="photos/clean-case-top.jpg" width="860"/>

<img alt="Bottom After Cleaning" src="photos/clean-case-bottom.jpg" width="860"/>

### First power on test

I connected a current-limited bench power supply set to 9V and 1A and
gingerly turned it on.  The 7805 voltage regulator was putting out 4.925V
and the system was consuming 650mA from the 9V supply.  So it looks like
no dead shorts on the board.

No blue smoke escaped, and the power LED turned on, so that's good.

### Checking signals

I checked the signals coming out of the CPU.  The clock on pin 6
was sitting at 3.57797MHz - close enough to 3.58MHz - so that's good.

The address lines all had reasonable looking activity, but about
half of the data lines had weird-looking signals.  Resting half-way
between 0V and 5V.

### Video test

Connecting up a Composite-to-HDMI video converter, I got garbage on
the screen.  Repowering on and off gives me two different images:

<img alt="Bad Screen 1" src="photos/bad-screen-1.jpg" width="860"/>

<img alt="Bad Screen 2" src="photos/bad-screen-2.jpg" width="860"/>

### TBD

More to come ...

## Other documentation

* [VZ200 startup sequence](doc/startup-sequence.md)
* [Useful Memory Locations](doc/useful-locations.md)

## Adapting the schematic

In theory the KiCad schematic could be modernised and then laid out on a
brand new PCB.  But a lot of work would be required to add footprints
and lay everything out.  Some of the components are pretty ancient
and would need modern replacements.

It would be nice to be able to build a brand new VZ200 from modern components.
I haven't attempted to do this, but others are welcome to try.  Here is a
list of things to fix:

* Replace the 6K of program RAM plus the 16K memory expansion module with a
single 32K static RAM chip on-board like the 62256.
* The 2K of video memory is on a separate data and address bus,
so it will probably need special handling.
* Replace the two 8K EPROM's with a modern 16K or 32K EEPROM.
* The MC6847 Video Display Generator (VDG) has additional graphics modes
that were not accessible with the original design.  Some people have
already modified their VZ200's to access these modes.
* The chiclet keyboard is horrible, but a lot of software depends upon
the specific arrangement of buttons in the key matrix.  Consider replacing
with mechanical key switches like on the VZ300, but the same matrix
arrangement.
* If you have FPGA skills, then consider replacing the MC6847 with an
alternative that puts out standard RGB or HDMI video instead of
composite video.
* The original schematic in the technical reference manual did not have
100nF filter capacitors on the major chips.  The actual PCB does have a
small number of 40nF capacitors and some electrolytics scattered around
that were not listed in the original schematic.  I have captured most of
them in my schematic.  Ensure that the voltage rails are properly filtered.
* U13 is a 74LS04 hex inverter with a spare inverter gate, but the input
was not tied to ground or 5V.  There is also a spare XOR gate in the
16K memory expansion module whose inputs aren't tied high or low either.

## Acknowledgements

I was inspired to try my hand at restoring a vintage 1980's computer by the
following YouTubers:

* [Adrian's Digital Basement](https://www.youtube.com/@adriansdigitalbasement)
* [Artic Retro](https://www.youtube.com/@Arcticretro)
* [Noel's Retro Lab](https://www.youtube.com/@NoelsRetroLab)
* [The 8-Bit Guy](https://www.youtube.com/@The8BitGuy)
* [Usagi Electric](https://www.youtube.com/@UsagiElectric)

Artic Retro in particular is an expert in VZ200 / Laser 210 computers.

Thanks for sharing the enthusiam and teaching me so much!

## History

My first computer was a Dick Smith Cat, which was a rebadged Laser 3000.
Mine died a long time ago and I sent it to the dump.  I now wish I hadn't!
The VZ200 I found on eBay is probably the closest I will come to my
beloved Cat - it is a similar vintage.

If anyone has a Dick Smith Cat or a Laser 3000, working or not, and are
willing to part with it, then please contact me via
[email](mailto:rhys.weatherley@gmail.com).

## License

<p xmlns:cc="http://creativecommons.org/ns#" xmlns:dct="http://purl.org/dc/terms/"><span property="dct:title">VZ200 Restoration</span> by <span property="cc:attributionName">Rhys Weatherley</span> is licensed under <a href="http://creativecommons.org/licenses/by-nc-sa/4.0/?ref=chooser-v1" target="_blank" rel="license noopener noreferrer" style="display:inline-block;">Attribution-NonCommercial-ShareAlike 4.0 International<img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/cc.svg?ref=chooser-v1"><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/by.svg?ref=chooser-v1"><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/nc.svg?ref=chooser-v1"><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/sa.svg?ref=chooser-v1"></a></p>

## Contact

For more information on this project, to report bugs, or to suggest
improvements, please contact the author Rhys Weatherley via
[email](mailto:rhys.weatherley@gmail.com).
