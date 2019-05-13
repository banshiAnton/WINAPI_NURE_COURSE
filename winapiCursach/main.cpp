#define _CRT_SECURE_NO_WARNINGS

#define ID_PICK_FILE_BUTTON 1337
#define ID_EDIT_BOX 1338

#define ID_BUTTON_COLOR 1339
#define ID_COMBOX 1340


#include <windows.h> // підключення бібліотеки з функціями API
#include <string>
#include <stdio.h>
#include <fstream>
#include <vector>

// Глобальні змінні:
HINSTANCE hInst; 	//Дескриптор програми
HWND hComBox;
LPCTSTR szWindowClass = "CHART";
LPCTSTR szTitle = "CHART";

int currentSec = 0;

bool isSetUp = false;

int minValue = INT_MAX;
int maxValue = INT_MIN;
int SUM = 0;

int buttonWidth = 80;
int buttonHeight = 20;

int paddingTop = 10;
int paddingLeft = 10;

int colMaxHeight = 200;

int colWidth = 22;
int lineBold = 10;

int ellipseD = 250;

COLORREF penColor = RGB(47, 175, 73);
COLORREF ellipseColors[4] = { RGB(66, 134, 244), RGB(211, 65, 244), RGB(77, 219, 41), RGB(219, 94, 41) };

int mainWidth = 0;
int mainHeight = 0;

int COUNTSECTIONS = 8;

struct chartItem
{
	std::string name;
	int val;
	COLORREF color;
};

std::vector<chartItem> chart;

RECT setMainSize(HWND, bool);
void pickFile(HWND);
void readFile(char*, HWND);
void setUp(std::string line);
void draw(HWND, HDC);

RECT setMainSize(HWND hWnd, bool set = true)
{
	RECT rt;
	GetClientRect(hWnd, &rt);

	if (set)
	{
		mainHeight = rt.bottom - rt.top;
		mainWidth = rt.right - rt.left;
	}

	return rt;
}

void pickFile(HWND hWnd)
{
	OPENFILENAME file;

	char fileName[100];

	ZeroMemory(&file, sizeof(OPENFILENAME));

	file.lStructSize = sizeof(OPENFILENAME);
	file.hwndOwner = hWnd;
	file.lpstrFile = fileName;
	file.lpstrFile[0] = '\0'; // no default file
	file.nMaxFile = 2000;
	file.lpstrFilter = "Charts Data File\0*.TXT\0";

	if ( !GetOpenFileName(&file) ) 
	{
		std::string error(file.lpstrFile);
		error = "Error Open File " + error;
		MessageBox(hWnd, error.c_str(), "Error", MB_OK);
	}

	readFile(file.lpstrFile, hWnd);
}

void readFile(char* filePath, HWND hWnd)
{
	std::string line;

	std::ifstream file(filePath);

	if ( file.is_open() )
	{
		getline(file, line); // get first line

		if (line != "!CHARTS") {
			MessageBox(NULL, "File invalid (Should first line must be \"!CHARTS\")", "Error", MB_OK);
			file.close();
		}

		chart.clear(); //clear vector
	
		while ( !file.eof() )
		{
			getline(file, line);
			setUp(line);
		}

		file.close();

		isSetUp = true;
		SendMessage(hComBox, (UINT)CB_SETCURSEL, currentSec, 0);
		InvalidateRect(hWnd, NULL, TRUE);
	}
}

void setUp(std::string line)
{
	chartItem item;

	std::string sep = " ";

	size_t pos = line.find(sep);

	item.name = line.substr(0, pos);
	item.val = std::stoi( line.substr( pos + 1, line.size() ) );

	SendMessage(hComBox, CB_ADDSTRING, 0, (LPARAM)item.name.c_str());

	if (item.val > maxValue) maxValue = item.val;
	if (item.val < minValue) minValue = item.val;
	SUM += item.val;

	item.color = RGB(196, 43, 147);

	chart.push_back( item );

	//std::string res = line.substr(0, pos) + line.substr(pos + 1, line.size()) + std::to_string(minValue);

	//SetWindowText(hEdit, res.c_str());
}

void draw(HWND hWnd, HDC hdc)
{
	/******************** Draw first chart ******************/

	if (!isSetUp) return;

	int leftGap = paddingLeft * 2;
	int maxLengthChart = mainWidth - leftGap - paddingLeft;
	int gap = paddingLeft * 2;

	int startTop = paddingTop * 2 + buttonHeight;
	int startTopGrid = startTop - 10;

	for (chartItem item : chart)
	{
		HBRUSH hBrush = CreateSolidBrush(item.color);

		SelectObject(hdc, hBrush);

		int length = (item.val * maxLengthChart) / maxValue;

		Rectangle(hdc, leftGap, startTop, leftGap + length, startTop + colWidth);

		startTop += gap + colWidth;
	}

	/***********************************************************/




	/********* Draw grid for first chart ******************/

	HPEN hPenGrid = CreatePen(NULL, 1, RGB(0,0,0));

	SelectObject(hdc, hPenGrid);

	int gridGap = maxLengthChart / COUNTSECTIONS;
	int valStep = maxValue / COUNTSECTIONS;
	int labelValue = 0;

	for (int i = 0; i <= COUNTSECTIONS; i++)
	{
		MoveToEx(hdc, leftGap, startTopGrid, NULL);
		LineTo(hdc, leftGap, startTop);

		char label[5];
		TextOut(hdc, leftGap - 6, startTop + 10, label, wsprintf(label, "%d", labelValue));

		labelValue += valStep;
		leftGap += gridGap;
	}

	/***********************************************************/





	/*********Draw bottom line for grid for first chart ******************/

	MoveToEx(hdc, paddingLeft * 2, startTop, NULL);
	LineTo(hdc, paddingLeft * 2 + maxLengthChart, startTop);

	/***********************************************************/




	/********* Draw second chart(line) ******************/

	startTop += 45;

	leftGap = paddingLeft * 2;

	HPEN hPen = CreatePen(NULL, lineBold, chart[0].color);

	SelectObject(hdc, hPen);

	int firstDot = (chart[0].val * colMaxHeight) / maxValue;
	MoveToEx(hdc, leftGap, startTop + (colMaxHeight - firstDot), 0);

	int lineGap = maxLengthChart / (chart.size() - 1);

	leftGap += lineGap;

	for(int i = 1; i < chart.size(); i++)
	{
		hPen = CreatePen(NULL, lineBold, chart[i].color);

		SelectObject(hdc, hPen);

		int height = (chart[i].val * colMaxHeight) / maxValue;

		LineTo(hdc, leftGap, startTop + (colMaxHeight - height));

		leftGap += lineGap;
	}

	/***********************************************************/





	/********* Draw grid for secod chart(line) ******************/

	int startLinesGrid = startTop;

	COUNTSECTIONS -= COUNTSECTIONS / 2;

	SelectObject(hdc, hPenGrid); 

	int valLineSte = (maxValue - minValue) / COUNTSECTIONS;
	int labeLineValue = maxValue;

	int topStep = ( ( ( (maxValue - minValue) * colMaxHeight) / maxValue) / COUNTSECTIONS);

	leftGap = paddingLeft * 2;

	for (int i = 0; i <= COUNTSECTIONS; i++)
	{
		MoveToEx(hdc, leftGap, startLinesGrid, NULL);
		LineTo(hdc, leftGap + maxLengthChart, startLinesGrid);

		char label[5];
		TextOut(hdc, leftGap - 6, startLinesGrid + 5, label, wsprintf(label, "%d", labeLineValue));

		startLinesGrid += topStep;
		labeLineValue -= valLineSte;
	}

	/***********************************************************/





	/********* Draw third chart(ellipse) ******************/

	HBRUSH hBrushEllipse = CreateSolidBrush(RGB(219, 41, 219));

	SelectObject(hdc, hBrushEllipse);

	HPEN hPenEllipse = CreatePen(NULL, 0, NULL);

	SelectObject(hdc, hPenEllipse);

	startTop += colMaxHeight - 20;

	Ellipse(hdc, paddingLeft, startTop, paddingLeft + ellipseD, startTop + ellipseD);

	int dwRadius = ellipseD / 2;

	int nX = paddingLeft + dwRadius;
	int nY = startTop + dwRadius;

	int xStartAngle = 0;

	int sumAngle = 0;

	for (int i = 0; i < chart.size(); i++)
	{

		SetDCBrushColor(hdc, chart[i].color);

		int xSweepAngle = (360 * chart[i].val) / SUM;
		sumAngle += xSweepAngle;

		if ((i == (chart.size() - 1)) && sumAngle < 360)
		{
			xSweepAngle += 360 - sumAngle;
		}

		BeginPath(hdc);
		SelectObject(hdc, GetStockObject(DC_BRUSH));
		MoveToEx(hdc, nX, nY, (LPPOINT)NULL);
		AngleArc(hdc, nX, nY, dwRadius, xStartAngle, xSweepAngle);
		LineTo(hdc, nX, nY);
		EndPath(hdc);
		StrokeAndFillPath(hdc);

		xStartAngle += xSweepAngle;
	}

	/***********************************************************/

}

// Попередній опис функцій

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// Основна програма 
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
	int nCmdShow)
{
	MSG msg;

	// Реєстрація класу вікна 
	MyRegisterClass(hInstance);

	// Створення вікна програми
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	// Цикл обробки повідомлень
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW; 		//стиль вікна
	wcex.lpfnWndProc = (WNDPROC)WndProc; 		//віконна процедура
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; 			//дескриптор програми
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION); 		//визначення іконки
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW); 	//визначення курсору
	wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW + 1); //установка фону
	wcex.lpszMenuName = NULL; 				//визначення меню
	wcex.lpszClassName = szWindowClass; 		//ім’я класу
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex); 			//реєстрація класу вікна
}

// FUNCTION: InitInstance (HANDLE, int)
// Створює вікно програми і зберігає дескриптор програми в змінній hInst

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	hInst = hInstance; //зберігає дескриптор додатка в змінній hInst
	hWnd = CreateWindow(szWindowClass, 	// ім’я класу вікна
		szTitle, 				// назва програми
		WS_OVERLAPPEDWINDOW,			// стиль вікна
		CW_USEDEFAULT, 			// положення по Х	
		CW_USEDEFAULT,			// положення по Y	
		CW_USEDEFAULT, 			// розмір по Х
		CW_USEDEFAULT, 			// розмір по Y
		NULL, 					// дескриптор батьківського вікна	
		NULL, 					// дескриптор меню вікна
		hInstance, 				// дескриптор програми
		NULL); 				// параметри створення.

	if (!hWnd) 	//Якщо вікно не творилось, функція повертає FALSE
	{
		return FALSE;
	}

	setMainSize(hWnd);

	CreateWindow("button", "Pick File", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 10, buttonWidth, buttonHeight, hWnd, (HMENU)ID_PICK_FILE_BUTTON, NULL, NULL);
	hComBox = CreateWindow("combobox", "PickColor", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, mainWidth - 350, mainHeight - 200, buttonWidth + 70, buttonHeight + 200, hWnd, (HMENU)ID_COMBOX, NULL, NULL);
	CreateWindow("button", "PickColorButton", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, mainWidth - 150, mainHeight - 50, buttonWidth + 70, buttonHeight + 20, hWnd, (HMENU)ID_BUTTON_COLOR, NULL, NULL);
	
	//hEdit = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_BORDER | ES_AUTOHSCROLL, 10, 50, 400, 300, hWnd, (HMENU)ID_EDIT_BOX, NULL, NULL);

	ShowWindow(hWnd, nCmdShow); 		//Показати вікно
	UpdateWindow(hWnd); 				//Оновити вікно
	return TRUE;
}

// FUNCTION: WndProc (HWND, unsigned, WORD, LONG)
// Віконна процедура. Приймає і обробляє всі повідомлення, що приходять в додаток

void pickColor(HWND hWnd)
{
	static DWORD rgbCurrent;
	COLORREF acrCustClr[16];

	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hWnd;
	cc.lpCustColors = (LPDWORD)acrCustClr;
	cc.rgbResult = rgbCurrent;

	if (ChooseColor(&cc))
	{
		chart[currentSec].color = cc.rgbResult;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rt;

	switch (message)
	{
	case WM_CREATE: 				//Повідомлення приходить при створенні вікна
		break;

	case WM_PAINT: 				//Перемалювати вікно
		hdc = BeginPaint(hWnd, &ps); 	//Почати графічний вивід	
		draw(hWnd, hdc);
		EndPaint(hWnd, &ps); 		//Закінчити графічний вивід	
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_PICK_FILE_BUTTON:
			//MessageBox(hWnd, "Pick File", "File", MB_OK);
			pickFile(hWnd);
			break;
		case ID_BUTTON_COLOR:

			currentSec = SendMessage(hComBox, (UINT)CB_GETCURSEL, 0, 0);

			pickColor(hWnd);

			InvalidateRect(hWnd, NULL, TRUE);

			break;
		default:
			break;
		}

		break;
	case WM_DESTROY: 				//Завершення роботи
		PostQuitMessage(0);
		break;
	default:
		//Обробка повідомлень, які не оброблені користувачем
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
