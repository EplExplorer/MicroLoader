
#include "EHelpFunc.h"

extern EContext* AppContext;

DWORD ServerPointTable[ESERVERCOUNT];

LPSTR sErrorListForE[] =
{
(LPSTR)"数组成员引用下标超出定义范围",
(LPSTR)"未得到所需要的数值型数据",
(LPSTR)"引用数组成员时维数不为1且不等于该数组目前所具有的维数",
(LPSTR)"数组成员引用下标必须大于等于1",
(LPSTR)"数据或数组类型不匹配",
(LPSTR)"调用DLL命令后发现堆栈错误，通常是DLL参数数目不正确",
(LPSTR)"子语句参数未返回数据或者返回了非系统基本类型或数组型数据",
(LPSTR)"被比较数据只能使用等于或不等于命令比较",
(LPSTR)"“多项选择”命令的索引值参数小于零或超出了所提供参数表范围",
(LPSTR)"“重定义数组”命令的数组维定义参数值必须大于零或单维时大于等于零",
(LPSTR)"所提供参数的数据类型不符合要求"
};

char* DefaultSystemAPI[] = {
	(char*)"kernel32.dll",
	(char*)"user32.dll",
	(char*)"ntdll.dll",
	(char*)"advapi32.dll"
};

#include "krnln/krnln_bnot.hpp"
#include "krnln/krnln_SetErrorManger.hpp"

KernelCmd KernelBaseCmd[] = {

	// 在这里添加偏移量对应的函数指针
	// {偏移量, 指针}
	{0xc0, krnln_bnot},
	{0xa08, krnln_SetErrorManger},
	{0x282, krnln_SetErrorManger},
	//{0xc4, band},
	//{0xc8, bor},
	//{0xcc, bxor},

	//{0x9f8, shl},
	//{0x9fc, shr},

	//{0x90c, pstr},
	//{0x910, pbin},

	{-1, 0},
};

void _cdecl krnl_MFree(void* lpMem)
{
	if (HeapValidate(AppContext->Heap, HEAP_NO_SERIALIZE, lpMem) == 0)
	{
		return;
	}

	HeapFree(AppContext->Heap, 0, lpMem);
}

void* _cdecl krnl_MMalloc(DWORD dwSize)
{
	void* pData = HeapAlloc(AppContext->Heap, HEAP_ZERO_MEMORY, dwSize);

	if (!pData)
	{
		char ErrorString[255];
		char** pRetnAddr = (char**)(&dwSize - 1); // 取返回地址
		wsprintf(ErrorString, "申请%d字节内存失败.\r\n"
			"栈信息:\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n",
			dwSize,
			pRetnAddr[0], pRetnAddr[1],
			pRetnAddr[2], pRetnAddr[3],
			pRetnAddr[4], pRetnAddr[5],
			pRetnAddr[6], pRetnAddr[7],
			pRetnAddr[8], pRetnAddr[9],
			pRetnAddr[10], pRetnAddr[11],
			pRetnAddr[12], pRetnAddr[13],
			pRetnAddr[14], pRetnAddr[15]);
		MessageBox(0, ErrorString, "error", MB_ICONERROR);
		krnl_MExitProcess(0);
	}

	return pData;
}

__declspec(naked) void _cdecl krnl_MExitProcess(DWORD uExitCode)
{

	__asm {
		push ebp
		mov ebp, esp
	}

	if (AppContext->ExitCallBack != 0) {
		UNKNOWFUN callback = (UNKNOWFUN)AppContext->ExitCallBack;
		callback();
	}

	FreeContext();

	ExitProcess(uExitCode);
	
	__asm {
		ret
	}

}

void _cdecl krnl_MOtherHelp(DWORD lpCallBack) 
{
	AppContext->ExitCallBack = (void*)lpCallBack;
}

void* _cdecl krnl_MRealloc(void* lpMem, DWORD dwSize)
{
	void* pData;

	if (lpMem)
		pData = HeapReAlloc(AppContext->Heap, 0, lpMem, dwSize);
	else
		pData = HeapAlloc(AppContext->Heap, HEAP_ZERO_MEMORY, dwSize);

	if (!pData)
	{
		char ErrorString[255];
		char** pRetnAddr = (char**)(&dwSize - 2); // 取返回地址
		wsprintf(ErrorString, "申请%d字节内存失败.\r\n"
			"栈信息:\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n"
			"0x%08X\r\n0x%08X\r\n",
			lpMem,
			pRetnAddr[0], pRetnAddr[1],
			pRetnAddr[2], pRetnAddr[3],
			pRetnAddr[4], pRetnAddr[5],
			pRetnAddr[6], pRetnAddr[7],
			pRetnAddr[8], pRetnAddr[9],
			pRetnAddr[10], pRetnAddr[11],
			pRetnAddr[12], pRetnAddr[13],
			pRetnAddr[14], pRetnAddr[15]);
		MessageBox(0, "内存不足", "error", MB_ICONERROR);
		krnl_MExitProcess(0);
	}

	return pData;
}

void _cdecl krnl_MReportError(DWORD nMsg, DWORD dwMethodId, DWORD dwPos)
{
	char ErrorString[255];

	LPSTR ptxt = NULL;
	if (0 < nMsg && nMsg < 12)
		ptxt = sErrorListForE[nMsg - 1];

	BOOL bThreeParam = FALSE;
	char* pRetnAddr = (char*)*(&nMsg - 1); // 取返回地址
	if (NULL != pRetnAddr)
	{
		if (0xC483 == *(unsigned short*)(pRetnAddr)) // 是否为add esp, xxx;
		{
			bThreeParam = (0x0C <= *(pRetnAddr + 2)); // 平栈数量为12，表示有三个参数
		}
	}
	if (bThreeParam) // 易语言5.8版本及以上才会有三个参数，旧的版本只有一个参数。
	{
		wsprintf(ErrorString, "program internal error number is %d. \r\n%s\r\n错误位置:%d,%d", nMsg, ptxt, dwMethodId, dwPos);
	}
	else
	{
		wsprintf(ErrorString, "program internal error number is %d. \r\n%s\r\n", nMsg, ptxt);
	}

	INT nNoErrorBox = 0;

	// 如果在错误回调中依然出现错误将不会再次调用错误回调
	if (AppContext->ErrorCallBack && !AppContext->IsErrorCallBack)
	{

		ERRORCALLBACK callback = (ERRORCALLBACK)AppContext->ErrorCallBack;

		AppContext->IsErrorCallBack = true;
		nNoErrorBox = callback(nMsg, ErrorString);
		AppContext->IsErrorCallBack = false;
	}

	if (!nNoErrorBox)
		MessageBox(0, ErrorString, "error", MB_ICONERROR);

	krnl_MExitProcess(0);
}

void _cdecl krnl_MReadProperty(void) {
	MessageBoxA(0, "暂不支持窗口相关功能", "error", MB_ICONERROR);
	krnl_MExitProcess(0);
	return;
}

void _cdecl krnl_MWriteProperty(void) {
	MessageBoxA(0, "暂不支持窗口相关功能", "error", MB_ICONERROR);
	krnl_MExitProcess(0);
	return;
}

void _cdecl krnl_MLoadBeginWin(void) {
	MessageBoxA(0, "暂不支持窗口相关功能", "error", MB_ICONERROR);
	krnl_MExitProcess(0);
	return;
}

void _cdecl krnl_MMessageLoop(UINT32 Param1) {
	return;
}

UINT32 DllCmdNO_CallDll;

PDLLCMD DllCmd_CallDll;

DWORD temp_CallDll;

DWORD i_CallDll;

HMODULE ThisLibrary_CallDll;

UNKNOWFUN UnKnowFun_CallDll;

__declspec(naked) void _stdcall krnl_MCallDllCmd(void) {

	__asm {
		pop temp_CallDll
		push eax
		pop DllCmdNO_CallDll
	}

	DllCmd_CallDll = AppContext->DllCmdHead;
	DllCmd_CallDll += DllCmdNO_CallDll;

	if ((*DllCmd_CallDll->DllFileName) == NULL) {
		i_CallDll = 0;
		while ((*DefaultSystemAPI[i_CallDll]) != NULL) {
			ThisLibrary_CallDll = LoadLibrary(DefaultSystemAPI[i_CallDll]);
			UnKnowFun_CallDll =
				(UNKNOWFUN)GetProcAddress(ThisLibrary_CallDll, DllCmd_CallDll->DllCmdName);
			if (UnKnowFun_CallDll != NULL) {
				__asm {
					// call UnKnowFun
					// 这个地方可能会引起杀毒软件的误报，如果有必要的话，需要处理一下
					push temp_CallDll
					jmp UnKnowFun_CallDll
				}
				break;
			}
			else {
				FreeLibrary(ThisLibrary_CallDll);
				i_CallDll++;
			}
		}

		if (UnKnowFun_CallDll == NULL) {
			krnl_MExitProcess(0);
		}

	}
	else {
		ThisLibrary_CallDll = LoadLibrary(DllCmd_CallDll->DllFileName);
		UnKnowFun_CallDll = (UNKNOWFUN)GetProcAddress(ThisLibrary_CallDll, DllCmd_CallDll->DllCmdName);
		if (UnKnowFun_CallDll != NULL) {
			__asm {
				// call UnKnowFun
				push temp_CallDll
				jmp UnKnowFun_CallDll
			}
		}
		else {
			krnl_MExitProcess(0);
		}
	}

	__asm {
		push eax
	}

	FreeLibrary(ThisLibrary_CallDll);

	__asm {
		pop eax
		push temp_CallDll
		ret
	}
}


PLIBINFO LibInfo_CallLib;
PFN_EXECUTE_CMD** CmdsFuncHead_CallLib;

__declspec(naked) void _cdecl krnl_MCallLibCmd(void) {

	DWORD LibCmdNO;

	__asm {
		push ebp
		mov ebp, esp
		mov LibCmdNO, eax
	}
	LibInfo_CallLib = AppContext->LibInfoHead;
	LibInfo_CallLib += LibCmdNO;
	if (LibInfo_CallLib->LibHandle == NULL || LibInfo_CallLib->LibInfo == NULL) {
		char ErrorString[256];

		sprintf(ErrorString, "%s\n\nIntra error string is \"%s\".", "无法调用支持库",
			LibInfo_CallLib->LibName);
		MessageBoxA(0, ErrorString, "error", MB_ICONERROR);
		krnl_MExitProcess(0);
	}
	CmdsFuncHead_CallLib = &(LibInfo_CallLib->LibInfo->m_pCmdsFunc);
	__asm {
		mov edx, [CmdsFuncHead_CallLib]
		add ebx, [edx]
		lea edx, dword ptr ss : [esp + 0x0c]
		sub esp, 0x0c
		push edx
		push dword ptr ss : [esp + 0x18]
		mov dword ptr ss : [esp + 0x08] , 0
		mov dword ptr ss : [esp + 0x0c] , 0
		mov dword ptr ss : [esp + 0x10] , 0
		lea edx, dword ptr ss : [esp + 0x08]
		push edx
		call dword ptr ds : [ebx]
		mov eax, dword ptr ss : [esp + 0x0c]
		mov edx, dword ptr ss : [esp + 0x10]
		mov ecx, dword ptr ss : [esp + 0x14]
		mov esp, ebp
		pop ebp
		retn
	}
}

int temp_CallKrnl;
bool FindOK_CallKrnl;
DWORD ThisBaseCmdOffset_CallKrnl;
PFN_EXECUTE_CMD* ThisExecuteCmdPoint_CallKrnl;
PLIBINFO ThisLibInfo_CallKrnl;
DWORD LibCmdNO;
char ErrorString_CallKrnl[256];
PFN_EXECUTE_CMD** ThisCmdsFuncHead_CallKrnl;

__declspec(naked) void _cdecl krnl_MCallKrnlLibCmd(void) {

	__asm {
		push ebp
		mov ebp, esp
		mov LibCmdNO, 0
		mov ThisBaseCmdOffset_CallKrnl, ebx
	}
	temp_CallKrnl = 0;
	FindOK_CallKrnl = false;
	while (KernelBaseCmd[temp_CallKrnl].CmdOffset != -1) {
		if (KernelBaseCmd[temp_CallKrnl].CmdOffset == ThisBaseCmdOffset_CallKrnl) {
			FindOK_CallKrnl = true;
			ThisExecuteCmdPoint_CallKrnl = &(KernelBaseCmd[temp_CallKrnl].CmdPoint);
			break;
		}
		temp_CallKrnl++;
	}
	if (FindOK_CallKrnl == false) {
		ThisLibInfo_CallKrnl = AppContext->LibInfoHead;
		ThisLibInfo_CallKrnl += LibCmdNO;
		if (ThisLibInfo_CallKrnl->LibHandle == NULL ||
			ThisLibInfo_CallKrnl->LibInfo == NULL) {
			sprintf(ErrorString_CallKrnl, "%s\n\nIntra error string is \"%s\".", "无法调用支持库",
				ThisLibInfo_CallKrnl->LibName);
			MessageBoxA(0, ErrorString_CallKrnl, "error", MB_ICONERROR);
			krnl_MExitProcess(0);
		}
		ThisCmdsFuncHead_CallKrnl = &(ThisLibInfo_CallKrnl->LibInfo->m_pCmdsFunc);

		__asm {
			mov eax, [ThisCmdsFuncHead_CallKrnl]
			add ebx, dword ptr ds : [eax]
		}
	}
	else {
		__asm {
			mov ebx, ThisExecuteCmdPoint_CallKrnl
		}
	}
	__asm {
		lea eax, dword ptr ss : [esp + 0x0c]
		sub esp, 0x0c
		push eax
		push dword ptr ss : [esp + 0x18]
		xor eax, eax
		mov dword ptr ss : [esp + 0x08] , eax
		mov dword ptr ss : [esp + 0x0c] , eax
		mov dword ptr ss : [esp + 0x10] , eax
		lea edx, dword ptr ss : [esp + 0x08]
		push edx
		call dword ptr ds : [ebx]
		mov eax, dword ptr ss : [esp + 0x0C]
		mov edx, dword ptr ss : [esp + 0x10]
		mov ecx, dword ptr ss : [esp + 0x14]
		mov esp, ebp
		pop ebp
		retn
	}

}

void InitServerPointTable(void) {
	ServerPointTable[0] = (DWORD)krnl_MReportError; // _krnl_MReportError
	ServerPointTable[1] = (DWORD)krnl_MCallDllCmd; // _krnl_MCallDllCmd
	ServerPointTable[2] = (DWORD)krnl_MCallLibCmd; // _krnl_MCallLibCmd
	ServerPointTable[3] = (DWORD)krnl_MCallKrnlLibCmd; // _krnl_MCallKrnlLibCmd
	ServerPointTable[4] = (DWORD)krnl_MReadProperty; // _krnl_MReadProperty
	ServerPointTable[5] = (DWORD)krnl_MWriteProperty; // _krnl_MWriteProperty
	ServerPointTable[6] = (DWORD)krnl_MMalloc; // _krnl_MMalloc
	ServerPointTable[7] = (DWORD)krnl_MRealloc; // _krnl_MRealloc
	ServerPointTable[8] = (DWORD)krnl_MFree; // _krnl_MFree
	ServerPointTable[9] = (DWORD)krnl_MExitProcess; // _krnl_MExitProcess
	ServerPointTable[10] = (DWORD)krnl_MMessageLoop; // _krnl_MMessageLoop
	ServerPointTable[11] = (DWORD)krnl_MLoadBeginWin; // _krnl_MLoadBeginWin
	ServerPointTable[12] = (DWORD)krnl_MOtherHelp; // _krnl_MOtherHelp
}

void UpdataServerPointTable(void* pBase, DWORD nHelpOffset)
{
	UINT32* ptemp = (UINT32*)((UINT32)pBase +
		nHelpOffset);
	for (int i = 0; i <= ESERVERCOUNT - 1; i++) {
		(*ptemp) = ServerPointTable[i];
		ptemp++;
	}
}