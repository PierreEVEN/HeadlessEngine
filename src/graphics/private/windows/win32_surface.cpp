#include "win32_surface.h"

#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <tchar.h>

const TCHAR      CLSNAME[] = TEXT("helloworldWClass");
LRESULT CALLBACK winproc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp);

namespace gfx::win32
{

Win32Window::Win32Window()
{
}

LRESULT CALLBACK winproc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
    return DefWindowProc(hwnd, wm, wp, lp);
}

Win32Window::~Win32Window()
{
    
}
} // namespace gfx::win32