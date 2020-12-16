#include "Misc.h"

CMisc Misc;

HBITMAP CMisc::ScreenCapture(int x, int y, int w, int h, HWND hWnd)
{
	HDC hdc = GetDC(hWnd); // get the desktop device context
	HDC hDest = CreateCompatibleDC(hdc); // create a device context to use yourself

	// create a bitmap
	HBITMAP hbDesktop = CreateCompatibleBitmap(hdc, w, h);

	// use the previously created device context with the bitmap
	SelectObject(hDest, hbDesktop);

	// copy from the desktop device context to the bitmap device context
	// call this once per 'frame'
	BitBlt(hDest, x, y, w, h, hdc, 0, 0, SRCCOPY);

	// after the recording is done, release the desktop context you got..
	ReleaseDC(NULL, hdc);

	// ..and delete the context you created
	DeleteDC(hDest);

	return hbDesktop;
}

ImColor CMisc::GetPixelColor(ImVec2 Pos, HWND hWnd)
{
	// Get global DC
	HDC hDc = ::GetDC(hWnd);

	// Get pixel color ref
	COLORREF color_ref = ::GetPixel(hDc, Pos.x, Pos.y);

	return ImGui::ColorConvertU32ToFloat4(color_ref);
}

ImColor CMisc::GetPixelAverageColor(ImVec2 Pos, HWND hWnd)
{
	// Get global DC
	HDC hDc = ::GetDC(hWnd);

	//Offset in pixels
	float offset = 5.f;

	// Center
	COLORREF cor_center = ::GetPixel(hDc, Pos.x, Pos.y);
	// Left
	COLORREF cor_left = ::GetPixel(hDc, Pos.x - offset, Pos.y);
	// Right
	COLORREF cor_right = ::GetPixel(hDc, Pos.x + offset, Pos.y);
	// Top
	COLORREF cor_top = ::GetPixel(hDc, Pos.x, Pos.y - offset);
	// Bottom
	COLORREF cor_bottom = ::GetPixel(hDc, Pos.x, Pos.y + offset);
	// Top left
	COLORREF cor_top_left = ::GetPixel(hDc, Pos.x - offset, Pos.y - offset);
	// Top Right
	COLORREF cor_top_right = ::GetPixel(hDc, Pos.x + offset, Pos.y - offset);
	// Bottom left
	COLORREF cor_bottom_left = ::GetPixel(hDc, Pos.x - offset, Pos.y + offset);
	// Bottom Right
	COLORREF cor_bottom_right = ::GetPixel(hDc, Pos.x + offset, Pos.y + offset);

	// Calcule Average
	unsigned long long average = (cor_center + cor_left + cor_right + cor_top + cor_bottom + cor_top_left + cor_top_right + cor_bottom_left + cor_bottom_right) / 9;

	return ImGui::ColorConvertU32ToFloat4(average);
}

ImColor CMisc::GetColorOnMousePosition(HWND hWnd)
{
	POINT _cursor;
	::GetCursorPos(&_cursor);

	return GetPixelColor(ImVec2(_cursor.x, _cursor.y), hWnd);
}

ImU32 CMisc::ImColorToHex(ImColor Color)
{
	ImU32 color_ref = Color.operator ImU32();

	//Get pixel color
	int red = GetRValue(color_ref);
	int green = GetGValue(color_ref);
	int blue = GetBValue(color_ref);

	return ((red & 0xff) << 16) + ((green & 0xff) << 8) + (blue & 0xff);
}

void CMisc::CopyToClipboard(ImColor Color)
{
	//buffer
	char hex_formart[10];

	//hex formart
	sprintf_s(hex_formart, 10, "%06X", ImColorToHex(Color));

	ImGui::LogToClipboard();
	ImGui::LogText(hex_formart);
	ImGui::LogFinish();
}