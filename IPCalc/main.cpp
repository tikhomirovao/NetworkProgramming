#define _CRT_SECURE_NO_WARNINGS
#include<Windows.h>
#include<CommCtrl.h>
#include<cstdio>	//sprintf
#include<iostream>
#include"resource.h"

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID SetIPPrefix(HWND hwnd);
VOID PrintInfo(HWND hwnd);
CHAR* IPaddressToString(DWORD dwIPaddress, CHAR sz_IPaddress[]);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc, 0);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		HWND hSpin = GetDlgItem(hwnd, IDC_SPIN_PREFIX);
		SendMessage(hSpin, UDM_SETRANGE, 0, MAKELPARAM(30, 1));
		SendMessage(hSpin, UDM_SETPOS, 0, 1);

		AllocConsole();
		freopen("CONOUT$", "w", stdout);
	}
	break;
	case WM_COMMAND:
	{
		HWND hIPaddress = GetDlgItem(hwnd, IDC_IPADDRESS);
		HWND hIPmask = GetDlgItem(hwnd, IDC_IPMASK);
		HWND hIPprefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);
		DWORD dwIPaddress = 0;
		DWORD dwIPmask = 0;
		switch (LOWORD(wParam))
		{
		case IDC_IPADDRESS:
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hIPaddress, IPM_GETADDRESS, 0, (LPARAM)&dwIPaddress);
				if (FIRST_IPADDRESS(dwIPaddress) < 128)		SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFF000000);
				else if (FIRST_IPADDRESS(dwIPaddress) < 192)	SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFFFF0000);
				else if (FIRST_IPADDRESS(dwIPaddress) < 224)	SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFFFFFF00);
				SetIPPrefix(hwnd);
				//PrintInfo(hwnd);
			}
		}
		break;
		/*case IDC_IPMASK:
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hIPmask, IPM_GETADDRESS, 0, (LPARAM)&dwIPmask);
				DWORD count = 0;
				for (DWORD i = dwIPmask; 0x80000000 & i; i <<= 1, count++);
				CHAR szIPprefix[3] = "";
				sprintf(szIPprefix, "%i", count);
				SendMessage(hIPprefix, WM_SETTEXT, 0, (LPARAM)szIPprefix);
			}
		}
		break;*/
		case IDC_EDIT_PREFIX:
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				DWORD dwIPmask = UINT_MAX;
				CHAR szIPprefix[3];
				SendMessage(hIPprefix, WM_GETTEXT, 3, (LPARAM)szIPprefix);
				DWORD dwIPprefix = atoi(szIPprefix);
				dwIPmask <<= (32 - dwIPprefix);
				SendMessage(hIPmask, IPM_SETADDRESS, 0, dwIPmask);
			}
		}
		break;
		case IDOK:
			break;
		case IDCANCEL:
			EndDialog(hwnd, 0);
		}
	}
	break;
	//Здесь лучше отслеживать ихменения префикса, а изменения маски лучше отслеживать в сообщении 'WM_COMMAND',
	//поскольку тогда маска и префикс связаны более интерактивно
	case WM_NOTIFY:
	{
		if (wParam == IDC_IPMASK || wParam == IDC_IPADDRESS)
		{
			//std::cout << "WM_NOTYFY:IDC_IPMASK" << std::endl;
			//std::cout << ((NMIPADDRESS*)lParam)->hdr.idFrom << std::endl;
			//std::cout << ((NMIPADDRESS*)lParam)-> << std::endl;
			SetIPPrefix(hwnd);
		}
		PrintInfo(hwnd);
	}
	break;
	case WM_CLOSE:
		FreeConsole();
		EndDialog(hwnd, 0);
	}
	return FALSE;
}
VOID SetIPPrefix(HWND hwnd)
{
	HWND hIPmask = GetDlgItem(hwnd, IDC_IPMASK);
	HWND hIPprefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);
	DWORD dwIPmask = 0;
	SendMessage(hIPmask, IPM_GETADDRESS, 0, (LPARAM)&dwIPmask);
	DWORD count = 0;
	for (DWORD i = dwIPmask; 0x80000000 & i; i <<= 1, count++);
	CHAR szIPprefix[3] = "";
	sprintf(szIPprefix, "%i", count);
	SendMessage(hIPprefix, WM_SETTEXT, 0, (LPARAM)szIPprefix);
}
VOID PrintInfo(HWND hwnd)
{
	CONST INT SIZE = 1024;
	CHAR sz_info[SIZE]{};
	CHAR sz_buffer[SIZE]{};
	CHAR sz_NetworkIP_buffer[SIZE];
	CHAR sz_BroadcastIP_buffer[SIZE];
	CHAR sz_NumerOfIPs[SIZE];
	CHAR sz_NumerOfHosts[SIZE];
	CHAR sz_prefix[3];
	HWND hIPaddress = GetDlgItem(hwnd, IDC_IPADDRESS);
	HWND hIPmask = GetDlgItem(hwnd, IDC_IPMASK);
	HWND hEditPrefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);
	HWND hStaticInfo = GetDlgItem(hwnd, IDC_STATIC_INFO);
	DWORD dwIPaddress = 0;
	DWORD dwIPmask = 0;
	DWORD dwIPprefix = 0;

	SendMessage(hIPaddress, IPM_GETADDRESS, 0, (LPARAM)&dwIPaddress);
	SendMessage(hIPmask, IPM_GETADDRESS, 0, (LPARAM)&dwIPmask);
	SendMessage(hEditPrefix, WM_GETTEXT, 3, (LPARAM)sz_prefix);
	dwIPprefix = atoi(sz_prefix);

	sprintf(sz_NetworkIP_buffer, "Адрес сети:\t\t\t%s", IPaddressToString(dwIPaddress & dwIPmask, sz_buffer));
	sprintf(sz_BroadcastIP_buffer, "Широковещательный адрес:\t%s", IPaddressToString(dwIPaddress | ~dwIPmask, sz_buffer));
	sprintf(sz_NumerOfIPs, "Количество IP-адресов:\t%u", 1 << (32 - dwIPprefix));
	sprintf(sz_NumerOfHosts, "Количество узлов:\t\t%u", (1 << (32 - dwIPprefix)) - 2);

	/*sprintf(sz_NumerOfIPs, "Количество IP-адресов:\t%i", 1 << (32 - dwIPprefix == 0 ? 32 : dwIPprefix));
	sprintf(sz_NumerOfHosts, "Количество узлов:\t\t%i", (1 << (32 - dwIPprefix == 0 ? 32 : dwIPprefix)) - 2);*/

	sprintf(sz_info, "%s\n%s\n%s\n%s", sz_NetworkIP_buffer, sz_BroadcastIP_buffer, sz_NumerOfIPs, sz_NumerOfHosts);
	SendMessage(hStaticInfo, WM_SETTEXT, 0, (LPARAM)sz_info);
}
CHAR* IPaddressToString(DWORD dwIPaddress, CHAR sz_IPaddress[])
{
	sprintf
	(
		sz_IPaddress,
		"%i.%i.%i.%i",
		FIRST_IPADDRESS(dwIPaddress),
		SECOND_IPADDRESS(dwIPaddress),
		THIRD_IPADDRESS(dwIPaddress),
		FOURTH_IPADDRESS(dwIPaddress)
	);
	return sz_IPaddress;
}