/*****************************************************************************/
/* Windows main function using graphics window to emulate display            */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <windows.h>
#include <sysMain.h>

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Main entrance function for windows
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	MSG msg;
	BOOL cont;

	sysInitialization();

	// Main message loop:
	cont = TRUE;
	do
	{ 
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			cont = GetMessage(&msg, NULL, 0, 0);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		sysMainTask();
		Sleep(0);
	} while (cont);

	drvColorGraphicsCleanup();
	sysCleanup();

	return 0;
}
