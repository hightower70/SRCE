/*****************************************************************************/
/* Win32 Graphics Display Emulator                                           */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stdint.h>
#include <tchar.h>
#include <Windows.h>
#include "sysConfig.h"
#include <sysTypes.h>
#include <sysUserInput.h>
#include <sysVirtualKeyboardCodes.h>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static HWND		l_hwnd					= 0;

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
HDC g_emulation_hdc;

/*****************************************************************************/
/* External functions                                                        */
/*****************************************************************************/
extern void drvUserInputEventHandler(uint8_t in_device_number, sysUserInputEventCategory in_event_category, sysUserInputEventType in_event_type, uint32_t in_event_param);
extern void drvGraphicsRefreshScreen(HDC in_hdc);

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
static HWND CreateEmuWindow(void);


/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes emulator window
void drvGraphicsDisplayInitialize(void)
{
	CreateEmuWindow();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief refreshes emulator screen
void drvColorGraphicsRefreshScreen(void)
{
	if( l_hwnd != NULL )
	{
		g_emulation_hdc = GetDC( l_hwnd );

		if( g_emulation_hdc != NULL )
			drvGraphicsRefreshScreen(g_emulation_hdc);

		ReleaseDC( l_hwnd, g_emulation_hdc );
	}
}


/*****************************************************************************/
/* Local Function implementation                                             */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Window message handler
static LRESULT CALLBACK WindowMessageHandler(	HWND		in_hwnd,
																							UINT		in_message,
																							WPARAM	in_wparam,
																							LPARAM	in_lparam )
{
  switch (in_message)
  {
		// window create
		case WM_CREATE:
		{
			// set size
			RECT work_area;
			RECT owner_rect;
			HWND owner_hwnd;
			RECT client_rect;
			RECT window_rect;
			int width, height;
			int left, top;

			// set window size
			GetClientRect(in_hwnd, &client_rect);
			GetWindowRect(in_hwnd, &window_rect );

			width = window_rect.right - window_rect.left;
			height = window_rect.bottom - window_rect.top;

			width		= guiSCREEN_WIDTH * guiemuZOOM + width - client_rect.right;
			height	= guiSCREEN_HEIGHT * guiemuZOOM + height - client_rect.bottom;

			owner_hwnd = GetDesktopWindow();

			GetWindowRect( owner_hwnd, &owner_rect );

			left = ( owner_rect.left + owner_rect.right	- width		) / 2;
			top = ( owner_rect.top  + owner_rect.bottom	- height	) / 2;

			// center within screen coordinates
			SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);

			if( left < work_area.left )
				left = work_area.left;

			if( top < work_area.top )
				top = work_area.top;

			if( left + width > work_area.right )
				left = work_area.right - width;

			if( top + height> work_area.bottom )
				top = work_area.bottom - height;

			SetWindowPos( in_hwnd,
										NULL,
										left,
										top,
										width, height,
										SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;

		// paint event
    case WM_PAINT:
			{
				PAINTSTRUCT ps;

				g_emulation_hdc = BeginPaint(in_hwnd, &ps);

				drvGraphicsRefreshScreen(g_emulation_hdc);

				EndPaint(in_hwnd, &ps);
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
      break;

		case WM_SYSKEYDOWN:
			drvUserInputEventHandler(1, (((HIWORD(in_lparam) & KF_REPEAT) == 0) ? sysUIEC_Pressed : sysUIEC_Repeated), sysUIET_Key, ((uint32_t)(((HIWORD(in_lparam) & KF_EXTENDED) == 0) ? 0 : sysVKC_SPECIAL_KEY_FLAG) | in_wparam));
			break;

		case WM_SYSKEYUP:
			drvUserInputEventHandler(1, sysUIEC_Released, sysUIET_Key, (uint32_t)((((in_lparam & (1 << 24)) != 0) ? sysVKC_SPECIAL_KEY_FLAG : 0) | in_wparam));
			break;

		case WM_KEYDOWN:
			drvUserInputEventHandler(1, (((HIWORD(in_lparam) & KF_REPEAT) == 0)? sysUIEC_Pressed : sysUIEC_Repeated), sysUIET_Key, (uint32_t)((((HIWORD(in_lparam) & KF_EXTENDED) == 0) ? 0 : sysVKC_SPECIAL_KEY_FLAG) | in_wparam));
			break;

		case WM_KEYUP:
			drvUserInputEventHandler(1, sysUIEC_Released, sysUIET_Key, (uint32_t)((((in_lparam & (1 << 24)) != 0) ? sysVKC_SPECIAL_KEY_FLAG : 0) | in_wparam));
			break;

		default:
      return DefWindowProc(in_hwnd, in_message, in_wparam, in_lparam);
      break;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Creates emulator window
static HWND CreateEmuWindow(void)
{
	// register class
	WNDCLASSEX wcex;

	wcex.cbSize					= sizeof(WNDCLASSEX);
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WindowMessageHandler;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = 0;
	wcex.hIcon          = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = _T("guiEmulationWindow");
	wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

  if(!RegisterClassEx(&wcex))
  {
		MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("dosEmuWin"), 0);

    return 0;
  }

	// create window
	l_hwnd = CreateWindow(	_T("guiEmulationWindow"),
											_T("Emulation Window"),
											WS_OVERLAPPEDWINDOW | WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX,
											CW_USEDEFAULT, CW_USEDEFAULT,
											0, 0,
											NULL,
											NULL,
											0,
											NULL );
	if( !l_hwnd )
	{
		MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("GUI Emulation"), 0);

    return 0;
	}

	if( l_hwnd != 0 )
	{
		ShowWindow(l_hwnd, SW_SHOWNORMAL);
		UpdateWindow(l_hwnd);
	}

	return l_hwnd;
}

