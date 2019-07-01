# ArcadeSoundTool
An ADPCM player with hardware specific elements
The UI is pretty basic, but clicking File/Open will load the main ROM loader. If a board requires ROMs chained together, keep clicking Add and add the Roms in order, first to last.

For OKI 6295, Sample rates are configurable via typing in the Sample rate box - more of this support will be added eventually, but 6295 was the most pressing need.

For the Williams sound board used by Y and T Unit (which uses a 6295 and banking control, use Open Williams to get a load for the U12 and U13 space, so the system can set the banking.

If you have issues with the tool, you may need the BASS library from 
http://www.un4seen.com/, the windows download at the top is all that's needed. The 64 bit DLL for the regular version, the 32 bit for the x86 version. Just place the DLL in the same folder as the EXE, and it'll all run.

Credit where it's due - thanks to Nick Sanders for the original Sound Tool and libraries that have been modified here for changing sample rates and other hardware concerns.

Bugs (Known)
OKI6295 mode (not Williams) will always attempt to find all 127 samples in a ROM, even if the table ends early, which can lead to some weird samples.

Dynamic sample rate changes only work in OKI6295 mode - some chips expect a fixed clock, others do things to it in such a way that a more complex logic is needed to realte a main rate to individual samples.

Most of these have been tested using UK slot machine ROMs that utilise these chips on their sound boards - not all possible play modes have been tested.

TODO:
Add the ability to save out an individual sample as an Audacity 'raw' file
Maybe do the same for WAV files?
Advanced mode: write your own start, finish address and arbitrarily play that section (useful for ROMs where samples have been 'hidden' by removing their pointers from the table.
(Possibly as another program) Rudimentary 6295 and family editor that can add a sample to ROM, and adjust related pointers to fit automatically.
