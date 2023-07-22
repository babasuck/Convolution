#include "window.h"
#include "Control.h"
#include "Convolution.h"
#include <assert.h>
#include "resource.h"
#pragma comment(lib, "Window SDK.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#define H 500 + 30
#define W 500 + 15

// Имена файлов критериев
#define BUFSIZE 255
struct StaticWindow* critStr;
struct StaticWindow* markStr;

void initControls(Window* wnd);
void setControls(Window* wnd);
void calculateOnClick(struct Window* window, struct Button* button);

void critButtonOnClick(struct Window* window, struct Button* button);
void markButtonOnClick(struct Window* window, struct Button* button);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	AllocConsole();
	Window* wnd = window_create_object();
	window_ctor(wnd, hInstance, L"Convolution", W, H);
	     SetMenu(window_getHWND(wnd), LoadMenu(hInstance, IDR_MENU));
	initControls(wnd);
	setControls(wnd);
	window_start_rendering(wnd);
	window_start_message_handling(wnd);
}

// Global controls 
// Кнопка расчитать
struct Button* calcButton;
// Кол-во переменных решения
struct TextEdit* nxEdit;
// Кол-во ограничений
struct TextEdit* naEdit;
// Загрузить критерии
struct Button* critButton;
// Загрузить оценки
struct Button* markButton;

void initControls(Window* wnd) {
	calcButton = Button_create_object();
	critButton = Button_create_object();
	markButton = Button_create_object();
	nxEdit = TextEdit_create_object();
	naEdit = TextEdit_create_object();
	critStr = StaticWindow_create_object();
	markStr = StaticWindow_create_object();
}

void setControls(Window* wnd) {
	// Кол-во переменных решения
	struct StaticWindow* nxText = StaticWindow_create_object();
	const wchar_t* nxT = L"Кол-во переменных решения";
	StaticWindow_ctor(nxText, wnd, nxT, 0, 39, 104, wcslen(nxT) * 8 + 5, 15);
	StaticWindow_set_isTransparent(nxText, 1);
	StaticWindow_set_TextColor(nxText, RGB(255, 255, 255));
	TextEdit_ctor(nxEdit, wnd, 0, 0, 39 + (wcslen(nxT) * 8 + 5), 104, 80, 17);
	SetWindowLong(Control_get_HWND(nxEdit), GWL_STYLE, GetWindowLong(Control_get_HWND(nxEdit), GWL_STYLE) + ES_NUMBER);

	// Кол-во ограничений
	struct StaticWindow* naText = StaticWindow_create_object();
	const wchar_t* naT = L"Кол-во ограничений";
	StaticWindow_ctor(naText, wnd, naT, 0, 39, 137, wcslen(naT) * 8 + 5, 15);
	StaticWindow_set_isTransparent(naText, 1);
	StaticWindow_set_TextColor(naText, RGB(255, 255, 255));
	TextEdit_ctor(naEdit, wnd, 0, 0, 39 + (wcslen(naT) * 8 + 5), 135, 80, 17);
	SetWindowLong(Control_get_HWND(naEdit), GWL_STYLE, GetWindowLong(Control_get_HWND(naEdit), GWL_STYLE) + ES_NUMBER);

	struct StaticWindow* kvaziText = StaticWindow_create_object();
	const wchar_t* kvaziT = L"Квазирешение";
	StaticWindow_ctor(kvaziText, wnd, kvaziT, 0, 39, 170, wcslen(kvaziT) * 8 + 5, 15);
	StaticWindow_set_isTransparent(kvaziText, 1);
	StaticWindow_set_TextColor(kvaziText, RGB(255, 255, 255));


	struct StaticWindow* convText = StaticWindow_create_object();
	const wchar_t* convT = L"Вид свертки";
	StaticWindow_ctor(convText, wnd, convT, 0, 39, 203, wcslen(convT) * 8, 15);
	StaticWindow_set_isTransparent(convText, 1);
	StaticWindow_set_TextColor(convText, RGB(255, 255, 255));


	struct StaticWindow* effText = StaticWindow_create_object();
	const wchar_t* effT = L"Частные критери эффективности:";
	StaticWindow_ctor(effText, wnd, effT, 0, 39, 241,
		wcslen(effT) * 8 + 5, 15);
	StaticWindow_set_isTransparent(effText, 1);
	StaticWindow_set_TextColor(effText, RGB(255, 255, 255));


	struct StaticWindow* matText = StaticWindow_create_object();
	const wchar_t* matT = L"Матрица парных сравнений:";
	StaticWindow_ctor(matText, wnd, matT, 0, 39, 273, wcslen(matT) * 8 + 5, 15);
	StaticWindow_set_isTransparent(matText, 1);
	StaticWindow_set_TextColor(matText, RGB(255, 255, 255));

	// Расчитать
	Button_ctor(calcButton, wnd, L"Расчитать", 0, 194, 400, 120, 30);
	Button_set_onClick(calcButton, &calculateOnClick);

	// Загрузить критерии
	Button_ctor(critButton, wnd, L"Загрузить", 0, 30 + (wcslen(effT) * 8 + 5), 241, 100, 20);
	Button_set_onClick(critButton, &critButtonOnClick);

	// Загрузить оценки
	Button_ctor(markButton, wnd, L"Загрузить", 0, 30 + (wcslen(matT) * 8 + 5), 273, 100, 20);
	Button_set_onClick(markButton, &markButtonOnClick);
}

void calculateOnClick(struct Window* window, struct Button* button) {
	if (!IsWindow(Control_get_HWND(critStr)) || !IsWindow(Control_get_HWND(markStr))) {
		return;
	}
	WCHAR buf[BUFSIZE];
	int n = 0, m = 0;
	TextEdit_getText(nxEdit, &buf);
	n = _wtoi(buf);
	TextEdit_getText(naEdit, &buf);
	m = _wtoi(buf);
	// Check if n or m is zero
	if (n == 0 || m == 0) {
		return;
	}
	// Коэффициенты при перменных 
	double* x = (double*)calloc(n, sizeof(*x));
	assert(x);
	for (int i = 0; i < n; i++) {
		// TODO заполнение вектора
	}
	// Allocate constrains matrix A
	double** A = (double**)calloc(m, sizeof(*A));
	assert(A);
	for (int i = 0; i < m; i++) {
		A[i] = (double*)calloc(n, sizeof(**A));
		// TODO заполнение матрицы
	}
	// Свободные коэффициенты ограничений
	double* b = (double*)calloc(m, sizeof(*b));
	assert(b);
	for (int i = 0; i < m; i++) {
		// TODO заполнение коэффициентов

	}
	lpSolve(n, m, x, A, b, MAXIM);
	// Free resources
	free(b);
	free(x);
	for (int i = 0; i < m; i++) {
		free(A[i]);
	}
	free(A);
}

void critButtonOnClick(struct Window* window, struct Button* button) {
	if (IsWindow(Control_get_HWND(critStr))) {
		DestroyWindow(Control_get_HWND(critStr));
	}
	OPENFILENAME ofs = {0};
	wchar_t buf[BUFSIZE] = {0};
	const wchar_t* filter = L"Text Files .txt\0*.txt\0\0";
	ofs.lStructSize = sizeof(OPENFILENAME);
	ofs.lpstrFile = &buf;
	ofs.nMaxFile = BUFSIZE;
	ofs.lpstrFilter = filter;
	ofs.nFilterIndex = 1;
	if (GetOpenFileName(&ofs)) {
		wchar_t critT[BUFSIZE];
		wcscpy_s(critT, BUFSIZE, ofs.lpstrFile);
		StaticWindow_ctor(critStr, window, critT, 0, 39, 311,
			wcslen(critT) * 8 + 5, 15);
		StaticWindow_set_isTransparent(critStr, 1);
		StaticWindow_set_TextColor(critStr, RGB(255, 0, 0));
	}
}

void markButtonOnClick(struct Window* window, struct Button* button) {
	if (IsWindow(Control_get_HWND(markStr))) {
		DestroyWindow(Control_get_HWND(markStr));
	}
	OPENFILENAME ofs = { 0 };
	wchar_t buf[BUFSIZE] = { 0 };
	const wchar_t* filter = L"Text Files .txt\0*.txt\0\0";
	ofs.lStructSize = sizeof(OPENFILENAME);
	ofs.lpstrFile = &buf;
	ofs.nMaxFile = BUFSIZE;
	ofs.lpstrFilter = filter;
	ofs.nFilterIndex = 1;
	if (GetOpenFileName(&ofs)) {
		wchar_t markT[BUFSIZE];
		wcscpy_s(markT, BUFSIZE, ofs.lpstrFile);
		StaticWindow_ctor(markStr, window, markT, 0, 39, 349,
			wcslen(markT) * 8 + 5, 15);
		StaticWindow_set_isTransparent(markStr, 1);
		StaticWindow_set_TextColor(markStr, RGB(255, 0, 0));
	}
}