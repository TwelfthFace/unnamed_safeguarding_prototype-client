#pragma once

#include <objidl.h>
#include <Windows.h>
#include <gdiplus.h>
#include <thread>
#include <atomic>
#include <string>

class ScreenLocker {
public:
    ScreenLocker();
    ~ScreenLocker();
    void runInThread();
    void waitForThread();
    void closeWindow();
    bool isLocked();
    static std::wstring picture_path;

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static Gdiplus::Bitmap* pImage;
    std::thread imageThread;
    std::atomic<bool> isWindowOpen;
    HWND hWnd;
    static ULONG_PTR gdiplusToken;

    int run(HINSTANCE hInstance, int nCmdShow);
    static void runThread(ScreenLocker* pScreenLocker);
};