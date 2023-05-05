#include "ScreenLocker.h"

using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")

std::wstring ScreenLocker::picture_path = L"lockscreen.bmp";
Gdiplus::Bitmap* ScreenLocker::pImage = nullptr;

ScreenLocker::ScreenLocker()
    : isWindowOpen(false), hWnd(nullptr) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

ScreenLocker::~ScreenLocker() {
    GdiplusShutdown(gdiplusToken);
}

void ScreenLocker::runInThread() {
    if (!isWindowOpen) {
        isWindowOpen = true;
        imageThread = std::thread(runThread, this);
    }
}

void ScreenLocker::runThread(ScreenLocker* pScreenLocker) {
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    int nCmdShow = SW_SHOW;
    pScreenLocker->run(hInstance, nCmdShow);
    pScreenLocker->isWindowOpen = false;
}


void ScreenLocker::waitForThread() {
    if (imageThread.joinable()) {
        imageThread.join();
    }
}

void ScreenLocker::closeWindow() {
    if (isWindowOpen) {
        if (hWnd != nullptr) {
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        }
    }
}

bool ScreenLocker::isLocked()
{
    return isWindowOpen;
}

int ScreenLocker::run(HINSTANCE hInstance, int nCmdShow) {
    const wchar_t szClassName[] = L"ScreenLocker";

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = szClassName;

    RegisterClassEx(&wc);

    HWND hWndTemp = CreateWindowEx(
        WS_EX_TOPMOST,
        szClassName,
        L"Fullscreen Image",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    hWnd = hWndTemp;

    if (!hWndTemp) {
        MessageBox(nullptr, L"Failed to create window.", L"Error", MB_OK);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

ULONG_PTR ScreenLocker::gdiplusToken; // Initialize the GdiplusToken variable

LRESULT CALLBACK ScreenLocker::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static Gdiplus::Image* pImage = nullptr;

    switch (message) {
    case WM_CREATE:
    {
        // Initialize GDI+ and load the image
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        pImage = Gdiplus::Image::FromFile(ScreenLocker::picture_path.c_str(), FALSE);
        if (pImage->GetLastStatus() != Gdiplus::Ok) {
            //"Failed to load image", L"Error", MB_OK | MB_ICONERROR);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        Gdiplus::Graphics graphics(hdc);
        if (pImage) {
            graphics.DrawImage(pImage, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        }

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
    {
        delete pImage;

        // Shutdown GDI+
        Gdiplus::GdiplusShutdown(gdiplusToken);
        PostQuitMessage(0);
    }
    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}