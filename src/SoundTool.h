#pragma once
#include <bass.h>

//Settings
#define MAXROMSIZE			8388608 //Maximum combined ROM Size in bytes - 8MB
#define MAXWPCROMSIZE		0x80000
#define MAXSUPPORTEDROMS	8		//Maximum number of separate ROM files to be loaded
#define MAXSUPPORTEDBANKS	8		//Maximum number of separate ROM files to be loaded
//#define MAXSAMPLES			256		//Maximum number of individual samples that can be loaded (OKI requires up to 256)
#define MAXSAMPLES		8*256		//Maximum number of individual samples that can be loaded (OKI requires up to 256)

#define BASSFREQUENCY		44200	//44.2KHz is standard on most sound cards, actual playback is lower speed anyway
#define BASSBUFFERSIZE		16000	//Number of samples to keep in the rolling sound buffer
#define TEMPBUFFERSIZE		0x80000 //Size of the temporary buffer to use while decoding samples
#define NUMSPEAKERS			2		//Left + Right

//ID Type Values
#define AutoDetectType 0
#define NoType	0 
#define OKIType 1
#define NECType 2
#define YMZType 4
#define BADType 255

//Universal Style Values
#define NoStyle	0
#define BADStyle 255

//ID Style Values OKI
#define OKI6376 1
#define OKI6295 2
#define WILLIAMSOKI 4

//ID Style Values YMZ280B
#define Scorp2Style 1
#define Epoch1Style 2
#define Epoch2Style 4
#define Scorp4Style 8
#define Scorp5Style 16

//Decode Values - YMZ280B only
#define Decode4ADPCM 0	//4 bit ADPCM
#define Decode16PCM 1	//16 bit linear PCM
#define Decode8PCM 2	//8 bit linear PCM

class SndTool {
protected:
	
public:
		
	SndTool();				//Constructor
	~SndTool();				//Destructor
		
	bool UpdateBass();		//Update the sound buffers, must be run regularly

	bool PlaySample(int);	//Starts Playback (Sample Number)
	bool StopSample();	 	//Stops Playback
	bool PauseSample();	 	//Pause / Unpause Playback
	bool SaveSample(int);	 	//Save Sample to WAV
	bool SaveSampleRAW(int);	//Save Sample as RAW bytes
	void SetLoop(bool);

	bool LoadSoundROMs(int, int);
	bool LoadWilliamsSoundROMs();
	//Call this to Load ROMs, must add ROM file path strings first. (Identity Type, Identity Style) Type set to 0 is automatic detection
	
	//ROM String storage functions
	bool DeleteROMString(int);		//Unload ROM file path string (index)
	bool DeleteWROMString(int, bool U12);		//Unload ROM file path string (index)
	bool ClearROMStrings(bool);		//Clears all ROM file path strings (clear ROMsToLoad value)
	bool ClearWROMStrings(bool, bool U12);		//Clears all ROM file path strings (clear ROMsToLoad value)
	wchar_t* GetROMString(int);		//Returns the string in (index);	
	wchar_t * GetWROMString(int Index, bool U12);
	int GetFileNames();				//Calls the windows common dialog
	int GetU12FileNames();
	int GetU13FileNames();
	bool AddROMString(wchar_t*);	//Adds a ROM file path string to storage (Wide String)

	bool AddWROMString(wchar_t * InString, bool U12);
	bool Convert();

	bool Convert(int rate);

private:

	//Clear current info and free buffers if neccessary
	bool ClearInfo();
	bool LoadWilliams();
	//Load ROM to memory
	bool Load();
	//Identify ROM
	bool Identify(int, int); //Forcing options (ID Type, ID Style)
	bool ConvertWilliams();
	//Convert ROM to ADPCM

	//Sound Stream
	HSTREAM StreamHandle;
	

	//Identity
	UINT8 IDType, IDStyle;
	
	

	//Load ROMs Data
	wchar_t * pszFileROMPath[MAXSUPPORTEDROMS];
	INT8 ROMsToLoad;

	wchar_t * pszFileU12ROMPath[MAXSUPPORTEDROMS];
	INT8 U12ROMsToLoad;

	wchar_t * pszFileU13ROMPath[MAXSUPPORTEDROMS];
	INT8 U13ROMsToLoad;

	//ROM Storage
	UINT8 Memory_Space[MAXROMSIZE];		//8MB ROM Space
	UINT8 U12Memory_Space[MAXWPCROMSIZE];		
	UINT8 U13Memory_Space[MAXWPCROMSIZE];
	UINT8 CommonBank[MAXWPCROMSIZE];
	UINT8 Bank_Space[MAXSUPPORTEDBANKS][MAXWPCROMSIZE];
	INT16 * Sample_Space[MAXSAMPLES];	//Sample Data
	INT32 TotalSize;					//Total size of combined ROMS	
	INT32 U12TotalSize;					//Total size of combined ROMS	
	INT32 U13TotalSize;					//Total size of combined ROMS	

	INT32 ROMSize[MAXSUPPORTEDROMS];
	INT32 TableOffset[MAXSUPPORTEDROMS];
	
	//Sample Data Tables
	float SampleSeconds[MAXSAMPLES];		//Sample Length in Seconds
	UINT8 SampleBank[MAXSAMPLES];			//Bank Sample is located in
	UINT8 SampleIndex[MAXSAMPLES];			//Index of Sample within current bank
	UINT32 SampleStart[MAXSAMPLES];			//Sample Start Location (Hex)
	UINT32 SampleEnd[MAXSAMPLES];			//Sample End Location (Hex)
	UINT32 SampleStartCur[MAXSUPPORTEDBANKS][MAXSAMPLES];
	UINT32 SampleStartBank[MAXSUPPORTEDBANKS][MAXSAMPLES];			//Sample Start Location (Hex)
	UINT32 SampleEndBank[MAXSUPPORTEDBANKS][MAXSAMPLES];			//Sample End Location (Hex)

	UINT32 SampleRate[MAXSAMPLES];			//Sample Rate (Hz)
	UINT32 SampleRateDivisor[MAXSAMPLES];	//Divisor of chip input sample rate (OKI Only I think)
	UINT32 SampleLengthBytes[MAXSAMPLES];	//Sample Length in Bytes
	UINT32 SampleLengthSamples[MAXSAMPLES]; //Sample Length in Samples
	UINT32 TotalSamples;					//Total Number of Samples in all ROMs and Banks
	UINT8 OKITables;						//Number of OKI style data tables found in all supplied ROMs

	bool skipbank2 = false;
	bool skipbank67 = false;
	//OKI ADPCM Conversion		
	INT32 ADPCMIndex, ADPCMLast;
	INT16 OKIDecodeNibble(UINT8);

	//NEC
	INT16 NECDecodeNibble(UINT8);
	UINT8 TuneLookup[MAXSUPPORTEDROMS][128];
	UINT32 SamplesInPage[MAXSUPPORTEDROMS];
	INT16 *StepNEC;

	//YMZ
	void YMZDecodeNibble(UINT8);		
	char ROMName[73];
	INT16 Signal, Step;
	INT16 Diff_LookUp[16];

	//Playback
	INT16 NowPlaying;		//Sample number now playing
	UINT8 Playing,			//Playing Flag - internal use
		  Looping,			//Loop Sample Y/N
		  EndOfSample;		//End of Sample
	UINT32 SamplePosition;	//Pointer to sample data

	//Bass Audio Library
	int InitBass();		//Initializes the Bass library
	bool StartBass();	//Starts audio stream & buffer
	bool EndBass();		//Stops audio stream & buffer
	bool DestroyBass();	//Removes the bass library and recovers memory
};

