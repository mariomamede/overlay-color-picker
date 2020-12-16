#pragma once
#include <Windows.h>
#include <D3D11.h>
#include <stdio.h>
#include <vector>

#include "../resource.h"
#include "../Misc/Misc.h"
#include "../ImGui/imgui.h"
#include "../Window/Window.h"
#include "../ImGui/IconsFontAwesome5.h"
#include "../ImGui/imgui_impl_win32.h"
#include "../ImGui/imgui_impl_dx11.h"

#define GUI_WINDOW_TEXT "Overlay Color Picker"
#define GUI_WINDOW_CLASS "GUI Class"

class CInterface
{
public:
	void InitTrayIcon();

	void ShowContextMenu(HWND hWnd);

	bool OnCreateWindow(int width, int height);

	void Render();

	bool IsMouseOnGui();

	bool IsMouseOnTitleBar();

	void SetupWindowPos(float x, float y) {
		window_pos = ImVec2(x, y);
	}

	void SetupWindowSize(float w, float h) {
		window_size = ImVec2(w, h);
	}

	void UpdateColor(ImColor Color) {
		color = Color;
	}

	HWND GetWindow() {
		return hWnd;
	}

	void Release();
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();

private:
	HWND hWnd;
	WNDCLASSEX wc;
	ImColor color;
	ImVec2 window_pos;
	ImVec2 window_size;
	std::vector<ImColor> color_array;

	bool OnCreateDevice(HWND hWnd);

	void OnSetupStyle();
	void OnSetupFont();
	void OnBuildInterface();
};

extern CInterface Gui;