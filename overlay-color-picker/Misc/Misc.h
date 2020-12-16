#pragma once
#include <Windows.h>
#include <stdio.h>

#include "../ImGui/imgui.h"

class CMisc
{
public:
	HBITMAP ScreenCapture(int x, int y, int w, int h, HWND hWnd = nullptr);

	//Get Pixel color
	ImColor GetPixelColor(ImVec2 Pos, HWND hWnd = nullptr);

	//Get Pixel Average color
	ImColor GetPixelAverageColor(ImVec2 Pos, HWND hWnd = nullptr);

	//Convert ImColor to RGB Hex
	ImU32 ImColorToHex(ImColor Color);

	//Get pixel color on mouse position
	ImColor GetColorOnMousePosition(HWND hWnd = nullptr);

	//Copy color to clipboard
	void CopyToClipboard(ImColor Color);
};

extern CMisc Misc;