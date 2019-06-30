////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{	
	m_Snd = 0;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}


bool ApplicationClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{	
	// Store the screen width and height.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	//Create the Sound Tool Device
	m_Snd = new SndTool;
	if (!m_Snd) {
		return false;
	}
	   
	return true;
}

SndTool * ApplicationClass::GetSnd() {
	return m_Snd;
}

void ApplicationClass::Shutdown()
{
	// Release the input object.
	if(m_Snd)
	{		
		delete m_Snd;
		m_Snd = 0;
	}

	return;
}


bool ApplicationClass::Frame()
{
	m_Snd->UpdateBass();
	return true;	
}
