#include "Window.h"

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_win32.h"

Window Win;

bool IsColorPicker = false;

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	/*case WM_CREATE:
		// get the height and width of the screen
		int _width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int _height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

		//Capture screenshot bitmap
		HBITMAP hBitmap = Misc.ScreenCapture(0, 0, _width, _height);

		//Set image to background
		Win.SetImage(hBitmap);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT     ps;
		BITMAP          bitmap;
		HBITMAP			hBitmap;
		HGDIOBJ         oldBitmap;
		HDC             hdc, hdcMem;

		hdc = BeginPaint(hWnd, &ps);
		hBitmap = Win.GetImage();
		hdcMem = CreateCompatibleDC(hdc);
		oldBitmap = SelectObject(hdcMem, hBitmap);

		GetObject(hBitmap, sizeof(bitmap), &bitmap);
		BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);

		EndPaint(hWnd, &ps);

		break;
	}*/
	case WM_LBUTTONDOWN:
		IsColorPicker = true;
		break;
	case WM_LBUTTONUP:
		IsColorPicker = false;
		break;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

bool Window::OnCreateWindow(int width, int height)
{
	// Create application window
	wc =
	{
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		WndProc,
		0L, 0L,
		GetModuleHandle(NULL),
		NULL,
		NULL,
		NULL,
		NULL,
		WINDOW_CLASS,
		NULL
	};

	if (!::RegisterClassEx(&wc))
		return false;

	//WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW
	hWnd = ::CreateWindow(wc.lpszClassName, WINDOW_TEXT, WS_POPUP, 0, 0, width, height, NULL, NULL, wc.hInstance, NULL);

	if (!hWnd) {
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return false;
	}

	// Show the window
	::ShowWindow(hWnd, SW_SHOWDEFAULT);
	::UpdateWindow(hWnd);

	return true;
}

bool Window::OnUpdateMsg()
{
	MSG msg;
	if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	return true;
}

HBITMAP Window::GetImage()
{
	return hBitmap;
}

void Window::SetImage(HBITMAP hBitmap)
{
	this->hBitmap = hBitmap;
}

void Window::Release()
{
	::DestroyWindow(hWnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);
}