static DWORD	l_thread_id = 0;
static HANDLE	l_thread_handle = NULL;

///////////////////////////////////////////////////////////////////////////////
// Init emulator window
void guiInitializeGraphicsDisplayEmulation(void)
{
	guiGraphicsInitialize();

	l_thread_handle = CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)ThreadRoutine,
		NULL,
		0,
		&l_thread_id);
}

void main(void)
{

	l_thread_handle = CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)ThreadRoutine,
		NULL,
		0,
		&l_thread_id);

	sysInitialization();

	while( IsEmulatorRunning() )
	{
		sysMainTask();
		Sleep(0);
	}

	drvGraphicsCleanUp();
	sysCleanup();
}

///////////////////////////////////////////////////////////////////////////////
// Thread functions
static void ThreadRoutine(void* in_param)
{
	MSG msg;

	l_hwnd = CreateEmuWindow();

	if (l_hwnd != 0)
	{
		// Main message loop:
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			Sleep(0);
		}
	}

	if (l_hwnd != 0)
		DestroyWindow(l_hwnd);

	l_thread_id = 0;
	l_thread_handle = NULL;

	ExitThread(0);
}

///////////////////////////////////////////////////////////////////////////////
// Is emulator running
BOOL IsEmulatorRunning(void)
{
	Sleep(0);

	return l_thread_handle != NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Resume thread
void guiEnableEmulatorThread(void)
{
	if (l_thread_handle != NULL)
		ResumeThread(l_thread_handle);
}

///////////////////////////////////////////////////////////////////////////////
// Postpone emulator thread
void guiDisableEmulatorThread(void)
{
	if (l_thread_handle != NULL)
		SuspendThread(l_thread_handle);
}
