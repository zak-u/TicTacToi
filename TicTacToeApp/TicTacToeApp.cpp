// TicTacToeApp.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "TicTacToeApp.h"
#include <windowsx.h>
#include <vector>

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TICTACTOEAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TICTACTOEAPP));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TICTACTOE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_GAME));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 400 , 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}
// Глобальные переменные для игры

const int CELL_SIZE = 100;
HBRUSH hbr1, hbr2;
HICON hIcon1, hIcon2;
int playerTurn = 1;
int gameBoard[9] = { 0,0,0,0,0,0,0,0,0 };
int winner = 0;
int wins[3];



bool GetGameBoardRect(HWND hwnd, RECT* pRect)
{
	RECT rc;
	if (GetClientRect(hwnd, &rc))
	{
		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;

		pRect->left = (width - CELL_SIZE * 3) / 2;
		pRect->top = (height - CELL_SIZE * 3) / 2;

		pRect->right = pRect->left + CELL_SIZE * 3;
		pRect->bottom = pRect->top + CELL_SIZE * 3;

		return true;
	}

	SetRectEmpty(pRect);
	return false;
}

void DrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}

int GetCellNumberFromPoint(HWND hwnd, int x, int y)
{
	RECT rc;
	POINT pt = { x, y };

	if (GetGameBoardRect(hwnd, &rc))
	{
		if (PtInRect(&rc, pt))
		{
			// пользователь нажал кнопку внутри игровой доски
			x = pt.x - rc.left;
			y = pt.y - rc.top;

			int column = x / CELL_SIZE;
			int row = y / CELL_SIZE;

			// преобразование в индекс
			return column + row * 3;
		}
	}
	return -1;		// за пределами игровой доски
}

bool GetCellRect(HWND hWnd, int index, RECT* pRect)
{
	RECT rcBoard;

	SetRectEmpty(pRect);
	if (index < 0 || index > 8)
	{
		return false;
	}

	if (GetGameBoardRect(hWnd, &rcBoard))
	{
		// преобразование индекса от 0 до 8 в пару x, y
		int y = index / 3;		// номер строки
		int x = index % 3;		// номер столба

		pRect->left = rcBoard.left + x * CELL_SIZE + 1;
		pRect->top = rcBoard.top + y * CELL_SIZE + 1;

		pRect->right = pRect->left + CELL_SIZE - 1;
		pRect->bottom = pRect->top + CELL_SIZE - 1;

		return true;
	}

	return false;
}

int GetWinner(int wins[3])
{
	/**
	* Возвращает:
	* 0 - Идет игра
	* 1 - Игрок выйграл
	* 2 - Игрок выйграл
	* 3 - Ничья
	*/

	int cells[] = { 0,1,2  ,  3,4,5  ,  6,7,8  ,  0,3,6  ,  1,4,7  ,  2,5,8  ,  0,4,8  ,  2,4,6 };

	// Проверка победителя
	for (int i = 0; i < ARRAYSIZE(cells); i += 3)
	{
		if ((gameBoard[cells[i]] != 0) && gameBoard[cells[i]] == gameBoard[cells[i + 1]] && gameBoard[cells[i]] == gameBoard[cells[i + 2]])
		{
			wins[0] = cells[i];
			wins[1] = cells[i + 1];
			wins[2] = cells[i + 2];

			return gameBoard[cells[i]];
		}
	}

	for (int i = 0; i < ARRAYSIZE(gameBoard); i++)
	{
		if (gameBoard[i] == 0)
			return 0;				
	}
	return 3;						
}

void  ShowTurn(HWND hwnd, HDC hdc)
{

	static const WCHAR szTurn1[] = L"Ход: Игрок 1";
	static const WCHAR szTurn2[] = L"Ход: Игрок 2";
	const WCHAR* pszTurnText = NULL;

	switch (winner)
	{
	case 0: // continue to play
		pszTurnText = (playerTurn == 1) ? szTurn1 : szTurn2;
		break;
	case 1: // Player 1 wins
		pszTurnText = L"Игрок 1 победил!";
		break;
	case 2: // Player 2 wins
		pszTurnText = L"Игрок 2 победил!";
		break;
	case 3: // tied
		pszTurnText = L"Ничья!!";
		break;
	}
	RECT rc;

	if (pszTurnText != NULL && GetClientRect(hwnd, &rc))
	{
		rc.top = rc.bottom - 48;
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, pszTurnText, lstrlen(pszTurnText), &rc, DT_CENTER);
	}
}

void DrawIconCentered(HDC hdc, RECT* pRect, HICON hIcon)
{
	const int ICON_WIDTH = 32;
	const int ICON_HEIGHT = 32;
	if (pRect != NULL)
	{
		int left = pRect->left + ((pRect->right - pRect->left) - ICON_WIDTH) / 2;
		int top = pRect->top + ((pRect->bottom - pRect->top) - ICON_HEIGHT) / 2;
		DrawIcon(hdc, left, top, hIcon);
	}
}

void ShowWinner(HWND hwnd, HDC hdc)
{
	RECT rcWin;

	for (int i = 0; i < 3; i++)
	{
		if (GetCellRect(hwnd, wins[i], &rcWin))
		{
			FillRect(hdc, &rcWin, (winner == 1) ? hbr1 : hbr2);
			DrawIconCentered(hdc, &rcWin, (winner == 1) ? hIcon1 : hIcon2);
		}
	}
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		hbr1 = CreateSolidBrush(RGB(204, 0, 0));
		hbr2 = CreateSolidBrush(RGB(0, 0, 255));

		hIcon1 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CROSS));
		hIcon2 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CIRCLE));

	}
	break;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		
		switch (wmId)
		{
		case ID_NEWGAME:
		{
			int ret = MessageBox(hWnd, L"Вы уверены, что хотите начать новую игру?", L"Новая игра", MB_YESNO | MB_ICONQUESTION);
			if (IDYES == ret)
			{
				playerTurn = 1;
				winner = 0;
				ZeroMemory(gameBoard, sizeof(gameBoard));

				InvalidateRect(hWnd, NULL, TRUE);	
				UpdateWindow(hWnd);					
			}
		}
		break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;

	case WM_LBUTTONDOWN:
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if (playerTurn == 0)
		{
			break;
		}

		int index = GetCellNumberFromPoint(hWnd, xPos, yPos);
		HDC hdc = GetDC(hWnd);
		if (NULL != hdc)
		{
			if (index != -1)
			{
				RECT rcCell;
				if ((gameBoard[index] == 0) && GetCellRect(hWnd, index, &rcCell))
				{
					gameBoard[index] = playerTurn;

					DrawIconCentered(hdc, &rcCell, (playerTurn == 1) ? hIcon1 : hIcon2);

					// проверка победителя

					winner = GetWinner(wins);
					if (winner == 1 || winner == 2)
					{
						ShowWinner(hWnd, hdc);

						// Есть победитель
						MessageBox(hWnd, (winner == 1) ? L"Игрок 1 Победил!" : L"Игрок 2 Победил!",
							L"Вы выйграли",
							MB_OK | MB_ICONINFORMATION);
						playerTurn = 0;
					}
					else if (winner == 3)
					{
						// Ничья
						MessageBox(hWnd, L"Никто не победил",
							L"Ничья",
							MB_OK | MB_ICONEXCLAMATION);
						playerTurn = 0;
					}
					else if (winner == 0)
					{
						playerTurn = (playerTurn == 1) ? 2 : 1;
					}

					ShowTurn(hWnd, hdc);

				}

			}

			ReleaseDC(hWnd, hdc);
		}
	}
	break;


	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;

		pMinMax->ptMinTrackSize.x = CELL_SIZE * 5;
		pMinMax->ptMinTrackSize.y = CELL_SIZE * 5;

	}
	break;


	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		RECT rc;

		if (GetGameBoardRect(hWnd, &rc))
		{
			RECT rcClient;

			if (GetClientRect(hWnd, &rcClient))
			{
				const WCHAR szPlayer1[] = L"Игрок 1";
				const WCHAR szPlayer2[] = L"Игрок 2";

				SetBkColor(hdc, RGB(128, 128, 128));
				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, RGB(255, 0, 0));
				TextOut(hdc, 16, 16, szPlayer1, ARRAYSIZE(szPlayer1));
				DrawIcon(hdc, 24, 40, hIcon1);

				SetTextColor(hdc, RGB(0, 0, 255));
				TextOut(hdc, rcClient.right - 72, 16, szPlayer2, ARRAYSIZE(szPlayer2));
				DrawIcon(hdc, rcClient.right - 60, 40, hIcon2);


				ShowTurn(hWnd, hdc);
			}



			// Рисуем границы игры
			FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

			for (int i = 1; i < 3; i++)
			{
				// DВертикальные линии
				DrawLine(hdc, rc.left + CELL_SIZE * i, rc.top + 15, rc.left + CELL_SIZE * i, rc.bottom - 15);

				// Горизонтальные линии 
				DrawLine(hdc, rc.left + 15, rc.top + CELL_SIZE * i, rc.right - 15, rc.top + CELL_SIZE * i);

			}
			RECT rcCell;

			for (int i = 0; i < 9; i++)
			{
				if (gameBoard[i] != 0 && GetCellRect(hWnd, i, &rcCell))
				{
					DrawIconCentered(hdc, &rcCell, (gameBoard[i] == 1) ? hIcon1 : hIcon2);
				}
			}

			if (winner == 1 || winner == 2)
			{
				ShowWinner(hWnd, hdc);
			}
		}

		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:
		DeleteObject(hbr1);
		DeleteObject(hbr2);

		DestroyIcon(hIcon1);
		DestroyIcon(hIcon2);

		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
