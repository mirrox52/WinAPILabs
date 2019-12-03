#include <Windows.h>

LRESULT WindowProc (HWND, UINT, WPARAM, LPARAM);
void Create (HINSTANCE);
void CreateBackground ();
void RecalculateSize (HWND);
void Draw ();
void DrawLines ();
void DrawRaws ();
void WriteText ();
int GetTextLength (const wchar_t *);

#define ForceRedraw(hWnd) InvalidateRect (hWnd, 0, 0); prev_Time = 0;
#define UpdateMax(value) if (value > max_Height) max_Height = value;