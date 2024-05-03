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

#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
#include "func_utiles.h"

typedef struct SSDT_ENTRY_s
{
	PUINT   ServiceTableBase;
	PUINT   ServiceCounterTableBase;
	UINT    NumberOfServices;
	PUCHAR  ParamTableBase;
}SSDT_Entry,*PSSDT_Entry;

PSSDT_Entry KeServiceDescriptorTable=NULL;

//------------------------------------------------------------------------------------------
// ZwCreateThread (Intenta crear el hilo)
//------------------------------------------------------------------------------------------
UINT ZwCreateThreadIndex;
typedef struct _USER_STACK
{
	PVOID  FixedStackBase;
	PVOID  FixedStackLimit;
	PVOID  ExpandableStackBase;
	PVOID  ExpandableStackLimit;
	PVOID  ExpandableStackBottom;
}USER_STACK,*PUSER_STACK;
typedef NTSTATUS (NTAPI *TZwCreateThread)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES,
										  HANDLE,PCLIENT_ID,PCONTEXT,PUSER_STACK,BOOLEAN);
TZwCreateThread pZwCreateThread;
NTSTATUS NTAPI HOOK_ZwCreateThread(OUT PHANDLE ThreadHandle,IN ACCESS_MASK DesiredAccess,
								   IN POBJECT_ATTRIBUTES ObjectAttributes,
								   IN HANDLE ProcessHandle,OUT PCLIENT_ID ClientId,
								   IN PCONTEXT ThreadContext,IN PUSER_STACK UserStack,
								   IN BOOLEAN CreateSuspended)
{
	NTSTATUS lStatus;
	CHAR CurFileName[MAX_PATH]={0},ProcFileName[MAX_PATH]={0};
	HANDLE CurrentPID=PsGetCurrentProcessId();
	HANDLE ProcessHandleId=(HANDLE)GetProcessIdByHandle(ProcessHandle);
	PEPROCESS EProc;

	lStatus=PsLookupProcessByProcessId(CurrentPID,&EProc);
	if(lStatus==STATUS_SUCCESS)
	{
		ImageFileName(EProc,CurFileName);
		ObDereferenceObject(EProc);
	}
	lStatus=PsLookupProcessByProcessId(ProcessHandleId,&EProc);
	if(lStatus==STATUS_SUCCESS)
	{
		ImageFileName(EProc,ProcFileName);
		ObDereferenceObject(EProc);
	}

	if(!strcmp(_strlwr(ProcFileName),"hl.exe"))
	{
		if(CurrentPID==ProcessHandleId || !strcmp(_strlwr(CurFileName),"explorer.exe"))
			return pZwCreateThread(ThreadHandle,DesiredAccess,ObjectAttributes,
									ProcessHandle,ClientId,ThreadContext,UserStack,
									CreateSuspended);
		else
		{
			DbgPrint("%s tried to create a remote thread in the game... ACCESS DENIED!!!\n",CurFileName);
			return STATUS_UNSUCCESSFUL;
		}
	}
	return pZwCreateThread(ThreadHandle,DesiredAccess,ObjectAttributes,ProcessHandle,
							ClientId,ThreadContext,UserStack,CreateSuspended);
}

void DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	_asm
	{
		cli;
		mov eax,cr0;
		and eax,0xfffeffff;
		mov cr0,eax;
		sti;
	}
	KeServiceDescriptorTable->ServiceTableBase[ZwCreateThreadIndex]=(UINT)pZwCreateThread;
	_asm
	{
		cli;
		mov eax,cr0;
		or eax,0x00010000;
		mov cr0,eax;
		sti;
	}
	DbgPrint("Processors unhooked!\n");
}

extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,
								IN PUNICODE_STRING pRegistryPath)
{
	UNICODE_STRING usSSDT,usZwQIP;

	pDriverObject->DriverUnload = DriverUnload;

	RtlInitUnicodeString(&usSSDT,L"KeServiceDescriptorTable");
	KeServiceDescriptorTable = (PSSDT_Entry)MmGetSystemRoutineAddress(&usSSDT);
	if(!KeServiceDescriptorTable)
	{
		DbgPrint("Couldn't Get SSDT :S\n");
		return STATUS_UNSUCCESSFUL;
	}
	RtlInitUnicodeString(&usZwQIP,L"ZwQueryInformationProcess");
	ZwQueryInformationProcess = (TZwQueryInformationProcess)
								MmGetSystemRoutineAddress(&usZwQIP);
	if(!ZwQueryInformationProcess)
	{
		DbgPrint("Couldn't Get ZwQueryInformationProcess :S\n");
		return STATUS_UNSUCCESSFUL;
	}

	ULONG Major;
	ULONG Minor;
	ULONG BuildNumber;
	UNICODE_STRING CSDVersion;
	PsGetVersion(&Major,&Minor,&BuildNumber,&CSDVersion);
	if(Major==5)
		if(Minor==1)
		{
			DbgPrint("Windows XP detected!\n");
			ZwCreateThreadIndex=0x35;
		}
	else if(Major==6)
	{
		if(!Minor)
		{
			DbgPrint("Windows Vista detected!\n");// Or Windows 2008 Server WARNING!!!
			ZwCreateThreadIndex=0x4C;
		}
		else if(Minor==1)
		{
			DbgPrint("Windows Seven detected!\n");// Or Windows 2008 Server R2 WARNING!!!
			ZwCreateThreadIndex=0x57;
		}
	}
	else
	{
		DbgPrint("This Windows version is not supported...\n");
		return STATUS_UNSUCCESSFUL;
	}

	_asm
	{
		cli;
		mov eax,cr0;
		and eax,0xfffeffff;
		mov cr0,eax;
		sti;
	}
	pZwCreateThread = (TZwCreateThread)KeServiceDescriptorTable->ServiceTableBase[ZwCreateThreadIndex];
	KeServiceDescriptorTable->ServiceTableBase[ZwCreateThreadIndex] = (UINT)HOOK_ZwCreateThread;
	_asm
	{
		cli;
		mov eax,cr0;
		or eax,0x00010000;
		mov cr0,eax;
		sti;
	}

	return STATUS_SUCCESS;
}