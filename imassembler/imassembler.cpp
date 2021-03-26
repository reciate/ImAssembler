#include <Windows.h>
#include <iostream>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	}
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSA window_class{ 0, WindowProc, 0, 0, hInstance, 0, 0, 0, 0, "WindowClass" };

	RegisterClassA(&window_class);

	HWND hwnd{ CreateWindowExA(0, "WindowClass", "ImAssembler", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL) };

	if (!hwnd) return 0;

	ShowWindow(hwnd, nCmdShow);

	MSG message{};

	while (GetMessageA(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}

	return 0;
}