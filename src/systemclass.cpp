////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "systemclass.h"
#include "resource.h"
#include <string> 
#include <Windows.h>



SystemClass::SystemClass()
{
	m_Application = 0;
}

SystemClass::~SystemClass()
{
}
void SystemClass::SetOutputMsg(wchar_t * InMsg, int Data) {

	wchar_t Temp[MAX_LOADSTRING];
	wchar_t Number[10];
	int Len1, Len2, TLen;

	_itow_s(Data, Number, sizeof(Number) / 2, 10);

	Len1 = lstrlenW(InMsg);
	Len2 = lstrlenW(Number);
	TLen = Len1 + Len2;

	for (int i = 0; i < Len1; i++) {
		Temp[i] = InMsg[i];
	}

	for (int i = 0; i < Len2; i++) {
		Temp[Len1 + i] = Number[i];
	}
	Temp[TLen] = NULL;

	SetOutput((LPARAM)Temp);

}

void SystemClass::SetOutputMsg(const wchar_t * InMsg) {

	wchar_t Temp[MAX_LOADSTRING];
	int Len1, Len2, TLen;


	Len1 = lstrlenW(InMsg);
	TLen = Len1;

	for (int i = 0; i < Len1; i++) {
		Temp[i] = InMsg[i];
	}

	Temp[TLen] = NULL;

	SetOutput((LPARAM)Temp);

}
void SystemClass::SetSampleCtrl(int rate) {
	SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_SAMP), EM_SETREADONLY, 0, 0);
	char txt[32] = { 0 };
	_snprintf_s(txt, 31, "%i", rate);
	SetDlgItemTextA(ApplicationHandle->m_hwnd, IDC_SAMP, txt);

}

void SystemClass::SetSampleList(int SampleNumber, int start, int end, int samples, int bytes, int rate, int bank, int index, float Seconds) {
	int temp = 0;
	SetSampleList(SampleNumber, 0, start, end, samples, bytes, rate, temp, bank, index, Seconds);
}
void SystemClass::SetSampleList(int SampleNumber, int cur, int start, int end, int samples, int bytes, int rate, int realbank, int bank, int index, float Seconds) {
	
	wchar_t Number[9], Output[90];

	//"Num   Start    End     Samples  Bytes     Rate   Bank   Index   Seconds"

	for (int i = 0; i < 76; i++) {		
		Output[i] = L' ';		
	}

	//num
	_itow_s(SampleNumber, Number, sizeof(Number) / 2, 10);
	
	for (int i = 0; i < 9; i++) {
		if (Number[i] == 0) {
			break;
		} else {
			Output[i] = Number[i];
		}		
	}
	_itow_s(cur, Number, sizeof(Number) / 2, 16);
	for (int i = 0; i < 9; i++) {
		if (Number[i] == 0) {
			break;
		}
		else {
			Output[i + 6] = Number[i];
		}
	}

	//start
	_itow_s(start, Number, sizeof(Number) / 2, 16);
	for (int i = 0; i < 9; i++) {
		if (Number[i] == 0) {
			break;
		} else {
			Output[i + 15] = Number[i];
		}
	}


	//end
	_itow_s(end, Number, sizeof(Number) / 2, 16);
	for (int i = 0; i < 9; i++) {
		if (Number[i] == 0) {
			break;
		} else {
			Output[i + 23] = Number[i];
		}
	}

	//samples
	_itow_s(samples, Number, sizeof(Number) / 2, 16);
	for (int i = 0; i < 9; i++) {
		if (Number[i] == 0) {
			break;
		} else {
			Output[i + 32] = Number[i];
		}
	}
	
	//bytes
	_itow_s(bytes, Number, sizeof(Number) / 2, 16);
	for (int i = 0; i < 9; i++) {
		if (Number[i] == 0) {
			break;
		} else {
			Output[i + 42] = Number[i];
		}
	}

	//rate
	_itow_s(rate, Number, sizeof(Number) / 2, 10);
	for (int i = 0; i < 9; i++) {
		if (Number[i] == 0) {
			break;
		} else {
			Output[i + 49] = Number[i];
		}
	}

	//code
	_itow_s(realbank, Number, sizeof(Number) / 2, 10);
	for (int i = 0; i < 9; i++) {
		if (Number[i] == 0) {
			break;
		}
		else {
			Output[i + 56] = Number[i];
		}
	}

	//bank
	_itow_s(bank, Number, sizeof(Number) / 2, 10);
	for (int i = 0; i < 4; i++) {
		if (Number[i] == 0) {
			break;
		} else {
			Output[i + 63] = Number[i];
		}
	}

	//index
	_itow_s(index, Number, sizeof(Number) / 2, 10);
	for (int i = 0; i < 9; i++) {
		if (Number[i] == 0) {
			break;
		} else {
			Output[i + 71] = Number[i];
		}
	}

	//Seconds
	swprintf(Number,8, L"%F",Seconds);
	for (int i = 0; i < 9; i++) {
		if (Number[i] == 0) {
			break;
		} else {
			Output[i + 75] = Number[i];
		}
	}
	//End String
	Output[82] = 0;

	SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_SAMPLE_LIST), LB_ADDSTRING, 0, (LPARAM)Output);
}



void SystemClass::SetOutput(LPARAM Para) {
	SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_LIST), LB_ADDSTRING, 0, Para);
}

void SystemClass::ClearROMOutput() {
	SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_LIST), LB_RESETCONTENT, NULL, NULL);
	SendMessage(GetDlgItem(ApplicationHandle->m_WROMhwnd, IDC_U12_LIST), LB_RESETCONTENT, NULL, NULL);
	SendMessage(GetDlgItem(ApplicationHandle->m_WROMhwnd, IDC_U13_LIST), LB_RESETCONTENT, NULL, NULL);

}

void SystemClass::ClearMainOutput() {
	//Clear Output Messages
	SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_LIST), LB_RESETCONTENT, NULL, NULL);
	//Clear Sample List
	SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_SAMPLE_LIST), LB_RESETCONTENT, NULL, NULL);
	//Set Top of Sample List
	SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_SAMPLE_LIST), LB_ADDSTRING, 0, (LPARAM)L"");
}

void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc = { sizeof(wc) };
	DEVMODE dmScreenSettings;
	int posX, posY;
	int FrameX, FrameY;

	// Get an external pointer to this object.	
	ApplicationHandle = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	LoadStringW(m_hinstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(m_hinstance, IDC_SOUNDTOOL, szWindowClass, MAX_LOADSTRING);

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(m_hinstance, MAKEINTRESOURCE(IDI_SOUNDTOOL));
	wc.hIconSm = LoadIcon(m_hinstance, MAKEINTRESOURCE(IDI_SMALL));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = MAKEINTRESOURCEW(IDC_SOUNDTOOL);
	wc.lpszClassName = szWindowClass;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		screenWidth = 800;
		screenHeight = 600;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	//Fonts
	m_CourierNewFnt = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Courier New");
	m_ArialFnt = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");

	// Create the Main window and associated controls
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, szWindowClass, L"Arcade Sound Tool", (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME), posX, posY, 749, 760, NULL, NULL, m_hinstance, NULL);
	FrameX = 12;
	FrameY = 52;
	m_Frame1 = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Output Messages", WS_CHILD | WS_VISIBLE | BS_GROUPBOX | BS_FLAT, FrameX, FrameY, 708, 209, m_hwnd, NULL, m_hinstance, NULL);
	m_list = CreateWindowEx(WS_EX_LEFT, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, FrameX + 6, FrameY + 19, 694, 186, m_hwnd, (HMENU)IDC_LIST, m_hinstance, NULL);
	
	FrameX = 12;
	FrameY = 267;
	m_Frame2 = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Num     CurPos        Start         End         Samples        Bytes        Rate       Bank       Code       Index     Seconds", WS_CHILD | WS_VISIBLE | BS_GROUPBOX | BS_FLAT, FrameX, FrameY, 708, 430, m_hwnd, NULL, m_hinstance, NULL);
	m_SampleList = CreateWindowEx(WS_EX_LEFT, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, FrameX + 6, FrameY + 19, 694, 400, m_hwnd, (HMENU)IDC_SAMPLE_LIST, m_hinstance, NULL);
		
	FrameX = 12;
	FrameY = 2;
	m_Frame3 = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Playback Controls", WS_CHILD | WS_VISIBLE | BS_GROUPBOX | BS_FLAT, FrameX, FrameY, 708, 45, m_hwnd, NULL, m_hinstance, NULL);
	m_PlayBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Play", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 5, FrameY + 16, 51, 22, m_hwnd, (HMENU)IDC_PLAY, m_hinstance, NULL);
	m_StopBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Stop", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 61, FrameY + 16, 51, 22, m_hwnd, (HMENU)IDC_STOP, m_hinstance, NULL);
	m_SampBox = CreateWindowEx(WS_EX_LEFT, L"STATIC", L"Sample Rate", WS_CHILD | WS_VISIBLE , FrameX + 115, FrameY + 16, 81, 22, m_hwnd, NULL, NULL, NULL);
	m_SampCtrl = CreateWindowEx(WS_EX_LEFT, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_READONLY, FrameX + 198, FrameY + 16, 51, 22, m_hwnd, (HMENU)IDC_SAMP, NULL, NULL);
	//	m_SaveBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Save WAV", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 117, FrameY + 16, 70, 22, m_hwnd, (HMENU)IDC_SAVE, m_hinstance, NULL);
//	m_RawBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Save RAW", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 190, FrameY + 16, 70, 22, m_hwnd, (HMENU)IDC_RAW, m_hinstance, NULL);
	m_PauseBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Pause", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 260, FrameY + 16, 51, 22, m_hwnd, (HMENU)IDC_PAUSE, m_hinstance, NULL);
	m_LoopBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Loop", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_AUTOCHECKBOX | BS_FLAT, FrameX + 315, FrameY + 16, 51, 22, m_hwnd, (HMENU)IDC_LOOP, m_hinstance, NULL);

	//Set Fonts
	SendMessage(m_SampleList, WM_SETFONT, (WPARAM)m_CourierNewFnt, NULL);
	SendMessage(m_list, WM_SETFONT, (WPARAM)m_CourierNewFnt, NULL);
	SendMessage(m_Frame1, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_Frame2, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_Frame3, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_PlayBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_StopBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_SampBox, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	//	SendMessage(m_SaveBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_RawBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_PauseBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_LoopBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
			
	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(m_hinstance, MAKEINTRESOURCE(IDI_SOUNDTOOL));
	wc.hIconSm = LoadIcon(m_hinstance, MAKEINTRESOURCE(IDI_SMALL));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"ROMLoad";
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);
	
	//ROM Loader Window
	m_ROMhwnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"ROMLoad", L"ROM Loader", WS_OVERLAPPEDWINDOW, posX, posY, 425, 315, m_hwnd, NULL, m_hinstance, NULL);

	//Group Frame 1
	FrameX = 10;
	FrameY = 7;
	m_ROMFrame1 = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"ROMs", WS_CHILD | WS_VISIBLE | BS_GROUPBOX | BS_FLAT, FrameX, FrameY, 388, 176, m_ROMhwnd, (HMENU)IDC_ROM_FRAME1, m_hinstance, NULL);
	//*Contents of Group Frame 1*

	//ROM Listbox
	m_ROMList = CreateWindowEx(WS_EX_LEFT, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, FrameX + 6, FrameY + 19, 376, 95, m_ROMhwnd, (HMENU)IDC_ROM_LIST, m_hinstance, NULL);

	//Buttons Add, Del, Clear
	m_ROMAddBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Add", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 331, FrameY + 118, 51, 22, m_ROMhwnd, (HMENU)IDC_ROM_ADD, m_hinstance, NULL);
	m_ROMDelBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Del", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 274, FrameY + 118, 51, 22, m_ROMhwnd, (HMENU)IDC_ROM_DEL, m_hinstance, NULL);
	m_ROMClearBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Clear", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 6, FrameY + 118, 51, 22, m_ROMhwnd, (HMENU)IDC_ROM_CLEAR, m_hinstance, NULL);
	//Buttons OK, Cancel
	m_ROMOKBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, 328, 245, 70, 26, m_ROMhwnd, (HMENU)IDC_ROM_OK, m_hinstance, NULL);
	m_ROMCancelBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, 252, 245, 70, 26, m_ROMhwnd, (HMENU)IDC_ROM_CANCEL, m_hinstance, NULL);

	//Set Fonts
	SendMessage(m_ROMFrame1, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMList, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMAddBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMDelBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMClearBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMOKBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMCancelBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);

	//Group Frame 2
	FrameX = 10;
	FrameY = 159;
	m_ROMFrame2 = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Format Detection", WS_CHILD | WS_VISIBLE | BS_GROUPBOX | BS_FLAT, FrameX, FrameY, 388, 80, m_ROMhwnd, (HMENU)IDC_ROM_FRAME2, m_hinstance, NULL);

	//* Contents of group frame 2 *

	//Combo box detection type
	m_ROMCombo = CreateWindowEx(WS_EX_LEFT, L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWN, FrameX + 6, FrameY + 19, 376, 300, m_ROMhwnd, (HMENU)IDC_ROM_COMBO, m_hinstance, NULL);
	m_ROMStyleCombo = CreateWindowEx(WS_EX_LEFT, L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWN, FrameX + 6, FrameY + 49, 376, 300, m_ROMhwnd, (HMENU)IDC_ROM_STYLE_COMBO, m_hinstance, NULL);

	//Set Combo Box Options
	SendMessage(GetDlgItem(m_ROMhwnd, IDC_ROM_COMBO), CB_ADDSTRING, 0, (LPARAM)L"Automatic");
	SendMessage(GetDlgItem(m_ROMhwnd, IDC_ROM_COMBO), CB_ADDSTRING, 0, (LPARAM)L"Force OKI");
	SendMessage(GetDlgItem(m_ROMhwnd, IDC_ROM_COMBO), CB_ADDSTRING, 0, (LPARAM)L"Force NEC");
	SendMessage(GetDlgItem(m_ROMhwnd, IDC_ROM_COMBO), CB_ADDSTRING, 0, (LPARAM)L"Force YAMAHA");
	SendMessage(GetDlgItem(m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_ADDSTRING, 0, (LPARAM)L"Not Required");



	//Set the option to option 0
	SendMessage(GetDlgItem(m_ROMhwnd, IDC_ROM_COMBO), CB_SETCURSEL, 0, 0);
	SendMessage(GetDlgItem(m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_SETCURSEL, 0, 0);

	//Set Fonts
	SendMessage(m_ROMFrame2, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMCombo, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMStyleCombo, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);

	m_WROMhwnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"ROMLoad", L"Williams ROM Loader", WS_OVERLAPPEDWINDOW, posX, posY, 425, 315, m_hwnd, NULL, m_hinstance, NULL);
	//Group Frame 1
	FrameX = 10;
	FrameY = 7;
	m_ROMFrame1 = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"U12", WS_CHILD | WS_VISIBLE | BS_GROUPBOX | BS_FLAT, FrameX, FrameY, 388, 176, m_WROMhwnd, (HMENU)IDC_ROM_FRAME1, m_hinstance, NULL);
	//*Contents of Group Frame 1*

	//ROM Listbox
	m_ROMList = CreateWindowEx(WS_EX_LEFT, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, FrameX + 6, FrameY + 19, 376, 60, m_WROMhwnd, (HMENU)IDC_U12_LIST, m_hinstance, NULL);

	//Buttons Add, Del, Clear
	m_ROMAddBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Add", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 331, FrameY + 80, 51, 22, m_WROMhwnd, (HMENU)IDC_U12_ADD, m_hinstance, NULL);
	m_ROMDelBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Del", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 274, FrameY + 80, 51, 22, m_WROMhwnd, (HMENU)IDC_U12_DEL, m_hinstance, NULL);
	m_ROMClearBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Clear", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 6, FrameY + 80, 51, 22, m_WROMhwnd, (HMENU)IDC_U12_CLEAR, m_hinstance, NULL);

	SendMessage(m_ROMFrame1, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMList, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMAddBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMDelBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMClearBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMOKBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMCancelBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);

	//Group Frame 2
	FrameX = 10;
	FrameY = 120;

	m_ROMFrame2 = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"U13", WS_CHILD | WS_VISIBLE | BS_GROUPBOX | BS_FLAT, FrameX, FrameY, 388, 176, m_WROMhwnd, (HMENU)IDC_ROM_FRAME2, m_hinstance, NULL);

	//ROM Listbox
	m_ROMList2 = CreateWindowEx(WS_EX_LEFT, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, FrameX + 6, FrameY + 19, 376, 60, m_WROMhwnd, (HMENU)IDC_U13_LIST, m_hinstance, NULL);

	//Buttons Add, Del, Clear
	m_ROMAddBn2 = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Add", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 331, FrameY + 80, 51, 22, m_WROMhwnd, (HMENU)IDC_U13_ADD, m_hinstance, NULL);
	m_ROMDelBn2 = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Del", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 274, FrameY + 80, 51, 22, m_WROMhwnd, (HMENU)IDC_U13_DEL, m_hinstance, NULL);
	m_ROMClearBn2 = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Clear", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, FrameX + 6, FrameY + 80, 51, 22, m_WROMhwnd, (HMENU)IDC_U13_CLEAR, m_hinstance, NULL);

	//Buttons OK, Cancel
	m_ROMOKBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, 328, FrameY + 120, 70, 26, m_WROMhwnd, (HMENU)IDC_WROM_OK, m_hinstance, NULL);
	m_ROMCancelBn = CreateWindowEx(WS_EX_LEFT, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_PUSHBUTTON | BS_FLAT, 252, FrameY + 120, 70, 26, m_WROMhwnd, (HMENU)IDC_ROM_CANCEL, m_hinstance, NULL);

	//Set Fonts
	SendMessage(m_ROMFrame2, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMList2, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMAddBn2, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMDelBn2, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMClearBn2, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMOKBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);
	SendMessage(m_ROMCancelBn, WM_SETFONT, (WPARAM)m_ArialFnt, NULL);


	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// show the mouse cursor.
	ShowCursor(true);

	return;
}

bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;


	// Initialize the width and height of the screen to zero before sending the variables into the function.
	screenWidth = 0;
	screenHeight = 0;

	// Initialize the windows api.
	InitializeWindows(screenWidth, screenHeight);

	// Create the application object.
	m_Application = new ApplicationClass;
	if(!m_Application)
	{
		return false;
	}

	// Initialize the application object.
	if(!m_Application->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight))
	{
		MessageBox(m_hwnd, L"Could not initialize the application object.", L"Error", MB_OK);
		return false;
	}
	
	return true;
}


void SystemClass::Shutdown()
{
	// Release the graphics object.
	if(m_Application)
	{
		m_Application->Shutdown();
		delete m_Application;
		m_Application = 0;
	}

	// Shutdown the window.
	ShutdownWindows();
	
	return;
}


void SystemClass::Run()
{
	MSG msg;
	bool done;
	HACCEL hAccelTable = LoadAccelerators(m_hinstance, MAKEINTRESOURCE(IDC_SOUNDTOOL));

	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));
	
	// Loop until there is a quit message from the window or the user.
	done = false;
	while(!done)
	{
		// Handle the windows messages.
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		// If windows signals to end the application then exit out.
		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.			
			if(!Frame())
			{
				done = true;
			}
		}

	}

	return;
}


bool SystemClass::Frame()
{
	// Do the frame processing for the application object.
	if(!m_Application->Frame())
	{
		return false;
	}

	return true;

}


LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

LRESULT CALLBACK SystemClass::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	HINSTANCE tinst = GetModuleHandle(NULL);

	int LW = LOWORD(wparam);
	int HW = HIWORD(wparam);	

	switch (umessage)
	{
		// Check if the window is being destroyed.
	case WM_COMMAND:
	{		
		
		// Parse the menu selections:
		switch (LW)
		{
		case IDC_PLAY:
			switch (HW) {
			case BN_CLICKED:
				SndTool* Snd = ApplicationHandle->m_Application->GetSnd();
				int i = (int)SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_SAMPLE_LIST), LB_GETCURSEL, NULL, NULL);
				if (i > 0) {
					Snd->PlaySample(i);
				}
				return true;
				break;
			}
			break;
		case IDC_STOP:
			switch (HW) {
			case BN_CLICKED:
				SndTool* Snd = ApplicationHandle->m_Application->GetSnd();
				Snd->StopSample();
				return true;
				break;
			}
			break;
		case IDC_RAW:
			switch (HW) {
			case BN_CLICKED:
				SndTool * Snd = ApplicationHandle->m_Application->GetSnd();
				int i = (int)SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_SAMPLE_LIST), LB_GETCURSEL, NULL, NULL);
				Snd->SaveSampleRAW(i);
				return true;
				break;
			}
			break;
		case IDC_PAUSE:
			switch (HW) {
			case BN_CLICKED:
				SndTool* Snd = ApplicationHandle->m_Application->GetSnd();
				Snd->PauseSample();
				return true;
				break;
			}
			break;
		case IDC_LOOP:
			switch (HW) {
			case BN_CLICKED:
				SndTool* Snd = ApplicationHandle->m_Application->GetSnd();
				int i = (int)SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_LOOP), BM_GETCHECK, NULL, NULL);
				Snd->SetLoop(i);
				return true;
				break;
			}
			break;
		case IDC_ROM_COMBO:
			switch (HW) {
			case CBN_SELCHANGE:
				int i = (int)SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_COMBO), CB_GETCURSEL, NULL, NULL);
				switch (i) {
				case OKIType:
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_RESETCONTENT, NULL, NULL);
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_ADDSTRING, 0, (LPARAM)L"m6376 Style");
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_ADDSTRING, 0, (LPARAM)L"m6295 Style");
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_SETCURSEL, NULL, NULL);
					break;
				case 3://YMZ
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_RESETCONTENT, NULL, NULL);
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_ADDSTRING, 0, (LPARAM)L"Scorp2 Style");
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_ADDSTRING, 0, (LPARAM)L"Epoch1 Style");
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_ADDSTRING, 0, (LPARAM)L"Epoch2 Style");
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_ADDSTRING, 0, (LPARAM)L"Scorp4 Style");
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_ADDSTRING, 0, (LPARAM)L"Scorp5 Style");					
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_SETCURSEL, NULL, NULL);
					break;
				default:
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_RESETCONTENT, NULL, NULL);
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_ADDSTRING, 0, (LPARAM)L"Not Required");
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_SETCURSEL, NULL, NULL);
					break;
				}
				break;
			}
			return true;
			break;
		case IDC_SAMP:
			switch (HW) {
			case EN_CHANGE:
				SndTool* Snd = ApplicationHandle->m_Application->GetSnd();
				SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_SAMPLE_LIST), LB_RESETCONTENT, NULL, NULL);
				int len = 1 + GetWindowTextLengthW(GetDlgItem(ApplicationHandle->m_hwnd, IDC_SAMP));
				std::wstring str(len, L'\0');
				GetWindowTextW(GetDlgItem(ApplicationHandle->m_hwnd, IDC_SAMP), &str[0], len);

				int rate = std::stoi(str);
				SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_SAMPLE_LIST), LB_ADDSTRING, 0, (LPARAM)L"");

				Snd->Convert(rate);

				break;
			}
			return true;
			break;
		case IDC_SAMPLE_LIST:
			switch (HW) {
			case LBN_SELCHANGE:
				SndTool * Snd = ApplicationHandle->m_Application->GetSnd();


				int i = (int)SendMessage(GetDlgItem(ApplicationHandle->m_hwnd, IDC_SAMPLE_LIST), LB_GETCURSEL, NULL, NULL);
				if (i > 0) {
					Snd->PlaySample(i);
				}
				break;
			}
			return true;
			break;
		case IDC_U12_ADD:
			switch (HW) {
			case BN_CLICKED:
				SndTool * Snd = ApplicationHandle->m_Application->GetSnd();
				int numFiles = Snd->GetU12FileNames();
				if (numFiles) {
					int base = (int)SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_U12_LIST), LB_GETCOUNT, NULL, NULL);
					for (int cnt = 0; cnt < numFiles; cnt++) {
						SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_U12_LIST), LB_ADDSTRING, NULL, (LPARAM)Snd->GetWROMString(cnt + base,true));
					}
				}
				break;
			}
			return true;
			break;
		case IDC_U13_ADD:
			switch (HW) {
			case BN_CLICKED:
				SndTool * Snd = ApplicationHandle->m_Application->GetSnd();
				int numFiles = Snd->GetU13FileNames();
				if (numFiles) {
					int base = (int)SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_U13_LIST), LB_GETCOUNT, NULL, NULL);
					for (int cnt = 0; cnt < numFiles; cnt++) {
						SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_U13_LIST), LB_ADDSTRING, NULL, (LPARAM)Snd->GetWROMString(cnt + base,false));
					}
				}
				break;
			}
			return true;
			break;
		case IDC_ROM_ADD:
			switch (HW) {
			case BN_CLICKED:
				SndTool* Snd = ApplicationHandle->m_Application->GetSnd();
				int numFiles = Snd->GetFileNames();
				if (numFiles) {
					int base = (int)SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_LIST), LB_GETCOUNT, NULL, NULL);
					for (int cnt = 0; cnt < numFiles; cnt++) {
						SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_LIST), LB_ADDSTRING, NULL, (LPARAM)Snd->GetROMString(cnt + base));
					}
				}
				break; 
			}
			return true;
			break;
		case IDC_ROM_DEL:
			switch (HW) {
			case BN_CLICKED:
				SndTool* Snd = ApplicationHandle->m_Application->GetSnd();
				int i = (int)SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_LIST), LB_GETCURSEL, NULL, NULL);
				if (i >= 0) {
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_LIST), LB_DELETESTRING, (WPARAM)i, NULL);
					Snd->DeleteROMString(i);
				}
				break;
			}
			return true;
			break;
		case IDC_U12_DEL:
			switch (HW) {
			case BN_CLICKED:
				SndTool * Snd = ApplicationHandle->m_Application->GetSnd();
				int i = (int)SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_U12_LIST), LB_GETCURSEL, NULL, NULL);
				if (i >= 0) {
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_U12_LIST), LB_DELETESTRING, (WPARAM)i, NULL);
					Snd->DeleteWROMString(i,true);
				}
				break;
			}
			return true;
			break;
		case IDC_U13_DEL:
			switch (HW) {
			case BN_CLICKED:
				SndTool * Snd = ApplicationHandle->m_Application->GetSnd();
				int i = (int)SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_U13_LIST), LB_GETCURSEL, NULL, NULL);
				if (i >= 0) {
					SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_U13_LIST), LB_DELETESTRING, (WPARAM)i, NULL);
					Snd->DeleteWROMString(i,false);
				}
				break;
			}
			return true;
			break;
		case IDC_ROM_CLEAR:
			switch (HW) {
			case BN_CLICKED:
				SndTool* Snd = ApplicationHandle->m_Application->GetSnd();				
				Snd->ClearROMStrings(true);
				SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_LIST), LB_RESETCONTENT, NULL, NULL);
				break;
			}
			return true;
			break;
		case IDC_U12_CLEAR:
			switch (HW) {
			case BN_CLICKED:
				SndTool * Snd = ApplicationHandle->m_Application->GetSnd();
				Snd->ClearWROMStrings(true, true);
				SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_U12_LIST), LB_RESETCONTENT, NULL, NULL);
				break;
			}
			return true;
			break;
		case IDC_U13_CLEAR:
			switch (HW) {
			case BN_CLICKED:
				SndTool * Snd = ApplicationHandle->m_Application->GetSnd();
				Snd->ClearWROMStrings(true, true);
				SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_U13_LIST), LB_RESETCONTENT, NULL, NULL);
				break;
			}
			return true;
			break;
		case IDC_ROM_OK:
			switch (HW) {
			case BN_CLICKED:
				SndTool* Snd = ApplicationHandle->m_Application->GetSnd();
				int i = (int)SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_COMBO), CB_GETCURSEL, NULL, NULL);
				int j = (int)SendMessage(GetDlgItem(ApplicationHandle->m_ROMhwnd, IDC_ROM_STYLE_COMBO), CB_GETCURSEL, NULL, NULL);
				Snd->LoadSoundROMs(i, j);
				ShowWindow(ApplicationHandle->m_ROMhwnd, SW_HIDE);
				ShowWindow(ApplicationHandle->m_hwnd, SW_SHOW);
				break;
			}
			return true;
			break;
		case IDC_WROM_OK:
			switch (HW) {
			case BN_CLICKED:
				SndTool * Snd = ApplicationHandle->m_Application->GetSnd();
				Snd->LoadWilliamsSoundROMs();
				ShowWindow(ApplicationHandle->m_ROMhwnd, SW_HIDE);
				ShowWindow(ApplicationHandle->m_hwnd, SW_SHOW);
				break;
			}
			return true;
			break;
		case IDC_ROM_CANCEL:
			switch (HW) {
			case BN_CLICKED:
				ShowWindow(ApplicationHandle->m_ROMhwnd, SW_HIDE);
				ShowWindow(ApplicationHandle->m_hwnd, SW_SHOW);
				break;
			}
			return true;
			break;
		case IDC_ROM_LIST:
			switch (HW) {
			case LBN_DBLCLK:

				break;
			}
			break;
		case IDC_LIST:				
			switch (HW) {
			case LBN_DBLCLK:
				
				break;
			}		
			return true;
			break;		
		case IDM_ABOUT:
			DialogBox(tinst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
			return true;
			break;
		case ID_FILE_EXIT:
			DestroyWindow(hwnd);
			return 0;
			break;
		case ID_FILE_OPENW:
			ShowWindow(ApplicationHandle->m_WROMhwnd, SW_SHOW);		
			ShowWindow(ApplicationHandle->m_hwnd, SW_HIDE);
			return true;
			break;
		case ID_FILE_OPEN:
			ShowWindow(ApplicationHandle->m_ROMhwnd, SW_SHOW);
			ShowWindow(ApplicationHandle->m_hwnd, SW_HIDE);
			return true;
			break;
		case ID_EXPORT_ALL:
			//BasicFileSave();
			return true;
			break;
		case ID_EXPORT_CURRENT:
			//BasicFileSave();			
			return true;
			break;
		default:
			return DefWindowProc(hwnd, umessage, wparam, lparam);
		}
		break;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hwnd, &ps);
		return true;
		break;
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
		break;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
		break;
	}

	// All other messages pass to the message handler in the system class.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
	return true;
}


INT_PTR CALLBACK SystemClass::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{	
	// Message handler for about box.

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void SystemClass::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if(FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(szWindowClass, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return;
}


int SystemClass::ToWideString(WCHAR* &pwStr, const char* pStr, int len, BOOL IsEnd)
{
	/******************************************************
	*function:  convert multibyte character set to wide-character set
	*param:      pwStr--[out] Points to a buffer that receives the translated buffers.
	*            pStr--[in] Points to the multibyte character set(or string) to be converted.
	*            len --[in] Specify the size in bytes of the string pointed to by the pStr parameter,
	*                       or it can be -1 if the string is null terminated.
	*            IsEnd--[in]Specify whether you add '\0' to the end of converted array or not.
	*return: the length of converted set (or string )
	*******************************************************/

	int nWideLen = MultiByteToWideChar(CP_ACP, 0, pStr, len, NULL, 0);
	if (len == -1)
	{
		--nWideLen;
	}
	if (nWideLen == 0)
	{
		return 0;
	}
	if (IsEnd)
	{
		pwStr = new WCHAR[(nWideLen + 1) * sizeof(WCHAR)];
		ZeroMemory(pwStr, (nWideLen + 1) * sizeof(WCHAR));
	}
	else
	{
		pwStr = new WCHAR[nWideLen * sizeof(WCHAR)];
		ZeroMemory(pwStr, nWideLen * sizeof(WCHAR));
	}
	MultiByteToWideChar(CP_ACP, 0, pStr, len, pwStr, nWideLen);
	return nWideLen;
}


void SystemClass::SetOutputMsg(const wchar_t * InMsg, int Data) {

	wchar_t Temp[MAX_LOADSTRING];
	wchar_t Number[10];
	int Len1, Len2, TLen;

	_itow_s(Data, Number, sizeof(Number) / 2, 10);

	Len1 = lstrlenW(InMsg);
	Len2 = lstrlenW(Number);
	TLen = Len1 + Len2;

	for (int i = 0; i < Len1; i++) {
		Temp[i] = InMsg[i];
	}

	for (int i = 0; i < Len2; i++) {
		Temp[Len1 + i] = Number[i];
	}
	Temp[TLen] = NULL;

	SetOutput((LPARAM)Temp);

}


void SystemClass::SetOutputMsg(wchar_t * InMsg, wchar_t * Data) {

	wchar_t Temp[MAX_LOADSTRING];
	int Len1, Len2, TLen;

	Len1 = lstrlenW(InMsg);
	Len2 = lstrlenW(Data);
	TLen = Len1 + Len2;

	for (int i = 0; i < Len1; i++) {
		Temp[i] = InMsg[i];
	}
	for (int i = 0; i < Len2; i++) {
		Temp[Len1 + i] = Data[i];
	}
	Temp[TLen] = NULL;

	SetOutput((LPARAM)Temp);

}


void SystemClass::SetOutputMsg(const wchar_t * InMsg, wchar_t * Data) {

	wchar_t Temp[MAX_LOADSTRING];
	int Len1, Len2, TLen;

	Len1 = lstrlenW(InMsg);
	Len2 = lstrlenW(Data);
	TLen = Len1 + Len2;

	for (int i = 0; i < Len1; i++) {
		Temp[i] = InMsg[i];
	}
	for (int i = 0; i < Len2; i++) {
		Temp[Len1 + i] = Data[i];
	}
	Temp[TLen] = NULL;

	SetOutput((LPARAM)Temp);

}


void SystemClass::SetOutputMsg(const wchar_t * InMsg, char * Data, int CharLen) {

	wchar_t Out[MAX_LOADSTRING];
	wchar_t * TempChar;
	int Len1, TLen;

	Len1 = lstrlenW(InMsg);
	TLen = Len1 + CharLen;	

	if (ToWideString(TempChar, Data, CharLen, true) > 0) {

		for (int i = 0; i < Len1; i++) {
			Out[i] = InMsg[i];
		}
		for (int i = 0; i < CharLen; i++) {
			Out[Len1 + i] = Data[i];
		}
		Out[TLen] = NULL;

		delete(TempChar);

		SetOutput((LPARAM)Out);
	}

}


void SystemClass::SetOutputMsg(const wchar_t * InMsg, const wchar_t * Data) {

	wchar_t Temp[MAX_LOADSTRING];
	int Len1, Len2, TLen;

	Len1 = lstrlenW(InMsg);
	Len2 = lstrlenW(Data);
	TLen = Len1 + Len2;

	for (int i = 0; i < Len1; i++) {
		Temp[i] = InMsg[i];
	}
	for (int i = 0; i < Len2; i++) {
		Temp[Len1 + i] = Data[i];
	}
	Temp[TLen] = NULL;

	SetOutput((LPARAM)Temp);

}