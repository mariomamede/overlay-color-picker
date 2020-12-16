#pragma once
#include <Windows.h>

#include "../GUI/GUI.h"
#include "../Misc/Misc.h"

#define WINDOW_TEXT "OCP - Overlay"
#define WINDOW_CLASS "OCP Class"

class Window
{
public:
	bool OnCreateWindow(int width, int height);

	bool OnUpdateMsg();

	HBITMAP GetImage();

	void SetImage(HBITMAP hBitmap);

	HWND GetWindow() {
		return hWnd;
	}

	void Release();

private:
	HWND hWnd;
	WNDCLASSEX wc;
	HBITMAP hBitmap;
};

extern Window Win;