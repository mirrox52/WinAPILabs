#include "stdafx.h"
#include "lab_2.h"

int prev_Time = 0, max_Height, vertical_Step, horizontal_Step, len, written, N = 10, M = 5;
LPMINMAXINFO lpMMI;																				// Содержит информацию о максимальном размере и положении окна, а также его минимальном и максимальном размере отслеживания
BOOL move_Right = 0, move_Left = 0, move_Up = 0, move_Down = 0, catched = 0;
RECT clientRect = { 0, 0, 800, 600 };															// Структура, определяющая прямоугольник (картинка)
HDC hdcBack;																					// Дескриптор контекста устройства
HPEN hpBorder;																					// обводка (цвет шрифта)
HBITMAP hbmBack;																				
RECT r, textRect;
TEXTMETRIC metrics;																				// параметры текста, шрифта
SIZE size;
const wchar_t *caption = L"Table";
#define className L"MainClass"
const wchar_t **textPointer;																	// матрица

#define str_frst L"Sample text"
#define str_scnd L"Another sample text!"
#define str_thrd L"Short text example."
#define str_frth L"This text example is a bit longer"
#define str_fifth L"And this string is the longest text sample!"

const wchar_t *strings[50] =
{
	str_frst, 0, 0, 0, 0,
	0, str_scnd, 0, 0, 0,
	0, 0, str_thrd, 0, 0,
	0, 0, 0, str_frth, 0,
	0, 0, 0, 0, str_fifth,
	str_frst, 0, 0, 0, 0,
	0, str_scnd, 0, 0, 0,
	0, 0, str_thrd, 0, 0,
	0, 0, 0, str_frth, 0,
	0, 0, 0, 0, str_fifth
};

WNDCLASSEX WndClassEx = { sizeof(WNDCLASSEX), 0, (WNDPROC)WindowProc, 0, 0, 0, 0, 0, 0, 0, className, 0 }; // класс окна

INT WINAPI WinMain(HINSTANCE hInstance,															// Описатель (дескриптор) загруженного в память модуля; адрес виртуальной памяти, с которого загружена программа
	HINSTANCE hPrevInstance,																	// дескриптор предыдущего экземпляра окна , в современных системах всегда 0
	LPSTR lpCmdLine,																			// Указатель командной строки
	int nCmdShow																				// Режим отображения окна (min, max, normal)
)
{

	MSG msg;

	
	Create(hInstance);

	while (GetMessage(&msg,																		// указатель на структуру
		0,																						// указатель окна чьи сообщения нужно обрабатывать
		0, 0))
	{
		DispatchMessageW(&msg);																	// Посылает сообщение функции WndProc()
	}

	ExitProcess(0);

}

LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	lpMMI = (LPMINMAXINFO)lParam;																// Содержит информацию о максимальном размере и положении окна, а также его минимальном и максимальном размере отслеживания
	PAINTSTRUCT ps;																				// структура для отрисовки
	HDC hdc;
	int time;

	switch (uMsg)
	{
	case WM_SIZE:																				// Сообщение WM_SIZE посылается окну после того, как его размер изменился
		RecalculateSize(hWnd);
		return 0;
	case WM_GETMINMAXINFO:																		// Сообщение WM_GETMINMAXINFO отправляется окну тогда, когда размер или позиция окна собираются измениться
		lpMMI->ptMinTrackSize.x = 700;
		lpMMI->ptMinTrackSize.y = 150;
		return 0;
	case WM_DESTROY:																			// Это сообщение посылается когда необходимо уничтожить окно
		PostQuitMessage(0);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);										// вызывается оконной процедурой по умолчанию, чтобы обеспечить обработку по умолчанию любого сообщения окна,
																								// которые приложение не обрабатывает

	case WM_KEYDOWN:																			// когда нажата несистемная клавиша
		if (wParam == VK_ESCAPE /*Клавиша Esc*/) {
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_PAINT:																				// запрос на окрашивание окна
		
			// Заполняет пpямоугольник, используя кисть Brush, до пpавой и нижней гpаниц
			FillRect(hdcBack,																	// Идентификатоp контекста устpойства
				&clientRect,																	// Заполняемый TRect
				0																				// Кисть заполнения
			);
			Draw();
		
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcBack, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		InvalidateRect(hWnd, 0, 0);
		return 0;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

}

void Create(HINSTANCE hInstance)
{

	HWND hWnd;																					// Описатель главного окна программы
	RECT R;

	R = clientRect;																				// Сохранение клиентской области
	//вычисляет требуемый размер прямоугольника окна, основанного на желаемом пользователем размере прямоугольника
	AdjustWindowRect(&R,																		// указатель на структуру
																								// прямоугольника пользователя
		WS_OVERLAPPEDWINDOW,																	// стиль окон (перекрывающие)
		0																						// флажок показа меню
	);												

	WndClassEx.hInstance = hInstance;
	// позволяет получить нам указатель на курсор
	WndClassEx.hCursor = LoadCursor(0,															//  указатель на модуль с курсором
		IDC_ARROW																				// имя курсора
	);

	RegisterClassEx(&WndClassEx);																// Регистрация класса окна

	// Создание окна (расширенный класс окна)
	hWnd = CreateWindowExW(0, className, (LPCWSTR)caption, WS_OVERLAPPEDWINDOW, 0, 0, R.right, R.bottom, 0, 0, hInstance, 0);

	RecalculateSize(hWnd);
	// заполняет заданный буфер метриками текущего выбранного шрифта.
	GetTextMetrics(hdcBack,																		// дескриптор DC
		&metrics																				// метрики текста
	);															
	horizontal_Step = metrics.tmHeight;
	// создает логическое перо
	hpBorder = CreatePen(PS_DOT,																// стиль пера 
		1,																						// ширина пера
		0																						// цвет пера
	);

	// выбирает объект в заданный контекст устройства
	SelectObject(hdcBack,																		// дескриптор контекста устройства (DC)
		hpBorder																				// дескриптор объекта
	);

	ShowWindow(hWnd, SW_SHOWNORMAL);															// Показать окно
	UpdateWindow(hWnd);																			// Вызванное обновление для полной прорисовки окна

}

void RecalculateSize(HWND hWnd)
{
	// извлекает координаты рабочей области окна
	GetClientRect(hWnd,																			// дескриптор окна
		&clientRect																				// адрес структуры рабочих координат
	);

	vertical_Step = clientRect.right / M;

	CreateBackground();
	ForceRedraw(hWnd);																			// InvalidateRect (hWnd, 0, 0); prev_Time = 0

}

void CreateBackground()
{

	HDC hdc;																					// Дескриптор контекста устройства

	// удаляет логическое перо, кисть, шрифт, точечную картинку, регион или палитру, освобождая все системные ресурсы, связанные с объектом
	DeleteObject(hbmBack																		// дескриптор графического объекта
	);

	
	hdc = GetDC(0);																				// извлекает дескриптор дисплейного контекста устройства (DC) для рабочей области заданного окна или для всего экрана
	hdcBack = CreateCompatibleDC(hdc);															// создает контекст устройства в памяти  (DC), совместимый с заданным устройством

	// создает точечный рисунок, совместимый с устройством, которое связано с заданным контекстом устройства
	hbmBack = CreateCompatibleBitmap(hdc,														// дескриптор DC
		clientRect.right,																		// ширина рисунка, в пикселях
		clientRect.bottom																		// высота рисунка, в пикселях
	);
	SelectObject(hdcBack, hbmBack);
	// освобождает контекст устройства
	ReleaseDC(0,																				// дескриптор окна
		hdc																						// дескриптор DC
	);

}

void Draw()
{

	textPointer = strings;																		// матрица

	// устанавливает координаты заданного прямоугольника
	SetRect(&r,																					// прямоугольник
		0, 0, vertical_Step, horizontal_Step);

	DrawLines();

}

void DrawLines()
{

	int i;

	for (i = 0; i < N; i++) {
		max_Height = r.bottom;
		DrawRaws();
		r.top = max_Height;
		r.bottom = max_Height + horizontal_Step;
		// обновляет текущую позицию указанной точки и, необязательно, возвращает предыдущую позицию
		MoveToEx(hdcBack,																		// дескриптор контекста устройства
			0,																					// координата x новой текущей позиции
			max_Height,																			// координата y новой текущей позиции
			0																					// старая текущая позиция
		);

		// чертит линию от текущей позиции до, но не включая в нее, указанной точки
		LineTo(hdcBack,																			// дескриптор контекста устройства
			clientRect.right,																	// координата x конечной точки
			max_Height																			// координата y конечной точки
		);
	}

	r.right = vertical_Step;

	for (i = 0; i < M; i++) {
		MoveToEx(hdcBack, r.right, 0, 0);
		LineTo(hdcBack, r.right, max_Height);
		r.right += vertical_Step;
	}

}

void DrawRaws()
{

	int i;

	for (i = 0; i < M; i++) {
		WriteText();
		r.left += vertical_Step;
		r.right += vertical_Step;
	}

	r.left = 0;
	r.right = vertical_Step;

}
//
//функция для отрисовки текста средствами WinApi. Внимание!!! Использует глобальные переменные!!!
void WriteText()
{

	int len, initLen, written;
	const wchar_t *string;

	string = *textPointer;
	textRect = r;
	written = 0;

	if (string) {
		len = GetTextLength(string);
		initLen = len;
		while (1) {
			// вычисляет ширину и высоту заданной строки текста
			GetTextExtentPoint32W(hdcBack,														// дескриптор DC
				string,																			// строка текста
				len,																			// символы в строке
				&size																			// размер строки
			);
			if (size.cx <= vertical_Step) {
				textRect.top++;
				textRect.left++;
				// рисует отформатированный текст в заданном прямоугольнике
				DrawText(hdcBack,																// дескриптор контекста устройства
					string,																		// текст для вывода
					len,																		// длина текста
					&textRect,																	// размеры поля форматирования
					0																			// параметры вывода текста
				);
				textRect.top--;
				textRect.left--;
				written += len;
				if (len != initLen) {
					textRect.top += horizontal_Step;
					textRect.bottom += horizontal_Step;
					string = *textPointer + len;
					len = initLen - len;
					initLen = len;
				}
				else {
					break;
				}
			}
			else {
				do {
					GetTextExtentPoint32W(hdcBack, string, --len, &size);
				} while (size.cx >= vertical_Step);
			}
		}
	}
	UpdateMax(textRect.bottom);																	// if (value > max_Height) max_Height = value
	textPointer++;

}

int GetTextLength(const wchar_t *string)
{

	int i = 0;

	if (string[i]) {
		for (i = 0; string[i]; i++);
	}
	else
	{
		i = 2;
	}
	return (i);

}