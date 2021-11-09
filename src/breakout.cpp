// breakout.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "breakout.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    GameMainWindow mainWnd;

    int ret = 0;

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    std::srand(GetTickCount());

    if (mainWnd.Init(hInstance, nCmdShow))
    {
        ret = mainWnd.Run();
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);

    return ret;
}
