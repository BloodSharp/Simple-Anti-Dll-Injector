//******************************************************************************************
//*
//*    Simple Anti Dll Injector
//*    Inline EAX Copyright (C) 2012 Agustín Alejandro dos Santos
//*    
//*    This program is free software: you can redistribute it and/or modify
//*    it under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    This program is distributed in the hope that it will be useful,
//*    but WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//*    GNU General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//*
//*    inline-eax.blogspot.com.ar
//******************************************************************************************
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

char Direccion[MAX_PATH];
SC_HANDLE hSCManager;
SC_HANDLE hService;
SERVICE_STATUS ss;
typedef BOOL (WINAPI *TIsWow64Process)(HANDLE,PBOOL);
TIsWow64Process pIsWow64Process;

BOOL FileExist(LPCSTR filename)
{
     WIN32_FIND_DATA finddata;
     return (FindFirstFile(filename,&finddata)!=INVALID_HANDLE_VALUE);
}

int WINAPI WinMain (HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszArgument,
					int nFunsterStil)
{
	BOOL Is64=FALSE;
	HINSTANCE hKernel=LoadLibrary("Kernel32.dll");
	pIsWow64Process=(TIsWow64Process)GetProcAddress(hKernel,"IsWow64Process");
	if(NULL!=pIsWow64Process)
	{
		pIsWow64Process(GetCurrentProcess(),&Is64);
		if(Is64)
		{
			MessageBox(0,"Windows 64 bits... AntiInjector will not run!",
				"AntiInjector - BloodSharp",MB_ICONERROR);
			return 0;
		}
	}
	GetCurrentDirectory(sizeof(Direccion),Direccion);
	strcat(Direccion,"\\AntiInjectorDriver.sys");
	if(!FileExist(Direccion))
		MessageBox(0,"antiInjector.sys not found! The anticheat will not run!",
		"antiInjector - BloodSharp",MB_ICONINFORMATION);
	else
	{
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if(hSCManager)
		{
			hService=CreateService(hSCManager,"AntiInjector","Nanananana Líder!!!",
				SERVICE_START|DELETE|SERVICE_STOP,SERVICE_KERNEL_DRIVER,
				SERVICE_DEMAND_START,SERVICE_ERROR_IGNORE,Direccion,NULL,NULL,NULL,NULL,
				NULL);
			if(!hService)
				hService=OpenService(hSCManager,"AntiInjector",
				SERVICE_START|DELETE|SERVICE_STOP);
			if(hService)
			{
				if(StartService(hService, 0, NULL))
				{
					MessageBox(0,"AntiInjector successfully loaded, try to inject a dll in hl.exe...\nPress Ok to close AntiInjector!",
						"AntiInjector - BloodSharp",MB_ICONINFORMATION);
					ControlService(hService,SERVICE_CONTROL_STOP,&ss);
				}
				else
					MessageBox(0,"Can't Start Driver! :S","AntiInjector - BloodSharp",MB_ICONERROR);
				DeleteService(hService);
				CloseServiceHandle(hService);
			}
			else
				MessageBox(0,"Can't Create Service! :S","AntiInjector - BloodSharp",MB_ICONERROR);
			CloseServiceHandle(hSCManager);
		}
		else
			MessageBox(0,"Can't Open Service Manager! :S","AntiInjector - BloodSharp",MB_ICONERROR);
	}
	return 0;
}
