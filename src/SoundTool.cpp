#include "stdafx.h"
#include "SoundTool.h"
#include "bass.h"
#include <stdio.h>
#include <ShObjIdl.h>
#include "systemclass.h"

SndTool::SndTool(void) {
	//Constructor

	int i;

	//Clear this before InitBass
	StreamHandle = 0;

	//Basss Audio Library
	InitBass();	
	StartBass();
	
	//reset count
	ROMsToLoad = 0;


	//ROM Strings
	for (i = 0; i < MAXSUPPORTEDROMS; i++) {
		pszFileROMPath[i] = 0;
	}

	//Sample Space 
	for (i = 0; i < MAXSAMPLES; i++) {
		Sample_Space[i] = 0;
	}
	
	//Sample Information Table
	ClearInfo();

	//OKI ADPCM Conversion
	ADPCMIndex = 0;
	ADPCMLast = 0;	

	//YMZ ADPCM Conversion
	Signal = 0;
	Step = 0;
	//Lookup Table
	//Diff_Lookup Table
	for (i = 0; i < 16; i++) {
		if (i & 8) {
			Diff_LookUp[i] = (0 - (((i & 7) << 1) + 1));
		}
		else {
			Diff_LookUp[i] = (((i & 7) << 1) + 1);
		}
	}

	//NEC ADPCM Conversion	
	INT16 T[16][16] = { 0, 0, 1, 2, 3, 5, 7, 10, 0, 0, -1, -2, -3, -5, -7, 10,
					 0, 1, 2, 3, 4, 6, 8, 13, 0, -1, -2, -3, -4, -6, -8, -13,
					 0, 1, 2, 4, 5, 7, 10, 15, 0, -1, -2, -4, -5, -7, -10, -15,
					 0, 1, 3, 4, 6, 9, 13, 19, 0, -1, -3, -4, -6, -9, -13, -19,
		 
			 		 0, 2, 3, 5, 8, 11, 15, 23, 0, -2, -3, -5, -8, -11, -15, -23,
					 0, 2, 4, 7, 10, 14, 19, 29, 0, -2, -4, -7, -10, -14, -19, -29,
					 0, 3, 5, 8, 12, 16, 22, 33, 0, -3, -5, -7, -12, -16, -22, -33,
					 1, 4, 7, 10, 15, 20, 29, 43, -1, -4, -7, -10, -15, -20, -29, -43,

					 1, 4, 8, 13, 18, 25, 35, 53, -1, -4, -8, -13, -18, -25, -35, -53,
					 1, 6, 10, 16, 22, 31, 43, 64, -1, -6, -10, -16, -22, -31, -43, -64,
					 2, 7, 12, 19, 27, 37, 51, 76, -2, -7, -12, -19, -27, -37, -51, -76,
					 2, 9, 16, 24, 34, 46, 64, 96, -2, -9, -16, -24, -34, -46, -64, -96,
		 
					 3, 11, 19, 29, 41, 57, 79, 117, -3, -11, -19, -29, -41, -57, -79, -117,
					 4, 13, 24, 36, 50, 69, 96, 143, -4, -13, -24, -36, -50, -69, -96, -143,
					 4, 16, 29, 44, 62, 85, 118, 175, -4, -16, -29, -44, -62, -85, -118, -175,
					 6, 20, 36, 54, 76, 104, 144, 214, -6, -20, -36, -54, -76, -104, -144, -214 };

	StepNEC = (INT16*)malloc(512);
	memcpy(StepNEC, T, 512);

	//Playback Flags
	NowPlaying = 0;
	Playing = 0;
	Looping = 0;
	EndOfSample = 1;
	SamplePosition = 0;

}

SndTool::~SndTool(void) {
	//Destructor
	
	EndBass();
	DestroyBass();

	for (int i = 0; i < MAXSUPPORTEDROMS; i++) {
		//Deallocate memory
		if (pszFileROMPath[ROMsToLoad]) {
			free(pszFileROMPath[ROMsToLoad]);
		}
	}

	//Sample Space 
	for (int i = 0; i < MAXSAMPLES; i++) {
		if (Sample_Space[i]) {
			free(Sample_Space[i]);
		}
	}

	free(StepNEC);
}


int SndTool::InitBass() {
	//Initialize the DLL
	return BASS_Init(-1, BASSFREQUENCY, 0, 0, NULL);
}


bool SndTool::StartBass(){
	//Create Stream
	if (!StreamHandle) {
		//44.2KHz, 2 Speakers, Front Speakers, Rolling Buffer, NULL
		StreamHandle = BASS_StreamCreate(BASSFREQUENCY, NUMSPEAKERS, BASS_SPEAKER_FRONT, STREAMPROC_PUSH, 0);
	}
	//Start Stream
	if (StreamHandle) {
		BASS_ChannelPlay(StreamHandle, true);//true signifies rolling sound buffer
		return true;
	}
	return false;
}


bool SndTool::UpdateBass() {		
	
	INT32 BufferUsed, BufferLength, BufferCount, SampleCount;
	INT16 * StreamBuffer;

	//Get Bytes Used
	BufferUsed = (BASS_ChannelGetData(StreamHandle, 0, BASS_DATA_AVAILABLE) * NUMSPEAKERS);
	if (BufferUsed < BASSBUFFERSIZE) {
		BufferLength = (BASSBUFFERSIZE - BufferUsed);
	}
	else {
		//fclose (DebugFile);
		return false;
	}
	if (BufferUsed <= 0) {
		int help = BASS_ErrorGetCode();
		if (StreamHandle) {
			BASS_ChannelPlay(StreamHandle, TRUE);			
		}
	}
	
	StreamBuffer = (INT16*)malloc((BASSBUFFERSIZE * sizeof(INT16) * NUMSPEAKERS));

	SampleCount = 0;

	if (Playing) {		

		for (BufferCount = 0; BufferCount < BufferLength; BufferCount++) {
			//Buffer Loop
			if (SamplePosition >= SampleLengthSamples[NowPlaying]) {
				//Sample Has Finished
				EndOfSample = 1;

				if (Looping) {
					//Replay
					//Reset Position
					SamplePosition = 0;
					//Reset End Of Sample Flag
					EndOfSample = 0;
				} else {
					//Fill buffer with Zero Data;				
					Playing = 0;
					StreamBuffer[SampleCount] = 0;
					SampleCount++;
					StreamBuffer[SampleCount] = 0;
					SampleCount++;
				}
			} else {

				//Sample Playback
				
				if (Sample_Space[NowPlaying]) {
					//Left Channel
					StreamBuffer[SampleCount] = Sample_Space[NowPlaying][SamplePosition];
				}

				SampleCount++;
				
				if (Sample_Space[NowPlaying]) {
					//Right Channel
					StreamBuffer[SampleCount] = Sample_Space[NowPlaying][SamplePosition];
				}

				SampleCount++;
				SamplePosition++;				
			}
		}

	} else {
		
		//Fill with zero data
		for (BufferCount = 0; BufferCount < (BufferLength); BufferCount++) {
			StreamBuffer[SampleCount] = 0;
			SampleCount ++;
			StreamBuffer[SampleCount] = 0;
			SampleCount ++;
		}

	}

	//Set The Data
	BASS_StreamPutData(StreamHandle, StreamBuffer, (BufferLength * sizeof(INT16) * NUMSPEAKERS));

	//Free audio buffer
	free(StreamBuffer);

	return true;
}


bool SndTool::PlaySample(int Sample) {
	NowPlaying = Sample;
	SamplePosition = 0;
	Playing = 1;	
	EndOfSample = 0;
	BASS_ChannelSetAttribute(StreamHandle, BASS_ATTRIB_FREQ, (float)SampleRate[NowPlaying]);
	return true;
}


bool SndTool::StopSample() {
	NowPlaying = 0;
	Playing = 0;
	EndOfSample = 1;
	return true;
}

bool SndTool::SaveSampleRAW(int Sample) {
	FILE *	tfile = 0;
	//Set up common dialog box
	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY)))
	{
		IFileSaveDialog *pFileSave;

		// Create the FileSaveDialog object.
		if (SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave))))
		{
			// Set the options on the dialog.
			DWORD dwFlags;

			// Before setting, always get the options first in order 
			// not to override existing options.
			if (SUCCEEDED(pFileSave->GetOptions(&dwFlags)))
			{
				// In this case, get shell items only for file system items.				
				if (SUCCEEDED(pFileSave->SetOptions(dwFlags | FOS_FORCEFILESYSTEM |  FOS_PATHMUSTEXIST | FOS_OVERWRITEPROMPT)))
				{
					// Show the Open dialog box.            
					if (SUCCEEDED(pFileSave->Show(NULL)))
					{
						//Get Result from dlg box
						IShellItem  *pItem;
						if (SUCCEEDED(pFileSave->GetResult(&pItem)))
						{
							//Get the file name from the dialog box.
							PWSTR pszFilePath;
							if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
								//Store returned path
								UINT32 StartPos = SampleStart[Sample];
								UINT32 EndPos = SampleEnd[Sample];
								UINT32 SampLength = EndPos - StartPos;
								_wfopen_s(&tfile, pszFilePath, L"wb");
								fwrite(Memory_Space+StartPos, SampLength,1,tfile);
								fclose(tfile);
								CoTaskMemFree(pszFilePath);
								pItem->Release();
							}
						}
					}
				}
			}
			pFileSave->Release();
		}
		CoUninitialize();
	}


/*	TCHAR szFilters[] = _T("All Files (*.*)|*.*||");

	CFileDialog::CFileDialog fileDlg(FALSE, _T("raw"), _T("*.raw"),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	UINT32 StartPos = SampleStart[Sample];
	UINT32 EndPos = SampleEnd[Sample];
	UINT32 SampLength = EndPos - StartPos;

	if (fileDlg.DoModal() == IDOK)
	{
		std::ofstream file;
		file.open(fileDlg.GetPathName(), std::ios_base::binary);
		file.write(Sample_Space[StartPos], SampLength);
	}
	*/
	return true;
}

bool SndTool::PauseSample() {	
	
	Playing ^= 1;
	return true;
}

void SndTool::SetLoop(bool val) {
	Looping = val;
}

bool SndTool::EndBass() {

	if (StreamHandle) {
		//End Stream
		BASS_ChannelStop(StreamHandle);
		//Destroy Stream
		BASS_StreamFree(StreamHandle);
		return true;
	}

	return false;

}


bool SndTool::DestroyBass() {
	//Free Memory	
	return BASS_Free();
}


bool SndTool::LoadSoundROMs(int id_Type, int id_Style) {

	//Clear current info and free buffers if neccessary
	if (ClearInfo()) {
		//Load ROM to memory
		if (Load()) {
			//Identify ROM
			if (Identify(id_Type, id_Style)){
				//Convert ROM to ADPCM
				if (Convert()) {
					//All Went Well
					return true;					
				}				
			}
		}
	}
	return false;
}

bool SndTool::LoadWilliamsSoundROMs() {

	//Clear current info and free buffers if neccessary
	if (ClearInfo()) {
		//Load ROM to memory
		if (LoadWilliams()) {
				//Convert ROM to ADPCM
				if (ConvertWilliams()) {
					//All Went Well
					return true;
				}
		}
	}
	return false;
}

bool SndTool::ClearInfo() {
	
	//Clear current info and free buffers if neccessary
	
	for (int i = 0; i < MAXSAMPLES; i++) {
	
		SampleSeconds[i] = 0.f;
		SampleBank[i] = 0;
		SampleIndex[i] = 0;
		SampleStart[i] = 0;
		SampleEnd[i] = 0;
		SampleRate[i] = 0;
		SampleRateDivisor[i] = 0;
		SampleLengthBytes[i] = 0;
		SampleLengthSamples[i] = 0;
	}

	TotalSamples = 0;

	
	return true;

}

bool SndTool::LoadWilliams() {
	//////////////////////////////////
	//Load Selected ROMs to memory
	//////////////////////////////////

	//Variables
	FILE *	tfile = 0;
	INT8	U12ROMsLeft;
	INT8	U13ROMsLeft;
	UINT8 * buffer = 0;
	UINT32	fileLen, strLength, RNSize;
	wchar_t U12FileName[MAX_LOADSTRING];
	wchar_t U13FileName[MAX_LOADSTRING];


	//Clear the output window
	ApplicationHandle->ClearROMOutput();
	ApplicationHandle->ClearMainOutput();


	//Clear stored sizes of ROMs
	for (int i = 0; i < MAXSUPPORTEDROMS; i++) {
		ROMSize[i] = 0;
	}

	//Clear ROM Memory
	ZeroMemory(U12Memory_Space, MAXWPCROMSIZE);

	//Reset total size
	U12TotalSize = 0;

	//Loop through all U12 ROMs and place them in the U12 Memory_Space
	for (U12ROMsLeft = 0; U12ROMsLeft < U12ROMsToLoad; U12ROMsLeft++) {

		//Get ROM Name
		strLength = lstrlenW(pszFileU12ROMPath[U12ROMsLeft]);

		for (int i = (strLength - 1); i > 0; i--) {
			if (pszFileU12ROMPath[U12ROMsLeft][i] == L'\\') {
				RNSize = (strLength - 1) - i;
				break;
			}
		}

		for (UINT32 i = 0; i <= RNSize; i++) {
			U12FileName[i] = pszFileU12ROMPath[U12ROMsLeft][(strLength - RNSize) + i];
		}
		U12FileName[RNSize] = NULL;

		//Open the file
		_wfopen_s(&tfile, pszFileU12ROMPath[U12ROMsLeft], L"rb");
		if (!tfile) {
			return 0;
		}

		//Get file length
		fseek(tfile, 0, SEEK_END);
		fileLen = ftell(tfile);
		rewind(tfile);

		//Store ROM File Length
		ROMSize[U12ROMsLeft] = fileLen;

		//Allocate memory for temporary buffer
		buffer = (UINT8*)malloc(sizeof(UINT8)*fileLen);
		if (!buffer)
		{
			fclose(tfile);
			return 0;
		}

		//Read file contents into temporary buffer
		fread(buffer, 1, fileLen, tfile);

		ApplicationHandle->SetOutputMsg(L"Found ROM File: ", U12FileName);

		//Close the file
		fclose(tfile);

		//Copy from temporary buffer to main storage
		if ((fileLen > 0) && (fileLen < MAXROMSIZE)) {
			memcpy(U12Memory_Space + U12TotalSize, buffer, fileLen);
		}

		//Deallocate memory
		free(buffer);

		//Increment Total Size
		U12TotalSize += fileLen;
	}
	// If not full, do as hardware does and mirror
	if (U12TotalSize < 524288) {
		ApplicationHandle->SetOutputMsg(L"Mirroring U12, Bank 2 and 3 deactivated (match Common and Bank 0/1 respectively)");
		skipbank2 = true;
		memcpy(U12Memory_Space + U12TotalSize, U12Memory_Space, (524288 - U12TotalSize));
	}

	ZeroMemory(U13Memory_Space, MAXWPCROMSIZE);
	U13TotalSize = 0;

	//Loop through all U13 ROMs and place them in the U13 Memory_Space
	for (U13ROMsLeft = 0; U13ROMsLeft < U13ROMsToLoad; U13ROMsLeft++) {

		//Get ROM Name
		strLength = lstrlenW(pszFileU13ROMPath[U13ROMsLeft]);

		for (int i = (strLength - 1); i > 0; i--) {
			if (pszFileU13ROMPath[U13ROMsLeft][i] == L'\\') {
				RNSize = (strLength - 1) - i;
				break;
			}
		}

		for (UINT32 i = 0; i <= RNSize; i++) {
			U13FileName[i] = pszFileU13ROMPath[U13ROMsLeft][(strLength - RNSize) + i];
		}
		U13FileName[RNSize] = NULL;

		//Open the file
		_wfopen_s(&tfile, pszFileU13ROMPath[U13ROMsLeft], L"rb");
		if (!tfile) {
			return 0;
		}

		//Get file length
		fseek(tfile, 0, SEEK_END);
		fileLen = ftell(tfile);
		rewind(tfile);

		//Store ROM File Length
		ROMSize[U13ROMsLeft] = fileLen;

		//Allocate memory for temporary buffer
		buffer = (UINT8*)malloc(sizeof(UINT8)*fileLen);
		if (!buffer)
		{
			fclose(tfile);
			return 0;
		}

		//Read file contents into temporary buffer
		fread(buffer, 1, fileLen, tfile);

		ApplicationHandle->SetOutputMsg(L"Found ROM File: ", U13FileName);

		//Close the file
		fclose(tfile);

		//Copy from temporary buffer to main storage
		if ((fileLen > 0) && (fileLen < MAXROMSIZE)) {
			memcpy(U13Memory_Space + U13TotalSize, buffer, fileLen);
		}

		//Deallocate memory
		free(buffer);

		//Increment Total Size
		U13TotalSize += fileLen;
	}
	// If not full, do as hardware does and mirror
	if (U13TotalSize < 524288) {
		ApplicationHandle->SetOutputMsg(L"Mirroring U13, Bank 4 and 5 deactivated (match 6 and 7 respectively)");
		skipbank67 = true;
		memcpy(U13Memory_Space + U13TotalSize, U13Memory_Space, (524288 - U13TotalSize));
	}

	memcpy(Memory_Space, U12Memory_Space, 524288);
	memcpy(Memory_Space+524288, U13Memory_Space, 524288);

	//Now do the banking shuffle
	//All banks contain the memory between 0x60000 and 0x80000 - the last 0x20000 of U12.
	//This is where the 'instant access' sounds live

	memcpy(CommonBank, U12Memory_Space + 393216, 131072);
	/*
	U12 0x00000 - 0x7ffff
	U13 0x80000 - 0xfffff
	NOT A BUG, duplication because bank 0's regular address 0x60000 is common bank
	m_okibank->configure_entry(0, &rom[0x40000]); U12
	m_okibank->configure_entry(1, &rom[0x40000]); U12
	m_okibank->configure_entry(2, &rom[0x20000]); U12
	m_okibank->configure_entry(3, &rom[0x00000]); U12
	m_okibank->configure_entry(4, &rom[0xe0000]); U13
	m_okibank->configure_entry(5, &rom[0xc0000]); U13
	m_okibank->configure_entry(6, &rom[0xa0000]); U13
	m_okibank->configure_entry(7, &rom[0x80000]); U13
	*/
	//So, to make the roms back up into files (top 0x20000 of banks):
	//U12 = 3+2+1+COMMON
	//U13 = 7+6+5+4
	//TODO: Move this to somewhere common as reinit func
	memcpy(Bank_Space[0], U12Memory_Space + 0x40000, 0x20000);
		//memcpy(bank_Space[0]+0x20000, CommonBank, 0x20000)
	memcpy(Bank_Space[1], U12Memory_Space + 0x40000, 0x20000);
		//memcpy(bank_Space[1]+0x20000, CommonBank, 0x20000)
//	if (!skipbank2) {
		memcpy(Bank_Space[2], U12Memory_Space + 0x20000, 0x20000);
		//memcpy(bank_Space[2]+0x20000, CommonBank, 0x20000)
		memcpy(Bank_Space[3], U12Memory_Space + 0x00000, 0x20000);
		//memcpy(bank_Space[3]+0x20000, CommonBank, 0x20000)
//	}
//	if (!skipbank67) {
		memcpy(Bank_Space[4], U13Memory_Space + 0x60000, 0x20000);
		//memcpy(bank_Space[4]+0x20000, CommonBank, 0x20000)
		memcpy(Bank_Space[5], U13Memory_Space + 0x40000, 0x20000);
//	}
	//memcpy(bank_Space[5]+0x20000, CommonBank, 0x20000)
	memcpy(Bank_Space[6], U13Memory_Space + 0x20000, 0x20000);
	//memcpy(bank_Space[6]+0x20000, CommonBank, 0x20000)
	memcpy(Bank_Space[7], U13Memory_Space + 0x00000, 0x20000);
	//memcpy(bank_Space[7]+0x20000, CommonBank, 0x20000)
	return true;
}

bool SndTool::Load() {
	//////////////////////////////////
	//Load Selected ROMs to memory
	//////////////////////////////////
	
	//Variables
	FILE *	tfile = 0;
	INT8	ROMsLeft;
	UINT8 * buffer = 0;
	UINT32	fileLen, strLength, RNSize;
	wchar_t FileName[MAX_LOADSTRING];


	//Clear the output window
	ApplicationHandle->ClearROMOutput();
	ApplicationHandle->ClearMainOutput();

	//Reset total size
	TotalSize = 0;

	//Clear stored sizes of ROMs
	for (int i = 0; i < MAXSUPPORTEDROMS; i++) {
		ROMSize[i] = 0;
	}
	
	//Clear ROM Memory
	ZeroMemory(Memory_Space, MAXROMSIZE);

	//Loop through all ROMs and place them in the Memory_Space
	for (ROMsLeft = 0; ROMsLeft < ROMsToLoad; ROMsLeft++) {

		//Get ROM Name
		strLength = lstrlenW(pszFileROMPath[ROMsLeft]);

		for (int i = (strLength - 1); i > 0; i--) {
			if (pszFileROMPath[ROMsLeft][i] == L'\\') {
				RNSize = (strLength - 1) - i;
				break;
			}
		}
		
		for (UINT32 i = 0; i <= RNSize; i++) {
			FileName[i] = pszFileROMPath[ROMsLeft][(strLength - RNSize) + i];
		}
		FileName[RNSize] = NULL;

		//Open the file
		_wfopen_s(&tfile, pszFileROMPath[ROMsLeft], L"rb");
		if (!tfile) {
			return 0;
		}

		//Get file length
		fseek(tfile, 0, SEEK_END);
		fileLen = ftell(tfile);
		rewind(tfile);
//		fseek(tfile, 0, SEEK_SET);

		//Store ROM File Length
		ROMSize[ROMsLeft] = fileLen;

		//Allocate memory for temporary buffer
		buffer = (UINT8*)malloc(sizeof(UINT8)*fileLen);
		if (!buffer)
		{
			fclose(tfile);
			return 0;
		}

		//Read file contents into temporary buffer
		fread(buffer, 1, fileLen, tfile);

		ApplicationHandle->SetOutputMsg(L"Found ROM File: ",FileName);

		//Close the file
		fclose(tfile);

		//Copy from temporary buffer to main storage
		if ((fileLen > 0) && (fileLen < MAXROMSIZE)) {
			memcpy(Memory_Space + TotalSize, buffer, fileLen);
		}

		//Deallocate memory
		free(buffer);

		//Increment Total Size
		TotalSize += fileLen;				
	}

	return true;
}


bool SndTool::Identify(int id_Type, int id_Style) {

	INT32 Offset, cnt, Position, CheckAddr;
	UINT8 CheckVal;
	UINT8 Type[MAXSUPPORTEDROMS];
	UINT8 Style[MAXSUPPORTEDROMS];

	//Identify ROM
	if (id_Type) {
		
		//Forced Identity
		IDType = (1 << (id_Type - 1));
		IDStyle = (1 << id_Style);
		OKITables = 1; //Need to add option toadjust this when forced

		ApplicationHandle->SetOutputMsg(L"ID Type Forced: ", IDType);
		ApplicationHandle->SetOutputMsg(L"ID Style Forced: ", IDStyle);

		ClearROMStrings(true); //Clear ROM strings now that we have loaded and identified the ROMs

		return true;

	} else {

		//Auto Detect Identity

		/*ID Type Values
		AutoDetectType 0
		NoType	0 
		OKIType 1
		NECType 2
		YMZType 4
		BADType 255

		//Universal Style Values
		NoStyle	0
		BADStyle 255

		//ID Style Values OKI
		OKI6376 1
		OKI6295 2
		WILLIAMSOKI 4

		//ID Style Values YMZ280B
		Scorp2Style 1
		Epoch1Style 2
		Epoch2Style 4
		Scorp4Style 8
		Scorp5Style 16
		*/


		//Loop through all roms and try to identify each rom.
		//At the end, roms should either have a single type or no type (similarly for style)
		//As long as types / styles aren't mixed, try and load them.

		OKITables = 0;

		for (int i = 0; i < ROMsToLoad; i++) {
			
			Offset = 0;
			if ((i - 1) >= 0) {
				for (int j = 0; j <= (i - 1); j++) {
					Offset += ROMSize[j];
				}
			}
			Type[i] = BADType;
			Style[i] = NoStyle;
			//Check For NEC
			if (Memory_Space[Offset + 1] == 0x5A) {
				if (Memory_Space[Offset + 2] == 0xA5) {
					if (Memory_Space[Offset + 3] == 0x69) {
						if (Memory_Space[Offset + 4] == 0x55) {
							Type[i] = NECType;
							ApplicationHandle->SetOutputMsg(L"ROM Style Unused: ", L"");
						}
					}
				}
			}
			//Check For OKI 6376
			if ((Memory_Space[Offset] & 0x3f) == 0) {
				if (Memory_Space[Offset + 1] == 0) {
					if (Memory_Space[Offset + 2] == 0) {
						if ((Memory_Space[Offset + 4] & 63) == 0) {
							if (Memory_Space[Offset + 5] == 0x02) {
								if (Memory_Space[Offset + 6] == 0x20) {
									Type[i] = OKIType;
									Style[i] = OKI6376;
									ApplicationHandle->SetOutputMsg(L"ROM Style: ", L"m6376");
									TableOffset[OKITables] = Offset;
									OKITables++;

								}
							}
						}
					}
				}
			}
			//Check For OKI 6295
			CheckVal = 1;
			for (int i = 0; i < 8; i++) {
				if (Memory_Space[Offset + i] != 0) CheckVal = 0;
			}
			if (Memory_Space[Offset + 8] != 0x3) CheckVal = 0;
			if (Memory_Space[Offset + 9] != 0x0) CheckVal = 0;
			if (Memory_Space[Offset + 10] != 0x0) CheckVal = 0;

			if (CheckVal) {
				Type[i] = OKIType;
				Style[i] = OKI6295;
				ApplicationHandle->SetOutputMsg(L"ROM Style: ", L"m6295");
				TableOffset[OKITables] = Offset;
				OKITables++;
			}

			//Check For YMZ - Scorp 2
			CheckVal = 1;
			for (cnt = 0; cnt < 20; cnt++) {
				if (Memory_Space[Offset + ((cnt * 16) + 1)] != 0x20) {
					CheckVal = 0;
					break;
				}
			}
			if (CheckVal) {
				Type[i] = YMZType;
				Style[i] = Scorp2Style;
				ApplicationHandle->SetOutputMsg(L"ROM Style Detected: ", L"Scorpion 2");
			}
			//Check Epoch Style 1
			CheckVal = 0;
			Position = 2;

			CheckAddr = Memory_Space[Position];
			CheckAddr = (CheckAddr << 8);
			Position++;
			CheckAddr += Memory_Space[Position];
			CheckAddr = (CheckAddr << 8);
			Position++;
			CheckAddr += Memory_Space[Position];
			CheckAddr = (CheckAddr << 8);
			Position++;
			CheckAddr += Memory_Space[Position];
			Position = CheckAddr;

			for (cnt = 0; cnt < 32; cnt++) {
				if (((Position + 3 + cnt) < MAXROMSIZE) && (Position >= 0)) {
					if (Memory_Space[Position + cnt] == 0x43) {
						if (Memory_Space[Position + cnt + 2] == 0x30) {
							if (Memory_Space[Position + cnt + 3] == 0x30) {
								if (CheckAddr < TotalSize) {
									CheckVal = 1;
								}
							}
						}
					}
				}
			}

			if (CheckVal) {
				Type[i] = YMZType;
				Style[i] = Epoch1Style;
				ApplicationHandle->SetOutputMsg(L"ROM Style Detected: ", L"Epoch s1");

			} else {

				//Check Epoch Style 2
				CheckVal = 0;
				Position = 10;
				if (Memory_Space[Position] == 0x0) {
					if (Memory_Space[Position + 1] == 0x0) {
						if (Memory_Space[Position + 2] == 0x2b) {
							if (Memory_Space[Position + 3] == 0x11) {
								if (Memory_Space[Position + 4] == 0x2) {
									CheckVal = 1;
								}
							}
						}
					}
				}
				if (CheckVal) {
					Type[i] = YMZType;
					Style[i] = Epoch2Style;
					ApplicationHandle->SetOutputMsg(L"ROM Style Detected: ", L"Epoch s2");
				}
			}
			//Check BFM Scorp 4
			CheckVal = 0;
			Position = 4;
			if (Memory_Space[Position] == 0x50) {
				if (Memory_Space[Position + 1] == 0x52) {
					CheckVal = 1;
				}
			}

			if (CheckVal) {
				Type[i] = YMZType;
				Style[i] = Scorp4Style;
				ApplicationHandle->SetOutputMsg(L"ROM Style Detected: ", L"Scorpion 4");
			}
		}

		
		//Check for ID Type
		CheckVal = 0;

		for (int i = 0; i < ROMsToLoad; i++) {
			if (Type[i] != 255) {
				CheckVal |= Type[i];
			}
		}

		switch (CheckVal) {
		case 1:
			//OKI Sound
			IDType = OKIType;
			ApplicationHandle->SetOutputMsg(L"ID Type Detected: ", L"OKI");
			break;
		case 2:
			//NEC Sound
			IDType = NECType;
			ApplicationHandle->SetOutputMsg(L"ID Type Detected: ", L"NEC upd7759");
			break;
		case 4:
			//YMZ Sound
			IDType = YMZType;
			ApplicationHandle->SetOutputMsg(L"ID Type Detected: ", L"YAMAHA ymz280b");
			break;
		case 0:
			//No ROMS
			IDType = NoType;
			ApplicationHandle->SetOutputMsg(L"ID Type Detected: ", L"Error NO ROMS");
			break;
		default:
			//Mixed ROM Types, Unknown ROM types, or other error
			IDType = BADType;
			ApplicationHandle->SetOutputMsg(L"ID Type Detected: ", L"ERROR! Mixed or Unknown");
		}
		
		//Check for ID Style
		CheckVal = 0;

		for (int i = 0; i < ROMsToLoad; i++) {
			if (Style[i] != 255) {
				CheckVal |= Style[i];
			}
		}

		if (IDType == OKIType) {

			switch (CheckVal) {
			case OKI6376:
				//OKI 6376
				IDStyle = OKIType;
				ApplicationHandle->SetOutputMsg(L"ID Style Detected: ", L"m6376");
				break;
			case OKI6295:
				//OKI 6295
				IDStyle = NECType;
				ApplicationHandle->SetOutputMsg(L"ID Style Detected: ", L"m6295");
				break;
			case 0:
				//No ROMS
				IDStyle = NoStyle;
				ApplicationHandle->SetOutputMsg(L"ID Style: ", L"ERROR! NO ROMS");
				break;
			default:
				//Mixed ROM Types, Unknown ROM types, or other error
				IDStyle = BADStyle;
				ApplicationHandle->SetOutputMsg(L"ID Style: ", L"ERROR! Mixed or Unknown");
			}

		} else if (IDType == YMZType) {

			switch (CheckVal) {
			case Scorp2Style:
				//Scorpion 2
				IDStyle = Scorp2Style;
				ApplicationHandle->SetOutputMsg(L"ID Style Detected: ", L"Scorpion 2");
				break;
			case Epoch1Style:
				//Epoch 1
				IDStyle = Epoch1Style;
				ApplicationHandle->SetOutputMsg(L"ID Style Detected: ", L"Epoch 1");
				break;
			case Epoch2Style:
				//Epoch 2
				IDStyle = Epoch2Style;
				ApplicationHandle->SetOutputMsg(L"ID Style Detected: ", L"Epoch 2");
				break;
			case Scorp4Style:
				//Scorpion 4
				IDStyle = Scorp4Style;
				ApplicationHandle->SetOutputMsg(L"ID Style Detected: ", L"Scorpion 4");
				break;
			case Scorp5Style:
				//Scorpion 5
				IDStyle = Scorp4Style;
				ApplicationHandle->SetOutputMsg(L"ID Style Detected: ", L"Scorpion 5");
				break;
			case 0:
				//No ROMS
				IDStyle = NoStyle;
				ApplicationHandle->SetOutputMsg(L"ID Style: ", L"ERROR! NO ROMS");
				break;
			default:
				//Mixed ROM Styles, Unknown ROM Styles, or other error
				IDStyle = BADStyle;
				ApplicationHandle->SetOutputMsg(L"ID Style: ", L"ERROR! Mixed or Unknown");
			}

		}

	}

	ClearROMStrings(true); //Clear ROM strings now that we have loaded and identified the ROMs

	return true;
}

bool SndTool::ConvertWilliams() {
	UINT32 cnt, cnt2, cnt3, Position, ByteCount, SampleCount;
	UINT8 EndOfSample, CurrentSample;

	//Temporary Sample Memory
	INT16 * SampleLoadTemp = (INT16*)malloc(sizeof(INT16)*TEMPBUFFERSIZE);

	for (cnt = 0; cnt < 8; cnt++) {
		SamplesInPage[cnt] = 127;

		Position = 0; //Set to begining of current ROM Page

		for (cnt2 = 0; cnt2 < 128; cnt2++) {

			CurrentSample = (cnt2 & 0xff);
			SampleStartCur[cnt][CurrentSample] = Position;
			SampleStartBank[cnt][CurrentSample] = ((Bank_Space[cnt][Position] & 0x3) << 16);
			Position++;
			SampleStartBank[cnt][CurrentSample] += (Bank_Space[cnt][Position] << 8);
			Position++;
			SampleStartBank[cnt][CurrentSample] += Bank_Space[cnt][Position];
			Position++;

			SampleEndBank[cnt][CurrentSample] = ((Bank_Space[cnt][Position] & 0x3) << 16);
			Position++;
			SampleEndBank[cnt][CurrentSample] += (Bank_Space[cnt][Position] << 8);
			Position++;
			SampleEndBank[cnt][CurrentSample] += Bank_Space[cnt][Position];

			bool endofbank = false;
			for (int chk = 0; chk < 2; chk++) {
				Position++;
				if (Bank_Space[cnt][Position] != 0) {
					SamplesInPage[cnt] = CurrentSample;
					endofbank = true;
				}
			}
			if (endofbank) {
				break;
			}
			Position ++;
			//			Position += 3;

		}
	}

	UINT16 CurrentOverallSample = 0;

	for (cnt = 0; cnt < 8; cnt++) {
		if (cnt == 2 && skipbank2) {
			continue;
		}

		if (cnt == 3 && skipbank2) {
			continue;
		}

		if (cnt == 4 && skipbank67) {
			continue;
		}

		if (cnt == 5 && skipbank67) {
			continue;
		}

		for (cnt2 = 1; cnt2 < (SamplesInPage[cnt] + 1); cnt2++) {
			//Set Current Sample
			CurrentOverallSample++;
			CurrentSample = (cnt2 & 0xff);
			//Set Position
			Position = SampleStartBank[cnt][CurrentSample];
			//End Of Sample Flag
			EndOfSample = 0;
			ByteCount = 0;
			SampleCount = 0;

			ADPCMIndex = 0;
			ADPCMLast = 0;

			//Clear Sample Space
			for (cnt3 = 1; cnt3 < TEMPBUFFERSIZE; cnt3++) {
				SampleLoadTemp[cnt3] = 0;
			}

			//Decode to ADPCM
			while (Position <= SampleEndBank[cnt][CurrentSample]) {
				if (Position >= 0x20000) {
					//Common bank
					SampleLoadTemp[SampleCount] = OKIDecodeNibble((CommonBank[Position - 0x20000] & 0xf0) >> 4);
					SampleCount++;
					SampleLoadTemp[SampleCount] = OKIDecodeNibble(CommonBank[Position - 0x20000] & 0xf);
					SampleCount++;
				}
				else {
					SampleLoadTemp[SampleCount] = OKIDecodeNibble((Bank_Space[cnt][Position] & 0xf0) >> 4);
					SampleCount++;
					SampleLoadTemp[SampleCount] = OKIDecodeNibble(Bank_Space[cnt][Position] & 0xf);
					SampleCount++;
				}
				Position++;
			}

			//Copy Temp Data To Storage
			if (Sample_Space[CurrentOverallSample]) {
				free(Sample_Space[CurrentOverallSample]);
			}
			Sample_Space[CurrentOverallSample] = (INT16 *)malloc(sizeof(INT16) * SampleCount);
			memcpy(Sample_Space[CurrentOverallSample], SampleLoadTemp, sizeof(INT16) * SampleCount);

			//Set Table Data
			TotalSamples++;
			int bank = cnt;
			SampleBank[CurrentOverallSample] = (cnt & 0xff);
			if (SampleEndBank[cnt][CurrentSample] >= 0x20000) {
				SampleBank[CurrentOverallSample] = (99);
			}
			SampleIndex[CurrentOverallSample] = (cnt2);
			SampleLengthBytes[CurrentOverallSample] = (SampleEndBank[cnt][CurrentSample] - SampleStartBank[cnt][CurrentOverallSample]);
			SampleLengthSamples[CurrentOverallSample] = SampleCount;
			SampleRate[CurrentOverallSample] = 7576;
			if (SampleRate[CurrentOverallSample]) {
				SampleSeconds[CurrentOverallSample] = ((1.f / (float)SampleRate[CurrentOverallSample]) * (float)SampleCount);
			}
			else {
				SampleSeconds[CurrentOverallSample] = 0;
			}

			//Show the Data

				ApplicationHandle->SetSampleList(CurrentOverallSample, SampleStartCur[cnt][CurrentSample], SampleStartBank[cnt][CurrentSample], SampleEndBank[cnt][CurrentSample], SampleLengthSamples[CurrentOverallSample], SampleLengthBytes[CurrentOverallSample],
					SampleRate[CurrentOverallSample], bank, SampleBank[CurrentOverallSample], SampleIndex[CurrentOverallSample], SampleSeconds[CurrentOverallSample]);
		}
	}

	ApplicationHandle->SetOutputMsg(L"Note: Bank 99 is common bank, all others are: ");
	ApplicationHandle->SetOutputMsg(L"Bank 0 and 1: U12 0x40000 - 0x5ffff");
	ApplicationHandle->SetOutputMsg(L"Bank 2      : U12 0x20000 - 0x3ffff");
	ApplicationHandle->SetOutputMsg(L"Bank 3      : U12 0x00000 - 0x1ffff");
	ApplicationHandle->SetOutputMsg(L"Bank 4      : U13 0x60000 - 0x7ffff");
	ApplicationHandle->SetOutputMsg(L"Bank 5      : U13 0x40000 - 0x5ffff");
	ApplicationHandle->SetOutputMsg(L"Bank 6      : U13 0x20000 - 0x3ffff");
	ApplicationHandle->SetOutputMsg(L"Bank 7      : U13 0x00000 - 0x1ffff");

	//	ApplicationHandle->SetOutputMsg(L"Num   Start    End     Samples  Bytes     Rate   Bank   Index   Seconds");

	//Deallocate memory
	free(SampleLoadTemp);


	return true;
}

bool SndTool::Convert() {
	return Convert(0);
}

bool SndTool::Convert(int rate) {

	UINT32 cnt, cnt2, cnt3, Position, ByteCount, SampleCount;
	UINT8 EndOfSample, CurrentSample, BytesLeft;
	
	//Temporary Sample Memory
	INT16 * SampleLoadTemp = (INT16*)malloc(sizeof(INT16)*TEMPBUFFERSIZE);

	//Convert ROM to ADPCM
	switch (IDType) {
	case 1://OKI
		ApplicationHandle->SetOutputMsg(L"Tables Found: ", OKITables);

		switch (IDStyle) {
		case OKI6376:
			for (cnt = 0; cnt < OKITables; cnt++) {
				SamplesInPage[cnt] = 111;

				Position = TableOffset[cnt];

				for (cnt2 = 0; cnt2 < (SamplesInPage[cnt] + 1); cnt2++) {
					CurrentSample = (((cnt * SamplesInPage[cnt]) + cnt2) & 0xff);
					SampleStart[CurrentSample] = ((Memory_Space[Position] & 0x3f) * 65536);

					switch (Memory_Space[Position] & 0xC0) {
					case 0://8
						SampleRateDivisor[CurrentSample] = 8;
						break;
					case 64://10
						SampleRateDivisor[CurrentSample] = 10;
						break;
					case 128://16
						SampleRateDivisor[CurrentSample] = 16;
						break;
					case 192://Invalid and Unknown
						SampleRateDivisor[CurrentSample] = 8;
						break;
					}
					Position++;
					SampleStart[CurrentSample] += (Memory_Space[Position] << 8);
					Position++;
					SampleStart[CurrentSample] += (Memory_Space[Position] + (cnt * (TotalSize >> 1)));
					Position += 2;
				}
			}

			TotalSamples = 0;

			for (cnt = 0; cnt < OKITables; cnt++) {
				for (cnt2 = 1; cnt2 < (SamplesInPage[cnt] + 1); cnt2++) {
					//Set Current Sample
					CurrentSample = (((cnt * SamplesInPage[cnt]) + cnt2) & 0xff);
					//Set Position
					Position = SampleStart[CurrentSample];
					//End Of Sample Flag
					EndOfSample = 0;
					ByteCount = 0;
					SampleCount = 0;

					ADPCMIndex = 0;
					ADPCMLast = 0;
					//Clear Sample Space
					for (cnt3 = 1; cnt3 < TEMPBUFFERSIZE; cnt3++) {
						SampleLoadTemp[cnt3] = 0;
					}

					//Decode to ADPCM
					while (EndOfSample == 0) {
						if (Memory_Space[Position] == 255) {
							BytesLeft = 127;
						}
						else {
							BytesLeft = ((Memory_Space[Position] & 127) + 1);
							EndOfSample = 1;
						}
						for (cnt3 = 1; cnt3 <= BytesLeft; cnt3++) {
							ByteCount++;
							Position++;
							SampleLoadTemp[SampleCount] = OKIDecodeNibble((Memory_Space[Position] & 0xf0) >> 4);
							SampleCount++;
							SampleLoadTemp[SampleCount] = OKIDecodeNibble(Memory_Space[Position] & 0xf);
							SampleCount++;
						}
						Position++;
					}

					//Copy Temp Data To Storage
					if (Sample_Space[CurrentSample]) {
						free(Sample_Space[CurrentSample]);
					}
					Sample_Space[CurrentSample] = (INT16 *)malloc(sizeof(INT16) * SampleCount);
					memcpy(Sample_Space[CurrentSample], SampleLoadTemp, sizeof(INT16) * SampleCount);

					//Set Table Data
					TotalSamples++;
					SampleEnd[CurrentSample] = Position;
					SampleBank[CurrentSample] = (cnt & 0xff);
					SampleIndex[CurrentSample] = (cnt2 & 0xff);
					SampleLengthBytes[CurrentSample] = ByteCount;
					SampleLengthSamples[CurrentSample] = SampleCount;
					SampleRate[TotalSamples] = (120000 / SampleRateDivisor[CurrentSample]);
					if (SampleRate[CurrentSample]) {
						SampleSeconds[CurrentSample] = ((1.f / (float)SampleRate[CurrentSample]) * (float)SampleCount);
					}
					else {
						SampleSeconds[CurrentSample] = 0;
					}

					//Show the Data
					ApplicationHandle->SetSampleList(CurrentSample, SampleStart[CurrentSample], SampleEnd[CurrentSample], SampleLengthSamples[CurrentSample], SampleLengthBytes[CurrentSample],
													 SampleRate[CurrentSample], SampleBank[CurrentSample], SampleIndex[CurrentSample], SampleSeconds[CurrentSample]);
				}
			}
			break;

		case OKI6295:
		case WILLIAMSOKI:

			for (cnt = 0; cnt < OKITables; cnt++) {
				SamplesInPage[cnt] = 127;

				Position = TableOffset[cnt]; //Set to begining of current ROM Page

				for (cnt2 = 0; cnt2 < (SamplesInPage[cnt] + 1); cnt2++) {
					
					CurrentSample = (((cnt * SamplesInPage[cnt]) + cnt2) & 0xff);

					SampleStart[CurrentSample] = ((Memory_Space[Position] & 0x3) << 16);
					Position++;
					SampleStart[CurrentSample] += (Memory_Space[Position] << 8);
					Position++;
					SampleStart[CurrentSample] += Memory_Space[Position];
					Position++;

					SampleEnd[CurrentSample] = ((Memory_Space[Position] & 0x3) << 16);
					Position++;
					SampleEnd[CurrentSample] += (Memory_Space[Position] << 8);
					Position++;
					SampleEnd[CurrentSample] += Memory_Space[Position];
					Position += 3;

				}
			}
			
			for (cnt = 0; cnt < OKITables; cnt++) {
				for (cnt2 = 1; cnt2 < (SamplesInPage[cnt] + 1); cnt2++) {
					//Set Current Sample
					CurrentSample = (((cnt * SamplesInPage[cnt]) + cnt2) & 0xff);
					//Set Position
					Position = SampleStart[CurrentSample];
					//End Of Sample Flag
					EndOfSample = 0;
					ByteCount = 0;
					SampleCount = 0;

					ADPCMIndex = 0;
					ADPCMLast = 0;
					
					//Clear Sample Space
					for (cnt3 = 1; cnt3 < TEMPBUFFERSIZE; cnt3++) {
						SampleLoadTemp[cnt3] = 0;
					}

					//Decode to ADPCM
					while (Position <= SampleEnd[CurrentSample]) {													
						SampleLoadTemp[SampleCount] = OKIDecodeNibble((Memory_Space[Position] & 0xf0) >> 4);
						SampleCount++;
						SampleLoadTemp[SampleCount] = OKIDecodeNibble(Memory_Space[Position] & 0xf);
						SampleCount++;						
						Position++;
					}

					//Copy Temp Data To Storage
					if (Sample_Space[CurrentSample]) {
						free(Sample_Space[CurrentSample]);
					}
					Sample_Space[CurrentSample] = (INT16 *)malloc(sizeof(INT16) * SampleCount);
					memcpy(Sample_Space[CurrentSample], SampleLoadTemp, sizeof(INT16) * SampleCount);

					//Set Table Data
					TotalSamples++;					
					SampleBank[CurrentSample] = (cnt & 0xff);
					SampleIndex[CurrentSample] = (cnt2 & 0xff);
					SampleLengthBytes[CurrentSample] = (SampleEnd[CurrentSample] - SampleStart[CurrentSample]);
					SampleLengthSamples[CurrentSample] = SampleCount;
					if (rate > 0) {
						SampleRate[CurrentSample] = rate;
						SampleRate[TotalSamples] = rate;
					}
					else {
						SampleRate[CurrentSample] = 8000;
						SampleRate[TotalSamples] = 8000;
					}
					if (SampleRate[CurrentSample]) {
						SampleSeconds[CurrentSample] = ((1.f / (float)SampleRate[CurrentSample]) * (float)SampleCount);
					}
					else {
						SampleSeconds[CurrentSample] = 0;
					}

					//Show the Data
					ApplicationHandle->SetSampleList(CurrentSample, SampleStart[CurrentSample], SampleEnd[CurrentSample], SampleLengthSamples[CurrentSample], SampleLengthBytes[CurrentSample],
													 SampleRate[CurrentSample], SampleBank[CurrentSample], SampleIndex[CurrentSample], SampleSeconds[CurrentSample]);

				}
			}


			break;
		}
		if (rate != SampleRate[CurrentSample]) {
			ApplicationHandle->SetSampleCtrl(SampleRate[CurrentSample]);
		}
		ApplicationHandle->SetOutputMsg(L"Samples Found: ", TotalSamples);

		break;
	case 2://NEC

		UINT8 PageTemp;
		UINT8 Pages;
		INT32 Page;
		UINT32 Header;
		UINT8 Repeat;
		UINT8 Value;
		UINT8 ValidHeader;
		INT32 RepeatOffset;
		UINT32 SilenceLength;
		INT32 MyRate;
		INT32 SampleCount;

		UINT16 Nibbles;
		INT32 NibbleCount;

		for (cnt = 0; cnt < 8; cnt++) {
			for (cnt2 = 0; cnt2 < 128; cnt2++) {
				TuneLookup[cnt][cnt2] = 0;
			}
		}

		

		PageTemp = ((TotalSize / 131072) & 0xff);

		CurrentSample = 0;

		for (Pages = 0; Pages < PageTemp; Pages++) {
			//Set Page Size
			Page = Pages * 131072;
			//Set Initial Position
			Position = Page;
			//Get Number of Samples
			SamplesInPage[Pages] = Memory_Space[Position];
			Position++;
			//Get Header
			Header = Memory_Space[Position];
			Header = (Header << 8);
			Position++;
			Header |= Memory_Space[Position];
			Header = (Header << 8);
			Position++;
			Header |= Memory_Space[Position];
			Header = (Header << 8);
			Position++;
			Header |= Memory_Space[Position];
			Position++;
			//Check we are within a ROM
			if (Page < TotalSize) {
				//Check for Valid Header
				if (Header == 0x5AA56955) {
					//Check at least 1 sample
					if (SamplesInPage[Pages] > 0) {
						//Decode Samples in Page
						for (cnt = 0; cnt <= SamplesInPage[Pages]; cnt++) {

							CurrentSample++;
							//Clear Sample Space
							for (cnt2 = 0; cnt2 < TEMPBUFFERSIZE; cnt2++) {
								SampleLoadTemp[cnt2] = 0;
							}
							//Reset Repeat Flag
							Repeat = 0;
							//Set Position
							Position = (Page + 5 + (cnt * 2));
							//Get Start Address
							//Byte 1
							SampleStart[CurrentSample] = Memory_Space[Position];
							SampleStart[CurrentSample] = (SampleStart[CurrentSample] << 8);
							Position++;
							//Byte 2
							SampleStart[CurrentSample] |= Memory_Space[Position];
							Position++;
							//Double the value
							SampleStart[CurrentSample] = (SampleStart[CurrentSample] << 1);
							//Add Page Offset
							SampleStart[CurrentSample] += Page;

							//Set End Address
							if (cnt < SamplesInPage[Pages]) {
								//Set Position
								Position = (Page + 5 + ((cnt + 1) * 2));
								//Get End Address
								//Byte 1
								SampleEnd[CurrentSample] = Memory_Space[Position];
								SampleEnd[CurrentSample] = (SampleEnd[CurrentSample] << 8);
								Position++;
								//Byte 2
								SampleEnd[CurrentSample] |= Memory_Space[Position];
								Position++;
								//Double the value
								SampleEnd[CurrentSample] = (SampleEnd[CurrentSample] << 1);
								//Add Page Offset
								SampleEnd[CurrentSample] += Page;
							}
							else {
								//Set End Address
								SampleEnd[CurrentSample] = (Page + 131071);
							}
							//Set Position - Skip Start Byte
							Position = (SampleStart[CurrentSample] + 1);
							//Reset ADPCM
							ADPCMIndex = 0;
							ADPCMLast = 0;
							//Reset Counts
							ByteCount = 0;
							SampleCount = 0;
							//Reset Flag
							ValidHeader = 0;
							//Reset Nibbles
							Nibbles = 0;

							while (Position < SampleEnd[CurrentSample]) {
								//Repeat
								if (Repeat) {
									Repeat -= 1;
									Position = RepeatOffset;
								}
								//Get Data Byte
								Value = Memory_Space[Position];
								Position++;
								//Data Switch
								switch (Value & 192) {
								case 0://Silence
									if (((Value & 63) == 0) && (ValidHeader)) {
										Position = SampleEnd[CurrentSample];
									}
									else {
										ValidHeader = 1;
										SilenceLength = ((Value & 63) * 20);
										ADPCMIndex = 0;
										ADPCMLast = 0;
										for (cnt2 = 0; cnt2 < SilenceLength; cnt2++) {
											SampleLoadTemp[SampleCount] = NECDecodeNibble(0);
											SampleCount ++;
										}

									}
									Nibbles = 0;
									break;
								case 64://256 Nibbles
									MyRate = (160000 / ((Value & 31) + 1));
									Nibbles = 256;
									ValidHeader = 1;
									break;
								case 128://n Nibbles
									MyRate = (160000 / ((Value & 31) + 1));
									Nibbles = (Memory_Space[Position] + 1);
									Position++;
									ValidHeader = 1;
									break;
								case 192://Repeat Loop
									Repeat = ((Value & 7) + 1);
									RepeatOffset = Position;
									ValidHeader = 1;
									break;
								}

								if (Nibbles) {
									for (NibbleCount = 0; NibbleCount < Nibbles; NibbleCount++) {
										if (NibbleCount & 1) {
											//Low Nibble
											SampleLoadTemp[SampleCount] = NECDecodeNibble(Value & 0xf);
										}
										else {
											//Next Byte
											Value = Memory_Space[Position];
											Position++;
											//High Nibble
											SampleLoadTemp[SampleCount] = NECDecodeNibble((Value & 0xf0) >> 4);
											ByteCount ++;
										}
										SampleCount ++;
									}
								}
							}

							if (SampleCount > 0) {
								//Valid Sample Set Data Table
								SampleRate[CurrentSample] = MyRate;
								SampleLengthBytes[CurrentSample] = ByteCount;
								SampleLengthSamples[CurrentSample] = SampleCount;

								//Copy Temp Data To Storage
								if (Sample_Space[CurrentSample]) {
									free(Sample_Space[CurrentSample]);
								}
								Sample_Space[CurrentSample] = (INT16 *)malloc(sizeof(INT16) * SampleCount + 1);
								memcpy(Sample_Space[CurrentSample], SampleLoadTemp, sizeof(INT16) * SampleLengthSamples[CurrentSample]);


							}
							else {
								//Not a Valid Sample, set some defaults
								SampleLengthBytes[CurrentSample] = 0;
								SampleLengthSamples[CurrentSample] = 0;
								SampleRate[CurrentSample] = 8000;
							}

							//Update the sample length to its real value
							SampleEnd[CurrentSample] = (SampleStart[CurrentSample] + ByteCount);
							//Bank
							SampleBank[CurrentSample] = Pages;
							//Index
							SampleIndex[CurrentSample] = (cnt & 0xff);
							//Tune Lookup
							TuneLookup[SampleBank[CurrentSample]][SampleIndex[CurrentSample]] = (CurrentSample & 0xff);
							//Seconds
							if (SampleRate[CurrentSample]) {
								SampleSeconds[CurrentSample] = ((1.f / (float)SampleRate[CurrentSample]) * (float)SampleLengthSamples[CurrentSample]);
							}
							else {
								//Prevent Divide By Zero
								SampleSeconds[CurrentSample] = 0;
							}
							//Show The Data							
							ApplicationHandle->SetSampleList(CurrentSample, SampleStart[CurrentSample], SampleEnd[CurrentSample], SampleLengthSamples[CurrentSample], SampleLengthBytes[CurrentSample],
															 SampleRate[CurrentSample], SampleBank[CurrentSample], SampleIndex[CurrentSample], SampleSeconds[CurrentSample]);
						}
						//fprintf(DebugFile, "Samples In Page %d \n", SamplesInPage[Pages]);   
					}
					else {
						//fprintf(DebugFile, "No Samples In Page %d \n", Pages);
					}
				}
				else {
					//fprintf(DebugFile, "No Valid Header In Page %d \n", Pages);
				}
			}
			else {
				//fprintf(DebugFile, "Not Within ROM Page %d \n", Pages);
			}
		}
		TotalSamples = CurrentSample;
		ApplicationHandle->SetOutputMsg(L"Samples Found: ", TotalSamples);
		break;

	case 4://YMZ

		INT32 NumSamp, NumberOfSamples, Offset, SStart, SEnd, SRate, SLength, SamplePointer, Done, NameLoc;
		UINT8 DecodeStyle = 0;
		
		switch (IDStyle) {
		case Scorp2Style:
			//Reset Position
			Position = 0;
			//Reset Sample COunter
			CurrentSample = 0;

			//Get data from ROM Table
			Done = 0;
			while (!Done) {
				//Sample Rate Byte
				switch (DecodeStyle) {
				case Decode4ADPCM: //4 Bit ADPCM
					SRate = (INT32)((double)(Memory_Space[Position] * 172.2656) + 172.2656) + 1;
					break;
				case Decode16PCM: //16 Bit Linear PCM
					SRate = (INT32)((((double)Memory_Space[Position] * 172.2656) + 172.2656) / 2) + 1;
					break;
				case Decode8PCM: //8 Bit Linear PCM
					SRate = (INT32)(((double)Memory_Space[Position] * 172.2656) + 172.2656) + 1;//Might be wrong, haven't found a ROM that uses it yet
					break;
				}

				Position++;
				//Unsure but if valid this is 32
				if (DecodeStyle == 0) {
					if (Memory_Space[Position] != 32) {
						Done = 1;
					} else {
						CurrentSample++;
					}
				} else if (DecodeStyle == 1) {
					if (Memory_Space[Position] != 64) {
						Done = 1;
					} else {
						CurrentSample++;
					}
				}

				Position++;
				//Start Address MSB
				SStart = Memory_Space[Position];
				Position++;
				SStart <<= 8;
				//End Address MSB
				SEnd = Memory_Space[Position];
				Position++;
				SEnd <<= 8;
				//Start Address Middle
				SStart += Memory_Space[Position];
				Position++;
				SStart <<= 8;
				//End Address Middle
				SEnd += Memory_Space[Position];
				Position++;
				SEnd = (SEnd * 256);
				//Start Address LSB
				SStart += Memory_Space[Position];
				Position++;
				//End Address LSB
				SEnd += Memory_Space[Position];
				Position += 9;

				if ((SStart == 0) && (SEnd == 0)) {
					Done = 1;
				}

				//Set Data in storage
				SampleBank[CurrentSample] = 0;
				SampleIndex[CurrentSample] = CurrentSample;
				SampleRate[CurrentSample] = SRate;
				SampleStart[CurrentSample] = SStart;
				SampleEnd[CurrentSample] = SEnd;

				switch (DecodeStyle) {
					case Decode4ADPCM: //4bit ADPCM
						SampleLengthSamples[CurrentSample] = ((SEnd - SStart) << 1);
						SampleLengthBytes[CurrentSample] = (SEnd - SStart);												
						break;
					case Decode16PCM : //16Bit Linear PCM
						SampleLengthSamples[CurrentSample] = ((SEnd - SStart) >> 1);
						SampleLengthBytes[CurrentSample] = (SEnd - SStart);					
						break;
					case Decode8PCM : //8 bit linear PCM
						SampleLengthSamples[CurrentSample] = (SEnd - SStart);
						SampleLengthBytes[CurrentSample] = (SEnd - SStart);
						break;
				}
				SampleSeconds[CurrentSample] = ((1.f / (float)SampleRate[CurrentSample]) * (float)SampleLengthSamples[CurrentSample]);
			}

			NumberOfSamples = CurrentSample;
			break;
		case Scorp4Style:

			//Get data from ROM Table

			//Reset Position
			Position = 0;
			//Reset Sample Counter
			CurrentSample = 0;
			//Start Offset - Byte 1
			Offset = (Memory_Space[Position]);
			Offset <<= 8;
			Position++;
			//Byte 2
			Offset += Memory_Space[Position];
			Position++;
			Offset <<= 8;
			//Byte 3
			Offset += Memory_Space[Position];
			Position++;
			Offset <<= 8;
			//Byte 4
			Offset += Memory_Space[Position];
			Position++;
			//Get 32 Byte Name String
			Position = Offset;
			
			for (cnt = 0; cnt < 32; cnt++) {
				ROMName[cnt] = Memory_Space[Position];
				if (ROMName[cnt] == NULL) ROMName[cnt] = ' ';
				Position++;
			}
			ROMName[32] = NULL;
			ApplicationHandle->SetOutputMsg(L"Found ROM Name: ", ROMName, 33);

			//Sample Start / Rate / Size
			Position = (Offset + 53);

			Done = 0;
			while (!Done){
				
				//Set current sample number
				CurrentSample = (CurrentSample + 1);
				//3 Byte Start Address
				//Byte 1
				SStart = Memory_Space[Position];
				Position++;
				SStart <<= 8;
				//Byte 2
				SStart += Memory_Space[Position];
				Position++;
				SStart <<= 8;
				//Byte 3
				SStart += Memory_Space[Position];
				//Skip 10 Bytes
				Position += 10;

				//Loop Start and Loop End Data here				

				//3 Byte Sample End
				//Byte 1
				SEnd = Memory_Space[Position];
				Position++;
				SEnd = (SEnd * 256);
				//Byte 2
				SEnd += Memory_Space[Position];
				Position++;
				SEnd = (SEnd * 256);
				//Byte 3
				SEnd += Memory_Space[Position];
				//Skip 1 Byte
				Position += 2;

				//Sample Rate
				//Byte 1
				SRate = (Memory_Space[Position] * 1000);
				//Skip 1 Byte
				Position += 2;

				//Check for End Of Samples
				if (SStart >= 0xFFFFFF){
					if (SEnd >= 0xFFFFFF){
						NumberOfSamples = (CurrentSample - 1);
						Done = 1;
					}
				}

				//Set Data in storage
				SampleBank[CurrentSample] = 0;
				SampleIndex[CurrentSample] = CurrentSample;
				SampleRate[CurrentSample] = SRate;
				SampleStart[CurrentSample] = SStart;
				SampleEnd[CurrentSample] = SEnd;
				SampleLengthSamples[CurrentSample] = ((SEnd - SStart) << 1);
				SampleLengthBytes[CurrentSample] = (SEnd - SStart);
				SampleSeconds[CurrentSample] = ((1.f / (float)SampleRate[CurrentSample]) * (float)SampleLengthSamples[CurrentSample]);
			}
			break;
		case Scorp5Style:

			break;
		case Epoch1Style:

			//Get data from ROM Table
			//Reset Position
			Position = 0;
			//Reset Sample COunter
			CurrentSample = 0;
			//Number of samples - Byte 1
			NumSamp = (Memory_Space[Position]);
			NumSamp = (NumSamp * 256);
			Position++;
			//Byte 2
			NumSamp = (NumSamp + Memory_Space[Position]);
			Position++;
			NumberOfSamples = NumSamp - 1;
			//Name Location
			//Byte 1
			NameLoc = Memory_Space[Position];
			NameLoc <<= 8;
			Position++;
			//Byte 2
			NameLoc += Memory_Space[Position];
			NameLoc <<= 8;
			Position++;
			//Byte 3
			NameLoc += Memory_Space[Position];
			NameLoc <<= 8;
			Position++;
			//Byte 4
			NameLoc += Memory_Space[Position];
			Position++;
			//ROM Name			
			Position = NameLoc;
			//Get 72 Byte String
			for (cnt = 0; cnt < 72; cnt++) {
				ROMName[cnt] = Memory_Space[Position];
				if (ROMName[cnt] == NULL) ROMName[cnt] = ' ';
				Position++;
			}
			ROMName[72] = NULL;
			ApplicationHandle->SetOutputMsg(L"Found ROM Name: ", ROMName, 73);			

			//Sample Start / Rate / Size
			Position = 15;

			for (CurrentSample = 1, CurrentSample <= NumberOfSamples; CurrentSample++;) {
				//Start Address
				//Byte 1
				SStart = Memory_Space[Position];
				Position++;
				SStart <<= 8;
				//Byte 2
				SStart += Memory_Space[Position];
				Position++;
				SStart <<= 8;
				//Byte 3
				SStart += Memory_Space[Position];
				Position++;
				SStart <<= 8;
				//Byte 4
				SStart += Memory_Space[Position];
				Position++;

				//Sample Length (Bytes)
				//Byte 1
				SLength = Memory_Space[Position];
				Position++;
				SLength <<= 8;
				//Byte 2
				SLength += Memory_Space[Position];
				Position++;
				SLength <<= 8;
				//Byte 3
				SLength += Memory_Space[Position];
				Position++;
				SLength <<= 8;
				//Byte 4
				SLength += Memory_Space[Position];
				Position++;

				//Sample Rate
				//Byte 1
				SRate = Memory_Space[Position];
				Position++;
				SRate <<= 8;
				//Byte 2
				SRate += Memory_Space[Position];
				Position++;
				SRate <<= 8;
				//Byte 3
				SRate += Memory_Space[Position];
				Position++;
				SRate <<= 8;
				//Byte 4
				SRate += Memory_Space[Position];
				Position += 2;

				//Set Data in storage
				SampleBank[CurrentSample] = 0;
				SampleIndex[CurrentSample] = CurrentSample;
				SampleRate[CurrentSample] = SRate;
				SampleStart[CurrentSample] = SStart;
				SampleEnd[CurrentSample] = (SStart + SLength);
				SampleLengthSamples[CurrentSample] = (SLength << 1);
				SampleLengthBytes[CurrentSample] = SLength;
				SampleSeconds[CurrentSample] = ((1.f / (float)SampleRate[CurrentSample]) * (float)SampleLengthSamples[CurrentSample]);

			}
			break;
		case Epoch2Style:

			//Get data from ROM Table
			//Reset Position
			Position = 0;
			//Reset Sample COunter
			CurrentSample = 0;
			//Number of samples - Byte 1
			NumSamp = (Memory_Space[Position]);
			NumSamp = (NumSamp * 256);
			Position++;
			//Byte 2
			NumSamp = (NumSamp + Memory_Space[Position]);
			Position++;
			NumberOfSamples = (NumSamp - 1);

			for( CurrentSample = 1; CurrentSample < NumberOfSamples; CurrentSample++){
				//Start Address
				//Byte 1
				SStart = Memory_Space[Position];
				Position++;
				SStart <<= 8;
				//Byte 2
				SStart += Memory_Space[Position];
				Position++;
				SStart <<= 8;
				//Byte 3
				SStart += Memory_Space[Position];
				Position++;
				SStart <<= 8;
				//Byte 4
				SStart += Memory_Space[Position];
				Position++;

				//Sample Length (Bytes)
				//Byte 1
				SLength = Memory_Space[Position];
				Position++;
				SLength <<= 8;
				//Byte 2
				SLength += Memory_Space[Position];
				Position++;
				SLength <<= 8;
				//Byte 3
				SLength += Memory_Space[Position];
				Position++;
				SLength <<= 8;
				//Byte 4
				SLength += Memory_Space[Position];
				Position++;

				//Sample Rate
				//Byte 1
				SRate = Memory_Space[Position];
				Position++;
				SRate = (SRate * 256);
				//Byte 2
				SRate = (SRate + Memory_Space[Position]);
				Position++;
				SRate = (SRate * 256);
				//Byte 3
				SRate = (SRate + Memory_Space[Position]);
				Position++;
				SRate = (SRate * 256);
				//Byte 4
				SRate = (SRate + Memory_Space[Position]);
				Position = (Position + 2);


				//Set Data in storage
				SampleBank[CurrentSample] = 0;
				SampleIndex[CurrentSample] = CurrentSample;
				SampleRate[CurrentSample] = SRate;
				SampleStart[CurrentSample] = SStart;
				SampleEnd[CurrentSample] = (SStart + SLength);
				SampleLengthSamples[CurrentSample] = (SLength << 1);
				SampleLengthBytes[CurrentSample] = SLength;
				SampleSeconds[CurrentSample] = ((1.f / (float)SampleRate[CurrentSample]) * (float)SampleLengthSamples[CurrentSample]);
				
			}
			break;			
		}	

		ApplicationHandle->SetOutputMsg(L"Samples Found: ", NumberOfSamples);

		//Step through each sample convert from ADPCM to PCM
		for (CurrentSample = 1; CurrentSample <= NumberOfSamples; CurrentSample++) {
			//Set Position
			Position = SampleStart[CurrentSample];
			//Set Initial Values
			Step = 127;
			Signal = 0;
			//Clear Sample Space
			for (cnt = 0; cnt < TEMPBUFFERSIZE; cnt++) {
				SampleLoadTemp[cnt] = 0;
			}
			//Reset Sample Counter
			SamplePointer = 0;
			switch (DecodeStyle) {
			case Decode4ADPCM: //4 bit ADPCM

				while (Position < SampleEnd[CurrentSample]) {
					//Nibble 1
					YMZDecodeNibble((Memory_Space[Position] & 0xF0) >> 4);
					//Store Sample
					SampleLoadTemp[SamplePointer] = Signal;
					//Increment Counter
					SamplePointer++;
					//Nibble 2
					YMZDecodeNibble(Memory_Space[Position] & 0xF);
					//Store Sample
					SampleLoadTemp[SamplePointer] = Signal;
					//Increment Counter
					SamplePointer++;
					//Increment Position
					Position++;
				}

				break;
			case Decode16PCM: //16 bit Linear PCM

				while (Position < SampleEnd[CurrentSample]) {
					//Data
					SampleLoadTemp[SamplePointer] = ((INT16)(Memory_Space[Position] << 8) + Memory_Space[Position + 1]);
					//Increment Counter
					SamplePointer = (SamplePointer + 1);
					//Increment Position
					Position = (Position + 2);
				}

				break;
			case Decode8PCM: //8 bit linear PCM

				while (Position < SampleEnd[CurrentSample]) {
					//Data
					SampleLoadTemp[SamplePointer] = Memory_Space[Position];
					//Increment Counter
					SamplePointer = (SamplePointer + 1);
					//Increment Posiiton
					Position++;
				}

				break;
			}

			//Copy Temp Data To Storage
			if (Sample_Space[CurrentSample]) {
				free(Sample_Space[CurrentSample]);
			}
			Sample_Space[CurrentSample] = (INT16 *)malloc(sizeof(INT16) * SampleLengthSamples[CurrentSample] + 1);
			memcpy(Sample_Space[CurrentSample], SampleLoadTemp, sizeof(INT16) * SampleLengthSamples[CurrentSample]);

			//Show The Data			
			ApplicationHandle->SetSampleList(CurrentSample, SampleStart[CurrentSample], SampleEnd[CurrentSample], SampleLengthSamples[CurrentSample], SampleLengthBytes[CurrentSample],
											 SampleRate[CurrentSample], SampleBank[CurrentSample], SampleIndex[CurrentSample], SampleSeconds[CurrentSample]);
		}
		break;
	}

	//Deallocate memory
	free(SampleLoadTemp);
	

	return true;
}


//To avoid issues with comDlg32.dll in Windows 10 we keep an internal buffer of paths and use the listbox for display only
//The Following 5 functions deal with obtaining the paths of files to be opened and storing, removing, or clearing them to act as if they are in the list box.
int SndTool::GetFileNames() {

	int res = 0;
	//Set up common dialog box
	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY)))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen))))
		{
			// Set the options on the dialog.
			DWORD dwFlags;

			// Before setting, always get the options first in order 
			// not to override existing options.
			if (SUCCEEDED(pFileOpen->GetOptions(&dwFlags)))
			{
				// In this case, get shell items only for file system items.				
				if (SUCCEEDED(pFileOpen->SetOptions(dwFlags | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST)))
				{
					// Show the Open dialog box.            
					if (SUCCEEDED(pFileOpen->Show(NULL)))
					{

						//Get Result from dlg box
						IShellItemArray  *pItems;						
						if (SUCCEEDED(pFileOpen->GetResults(&pItems)))
						{
							//Get the file name from the dialog box.
							PWSTR pszFilePath;
							DWORD Num;
							if (SUCCEEDED(pItems->GetCount(&Num)))
							{
								IShellItem *pItem;
								for (DWORD i = 0; i < Num; i++) {
									if (SUCCEEDED(pItems->GetItemAt(i, &pItem)))
									{
										if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
										//Store returned path
										AddROMString(pszFilePath);
										res++;										
										CoTaskMemFree(pszFilePath);
										pItem->Release();
									}
								}
							}
							pItems->Release();
						}
					}
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	//Returns number of files to open, 0 = none.
	return res;

}

int SndTool::GetU12FileNames() {

	int res = 0;
	//Set up common dialog box
	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY)))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen))))
		{
			// Set the options on the dialog.
			DWORD dwFlags;

			// Before setting, always get the options first in order 
			// not to override existing options.
			if (SUCCEEDED(pFileOpen->GetOptions(&dwFlags)))
			{
				// In this case, get shell items only for file system items.				
				if (SUCCEEDED(pFileOpen->SetOptions(dwFlags | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST)))
				{
					// Show the Open dialog box.            
					if (SUCCEEDED(pFileOpen->Show(NULL)))
					{

						//Get Result from dlg box
						IShellItemArray  *pItems;
						if (SUCCEEDED(pFileOpen->GetResults(&pItems)))
						{
							//Get the file name from the dialog box.
							PWSTR pszFilePath;
							DWORD Num;
							if (SUCCEEDED(pItems->GetCount(&Num)))
							{
								IShellItem *pItem;
								for (DWORD i = 0; i < Num; i++) {
									if (SUCCEEDED(pItems->GetItemAt(i, &pItem)))
									{
										if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
											//Store returned path
											AddWROMString(pszFilePath,true);
										res++;
										CoTaskMemFree(pszFilePath);
										pItem->Release();
									}
								}
							}
							pItems->Release();
						}
					}
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	//Returns number of files to open, 0 = none.
	return res;

}

int SndTool::GetU13FileNames() {

	int res = 0;
	//Set up common dialog box
	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY)))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen))))
		{
			// Set the options on the dialog.
			DWORD dwFlags;

			// Before setting, always get the options first in order 
			// not to override existing options.
			if (SUCCEEDED(pFileOpen->GetOptions(&dwFlags)))
			{
				// In this case, get shell items only for file system items.				
				if (SUCCEEDED(pFileOpen->SetOptions(dwFlags | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST)))
				{
					// Show the Open dialog box.            
					if (SUCCEEDED(pFileOpen->Show(NULL)))
					{

						//Get Result from dlg box
						IShellItemArray  *pItems;
						if (SUCCEEDED(pFileOpen->GetResults(&pItems)))
						{
							//Get the file name from the dialog box.
							PWSTR pszFilePath;
							DWORD Num;
							if (SUCCEEDED(pItems->GetCount(&Num)))
							{
								IShellItem *pItem;
								for (DWORD i = 0; i < Num; i++) {
									if (SUCCEEDED(pItems->GetItemAt(i, &pItem)))
									{
										if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
											//Store returned path
											AddWROMString(pszFilePath,false);
										res++;
										CoTaskMemFree(pszFilePath);
										pItem->Release();
									}
								}
							}
							pItems->Release();
						}
					}
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	//Returns number of files to open, 0 = none.
	return res;

}

wchar_t* SndTool::GetROMString(int Index) {

	//Return requested string at Index
	return pszFileROMPath[Index];

}

wchar_t* SndTool::GetWROMString(int Index, bool U12) {

	if (U12) {
		return pszFileU12ROMPath[Index];
	}
	else {
		return pszFileU13ROMPath[Index];
	}
}

bool SndTool::AddROMString(wchar_t * InString) {	
	
	//Get Length of incoming string
	size_t Len = wcslen(InString);
	//Allocate Memory
	pszFileROMPath[ROMsToLoad] = (wchar_t*)malloc(MAX_PATH);
	//Copy String to storage
	if (pszFileROMPath[ROMsToLoad]) {
		wcscpy_s(pszFileROMPath[ROMsToLoad], Len + 1, InString);
	}
	//Increment count
	ROMsToLoad++;

	return true;
}

bool SndTool::AddWROMString(wchar_t * InString, bool U12) {

	//Get Length of incoming string
	size_t Len = wcslen(InString);
	//Allocate Memory
	if (U12) {
		pszFileU12ROMPath[U12ROMsToLoad] = (wchar_t*)malloc(MAX_PATH);
		//Copy String to storage
		if (pszFileU12ROMPath[U12ROMsToLoad]) {
			wcscpy_s(pszFileU12ROMPath[U12ROMsToLoad], Len + 1, InString);
		}
		//Increment count
		U12ROMsToLoad++;
	}
	else {
		pszFileU13ROMPath[U13ROMsToLoad] = (wchar_t*)malloc(MAX_PATH);
		//Copy String to storage
		if (pszFileU13ROMPath[U13ROMsToLoad]) {
			wcscpy_s(pszFileU13ROMPath[U13ROMsToLoad], Len + 1, InString);
		}
		//Increment count
		U13ROMsToLoad++;
	}

	return true;
}

bool SndTool::DeleteROMString(int Index) {

	//Check Bounds
	if ((Index >= 0) && (Index <= (MAXSUPPORTEDROMS - 1))){
		//'Delete' the entry in the storage array
		if (pszFileROMPath[Index] != 0) {
			//free memory
			free(pszFileROMPath[Index]);
			//clear pointer
			pszFileROMPath[Index] = 0;
		}
		//decrement count
		ROMsToLoad -= 1;
		//Move everything up if necessary
		//No need to allocate or free we are just shuffling pointers
		if (Index < ROMsToLoad) {
			for (int i = Index; i < (MAXSUPPORTEDROMS - 1); i++) {
				pszFileROMPath[i] = pszFileROMPath[i + 1];
			}
			//Top entry will always be cleared
			pszFileROMPath[(MAXSUPPORTEDROMS - 1)] = 0;
		}
	}

	return true;

}

bool SndTool::DeleteWROMString(int Index, bool U12) {

	if (U12) {
		//Check Bounds
		if ((Index >= 0) && (Index <= (MAXSUPPORTEDROMS - 1))) {
			//'Delete' the entry in the storage array
			if (pszFileU12ROMPath[Index] != 0) {
				//free memory
				free(pszFileU12ROMPath[Index]);
				//clear pointer
				pszFileU12ROMPath[Index] = 0;
			}
			//decrement count
			U12ROMsToLoad -= 1;
			//Move everything up if necessary
			//No need to allocate or free we are just shuffling pointers
			if (Index < U12ROMsToLoad) {
				for (int i = Index; i < (MAXSUPPORTEDROMS - 1); i++) {
					pszFileU12ROMPath[i] = pszFileU12ROMPath[i + 1];
				}
				//Top entry will always be cleared
				pszFileU12ROMPath[(MAXSUPPORTEDROMS - 1)] = 0;
			}
		}
	}
	else {
		//Check Bounds
		if ((Index >= 0) && (Index <= (MAXSUPPORTEDROMS - 1))) {
			//'Delete' the entry in the storage array
			if (pszFileU13ROMPath[Index] != 0) {
				//free memory
				free(pszFileU13ROMPath[Index]);
				//clear pointer
				pszFileU13ROMPath[Index] = 0;
			}
			//decrement count
			U13ROMsToLoad -= 1;
			//Move everything up if necessary
			//No need to allocate or free we are just shuffling pointers
			if (Index < U13ROMsToLoad) {
				for (int i = Index; i < (MAXSUPPORTEDROMS - 1); i++) {
					pszFileU13ROMPath[i] = pszFileU13ROMPath[i + 1];
				}
				//Top entry will always be cleared
				pszFileU13ROMPath[(MAXSUPPORTEDROMS - 1)] = 0;
			}
		}
	}

	return true;

}

bool SndTool::ClearROMStrings(bool Clear) {

	//clear all stored paths and associated memory
	for (int i = 0; i < MAXSUPPORTEDROMS; i++) {
		if (pszFileROMPath[i] != 0) {
			free(pszFileROMPath[i]);
			pszFileROMPath[i] = 0;
		}
	}
	
	if (Clear) ROMsToLoad = 0;
	return true;

}

bool SndTool::ClearWROMStrings(bool Clear, bool U12) {

	if (U12) {
		//clear all stored paths and associated memory
		for (int i = 0; i < MAXSUPPORTEDROMS; i++) {
			if (pszFileU12ROMPath[i] != 0) {
				free(pszFileU12ROMPath[i]);
				pszFileU12ROMPath[i] = 0;
			}
		}

		if (Clear) U12ROMsToLoad = 0;
	}
	else {
		//clear all stored paths and associated memory
		for (int i = 0; i < MAXSUPPORTEDROMS; i++) {
			if (pszFileU13ROMPath[i] != 0) {
				free(pszFileU13ROMPath[i]);
				pszFileU13ROMPath[i] = 0;
			}
		}

		if (Clear) U13ROMsToLoad = 0;
	}
	return true;

}

INT16 SndTool::NECDecodeNibble(UINT8 Nibble) {

	INT32 Sample;
	INT16 NECAdjusts[16] = { -1, -1, 0, 0, 1, 2, 2, 3, -1, -1, 0, 0, 1, 2, 2, 3 };


	Sample = (ADPCMLast + StepNEC[ADPCMIndex * 16 + Nibble]);

	ADPCMIndex += NECAdjusts[Nibble];
	//Clamp Index
	if (ADPCMIndex < 0) {
		ADPCMIndex = 0;
	}
	else if (ADPCMIndex > 15) {
		ADPCMIndex = 15;
	}
	//Clamp Sample
	if (Sample > 255) {
		Sample = 255;
	}
	else if (Sample < -255) {
		Sample = -255;
	}
	ADPCMLast = Sample;
	Sample = (Sample << 7);
	//Clamp Sample
	if (Sample > 32767) {
		Sample = 32767;
	}
	else if (Sample < -32768) {
		Sample = -32768;
	}
	return (Sample & 0xffff);
}


INT16 SndTool::OKIDecodeNibble(UINT8 Nibble) {

	INT32 SS, Sample, Diff, E;

	INT16 StepOKI[49] = { 16, 17, 19, 21, 23, 25, 28, 31, 34, 37,
					 41, 45, 50, 55, 60, 66, 73, 80, 88, 97,
					 107, 118, 130, 143, 157, 173, 190, 209, 230, 253,
					 279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
					 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552 };

	INT16 OKIAdjusts[16] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };

	SS = StepOKI[ADPCMIndex];
	E = (SS / 8);
	if (Nibble & 1) {
		E += (SS >> 2);
	}
	if (Nibble & 2) {
		E += (SS >> 1);
	}
	if (Nibble & 4) {
		E += SS;
	}
	Diff = (Nibble & 8) ? -E : E;

	Sample = (ADPCMLast + Diff);

	if (Sample > 2048) {
		Sample = 2048;
	}
	if (Sample < -2048) {
		Sample = -2048;
	}

	ADPCMLast = Sample;
	ADPCMIndex += OKIAdjusts[Nibble];

	if (ADPCMIndex > 48) {
		ADPCMIndex = 48;
	}
	if (ADPCMIndex < 0) {
		ADPCMIndex = 0;
	}

	return (Sample & 0xffff);

}


void SndTool::YMZDecodeNibble(UINT8 Nibble) {

	INT16 Index_Scale[8] = { 0xE6, 0xE6, 0xE6, 0xE6, 0x133, 0x199, 0x200, 0x266 };	
	int sig, ste;

	sig = Signal;
	ste = Step;

	//Calculate Signal
	sig = (sig + ((ste * Diff_LookUp[Nibble & 0xf]) >> 3));

	//Clamp
	if (sig > 32767) {
		sig = 32767;
	}
	else if (sig < -32768) {
		sig = -32768;
	}

	//Adjust Step
	ste = ((ste * Index_Scale[Nibble & 0x7]) >> 0x8);

	//Clamp
	if (ste > 24576) {
		ste = 24576;
	}
	else if (ste < 127) {
		ste = 127;
	}

	//Save Signal & Step
	Signal = sig;
	Step = ste;

}

