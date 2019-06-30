# ArcadeSoundTool
An ADPCM player with hardware specific elements
The UI is pretty basic, but clicking File/Open will load the main ROM loader. If a board requires ROMs chained together, keep clicking Add and add the Roms in order, first to last.

For OKI 6295, Sample rates are configurable via typing in the Sample rate box - more of this support will be added eventually, but 6295 was the most pressing need.

For the Williams sound board used by Y and T Unit (which uses a 6295 and banking control, use Open Williams to get a load for the U12 and U13 space, so the system can set the banking.

If you have issues with the tool, you may need the BASS library from 
http://www.un4seen.com/, the windows download at the top is all that's needed. The 64 bit DLL for the regular version, the 32 bit for the x86 version.
