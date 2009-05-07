#define _WIN32_IE 0x0600

#include "resource.h"

#define _WINSOCKAPI_

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <time.h>

#define FNPICON_MUTEX "THIS IS THE FREENOTE PROBE MUTEX NAME!  IT IS SUPPOSED TO BE GLOBALLY UNIQUE!\1\3\3\7"

	HINSTANCE hinst;
HWND create_window(HINSTANCE inst, ATOM a);
LRESULT CALLBACK callback(HWND h, UINT m, WPARAM w, LPARAM l);

#define DEBOOG() printf("line %d\n", __LINE__); fflush(NULL)

int wtf(HINSTANCE hi);
int stfu(HINSTANCE hi, int i);

//GLOBAL FUCKING VARIABLES
	HWND window;
	HINSTANCE instance;

BOOL running(char *name)
{
	static HANDLE hMutex = NULL;
	
	if( hMutex == NULL ) {
		hMutex = CreateMutex( NULL, FALSE , name);
		return (	GetLastError() == ERROR_ALREADY_EXISTS || 
				GetLastError() == ERROR_ACCESS_DENIED	);
	}
	return FALSE;
}

void real_bad_error(char *s)
{
	MessageBox(0, s, "FreeNote Probe:  Error!", MB_ICONHAND);
	exit(-1);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	   LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASSEX wc;
	NOTIFYICONDATA icondata;
	int x;
	instance = hInstance;

	if(running(FNPICON_MUTEX))
		real_bad_error("The FreeNote Probe Systray Monitor\nis already running!\0MULTIMEDIA APPLICATIONS!~!!!!!~!#!#ONE!!!");

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = callback;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_FREENOTE));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_SCROLLBAR + 1);
	wc.lpszClassName = "proboclass";
	wc.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_FREENOTE));
	if (!RegisterClassEx(&wc)) 
		real_bad_error("Window class registration failed.");

/*
	window = CreateWindow( 
        "proboclass",        // name of window class 
        "Sample",            // title-bar string 
        WS_OVERLAPPEDWINDOW, // top-level window 
        CW_USEDEFAULT,       // default horizontal position 
        CW_USEDEFAULT,       // default vertical position 
        CW_USEDEFAULT,       // default width 
        CW_USEDEFAULT,       // default height 
        (HWND) NULL,         // no owner window 
        (HMENU) NULL,        // use class menu 
        wc.hInstance,        // handle to application instance 
        (LPVOID) NULL);      // no window-creation data 
*/
	window = CreateDialog(wc.hInstance, MAKEINTRESOURCE(IDI_OT),
				GetDesktopWindow(), callback);
	if (window == NULL)
		real_bad_error("Couldn't create a window!");

	icondata.cbSize = sizeof(NOTIFYICONDATA);
	icondata.hWnd = window;
	icondata.uID = 100;
	icondata.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO;
	icondata.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FREENOTE));
	icondata.uCallbackMessage = 0xb00;
	icondata.dwInfoFlags = NIIF_INFO;
	sprintf(icondata.szTip, "FreeNote Probe (running)\0"
		"YOU FOUND HIDDEN BONUS TEXT!!");
	sprintf(icondata.szInfoTitle, 
		"FreeNote Probe\0Heh, looking at the binary?");
	sprintf(icondata.szInfo, "FreeNote Probe started.\n"
				 "Click here to run the probe monitor.\0"
		"FUCK MICROSOFT!! WIN32 API IS SOOO STUPID!!");

	Shell_NotifyIcon(NIM_DELETE, &icondata); //Just in case.
	if(!Shell_NotifyIcon(NIM_ADD, &icondata))  {
		real_bad_error("Couldn't add icon to the systray!");
	}
	else 
		MessageBox(0,	"You are invited to play with the icon.\n"
				"Click 'OK' when you're done.\n"
				"When that gets old, run strings on the binary."
				,
			"Waiting for Edward", 0);
		
	Shell_NotifyIcon(NIM_DELETE, &icondata);
	DestroyIcon(icondata.hIcon);

	real_bad_error("Pete hasn't taken out the debug messages yet!");
	return -1;  //because fuck you.
}

int icon_activity(int i, int m)
{

	char g[2048];
	switch(m) {
		case 1026:		
			printf("Icon created (wow).\n");
			fflush(NULL);
			return 0;	//iono
		case 1027:
			printf("Icon destroyed before balloon went away!\n");
			fflush(NULL);
			return 0;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_WININICHANGE:
		case 1028:
			return 0;	//IGNORED!!

		case WM_DEVICECHANGE:
			return 0x424d5144; //FUCKTARDS

		case 1029:		//iono
		case WM_LBUTTONDBLCLK:	//515
			ShowWindow(window, SW_SHOWNORMAL);
			sprintf(g, "Pretend that what you just did ran the\n"
				   "probe monitor.");
			MessageBox(0, g, "Stubbed...", 0);
			return 0;
		case WM_RBUTTONUP:	//517
				{
				HMENU menu;
				POINT p;
				GetCursorPos(&p);
				menu = LoadMenu(instance, MAKEINTRESOURCE
					     (188));
				if (!menu) 
					real_bad_error("NO MENU!!");
				TrackPopupMenu(GetSubMenu (menu, 0),
						TPM_RIGHTBUTTON |
						TPM_LEFTBUTTON, p.x, p.y, 0, 
						window, NULL);
				DestroyMenu(menu);
				}
			return 0;

		case WM_MOUSEMOVE:	//512
			return 0;	//WE DON'T FUCKING CARE!!
			
		default:
			printf("identifier:  %d, message:  %d\n", i, m);
			fflush(NULL);
			sprintf(g, "THIS IS BROKEN: message %d.\n(Please let me know what caused that\nand the message number.)\0FUCK MICROSOFT", m);
			MessageBox(0, g, 0, 0);
	}
	return -1;
}

LRESULT CALLBACK callback(HWND h, UINT m, WPARAM w, LPARAM l)
{
	char g[2048];
	switch(m) {
		case 0xb00:
			return icon_activity(w, l);
		case WM_NCCREATE:		//129
			return TRUE;

		case WM_NCCALCSIZE:		//131
		case WM_GETMINMAXINFO:
		case WM_SETTINGCHANGE:
			return 0;

		case WM_CREATE:			//1
			return 0;

		case WM_ACTIVATEAPP:		//28
			return -1;
			break;
		case WM_DEVICECHANGE:
			return 0x42d5144; //FUCKTARDS

		case WM_INITDIALOG:
			return 0;		//we don't want focus.

		case WM_ENTERMENULOOP:		//529
		case 48:			//iono
			return 0;

		case WM_QUERYOPEN:
			return 1;


		default:
			sprintf(g, "wakaranai: message %d.\0FUCK MICROSOFT", m);
			real_bad_error(g);
	}
}
