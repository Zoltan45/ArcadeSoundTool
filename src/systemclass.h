////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

//Specify that we want to use common controls version 6, this allows the nicer window styles.
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define WIN32_LEAN_AND_MEAN

#define MAX_LOADSTRING 100

//////////////
// INCLUDES //
//////////////
#include <windows.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "applicationclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: SystemClass
////////////////////////////////////////////////////////////////////////////////
class SystemClass
{
public:
	
	SystemClass();	
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
	
	void SetOutputMsg(wchar_t *, int);
	void SetOutputMsg(const wchar_t *);
	void SetSampleCtrl(int rate);
	void SetOutputMsg(wchar_t *, wchar_t *);
	void SetOutputMsg(const wchar_t *, int);
	void SetOutputMsg(const wchar_t *, wchar_t *);
	void SetOutputMsg(const wchar_t *, char *, int);
	void SetOutputMsg(const wchar_t *, const wchar_t *);
	
	void ClearROMOutput();
	void ClearMainOutput();

	void SetSampleList(int SampleNumber, int cur, int start, int end, int samples, int bytes, int rate, int realbank, int bank, int index, float Seconds);
	void SetSampleList(int SampleNumber, int start, int end, int samples, int bytes, int rate, int bank, int index, float Seconds);

private:

	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

	//
	WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
	WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
	
	//Fonts
	HFONT m_CourierNewFnt, m_ArialFnt;
	
	//Main Form & Controls
	HWND m_hwnd, m_list, m_Frame1, m_Frame2, m_Header, m_Frame3, m_SampleList, m_SampBox, m_SampCtrl, m_PlayBn, m_StopBn, m_SaveBn, m_RawBn, m_LoopBn, m_PauseBn;
	
	//Output Handler
	void SetOutput(LPARAM);

	//ROM Loader Form & Controls
	HWND m_ROMhwnd, m_WROMhwnd, m_ROMList, m_ROMCombo, m_ROMStyleCombo, m_ROMAddBn, m_ROMClearBn, m_ROMDelBn, m_ROMOKBn, m_ROMCancelBn, m_ROMFrame1, m_ROMFrame2, m_ROMList2, m_ROMAddBn2, m_ROMClearBn2, m_ROMDelBn2;

	//Char * to wchar_t * conversion for ROM Names
	int ToWideString(WCHAR* &pwStr, const char* pStr, int len, BOOL IsEnd);
	
private:

	HINSTANCE m_hinstance;
	ApplicationClass* m_Application;

};


/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////

/////////////
// GLOBALS //
/////////////
static SystemClass* ApplicationHandle = 0;


#endif