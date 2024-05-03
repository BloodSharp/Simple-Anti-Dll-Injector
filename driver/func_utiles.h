#ifndef FUNC_UTILES_H
#define FUNC_UTILES_H

typedef NTSTATUS (WINAPI *TZwQueryInformationProcess)(HANDLE,PROCESSINFOCLASS,PVOID,ULONG,PULONG);
TZwQueryInformationProcess ZwQueryInformationProcess=NULL;

BOOL __stdcall ImageFullPath(PEPROCESS eprocess,PCHAR fullname)
{
    BOOL ret=FALSE;
    BYTE buffer[sizeof(UNICODE_STRING)+MAX_PATH*sizeof(WCHAR)];
    HANDLE handle;
    DWORD returnedLength=0;
    ANSI_STRING DestinationString;
    if(NT_SUCCESS(ObOpenObjectByPointer(eprocess,OBJ_KERNEL_HANDLE,NULL,GENERIC_READ,0,KernelMode,&handle)))
    {
        if(NT_SUCCESS(ZwQueryInformationProcess(handle,ProcessImageFileName,buffer,sizeof(buffer),&returnedLength)))
        {
            RtlUnicodeStringToAnsiString(&DestinationString,(UNICODE_STRING*)buffer,TRUE);
            strncpy(fullname,DestinationString.Buffer,DestinationString.Length);
            ret=TRUE;
            fullname[DestinationString.Length]=0;
            RtlFreeAnsiString(&DestinationString);
        }
        ZwClose(handle);
    }
    return ret;
}

BOOL __stdcall ImageFileName(PEPROCESS eprocess,PCHAR filename)
{
    CHAR sImageFullPath[MAX_PATH]={0};
    if(ImageFullPath(eprocess,sImageFullPath))
    {
        PCHAR pIFN=sImageFullPath,pIFP=sImageFullPath;
        while(*pIFP)
            if(*(pIFP++)=='\\')
                pIFN=pIFP;
        strcpy(filename,pIFN);
        return TRUE;
    }
    return FALSE;
}

DWORD GetProcessIdByHandle(HANDLE Process)
{
	PROCESS_BASIC_INFORMATION ProcessBasicInfo;
	if(NT_SUCCESS(ZwQueryInformationProcess(Process,ProcessBasicInformation,&ProcessBasicInfo,sizeof(PROCESS_BASIC_INFORMATION),NULL)))
		return ProcessBasicInfo.UniqueProcessId;
	return 0;
}
/*
BOOL __stdcall IsCurrentProcess(PCHAR Process)
{
	PROCESS_BASIC_INFORMATION ProcessBasicInfo;
    CHAR ProcName[MAX_PATH];
    PEPROCESS Curr=PsGetCurrentProcess();
    if(ImageFileName(Curr,ProcName))
        if(!strcmp(ProcName,Process))
            return TRUE;
    return FALSE;
}
*/

#endif // FUNC_UTILES_H