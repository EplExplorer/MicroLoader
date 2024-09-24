#include "EHelpFunc.h"

// 引入日志库
#include "logger/easylogging++.h"

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
	(char*)"advapi32.dll",
	NULL
};

KernelCmd KernelBaseCmd;

void _cdecl krnl_MFree(void* lpMem)
{

#ifdef _DEBUG
	LOG(INFO) << "内存释放 lpMem: " << (DWORD)lpMem;
#endif

	if (HeapValidate(AppContext->Heap, HEAP_NO_SERIALIZE, lpMem) == 0)
	{
		return;
	}

	HeapFree(AppContext->Heap, 0, lpMem);
}

void* _cdecl krnl_MMalloc(DWORD dwSize)
{

#ifdef _DEBUG
	LOG(INFO) << "内存分配 dwSize: " << dwSize;
#endif

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

void _cdecl krnl_MExitProcess(DWORD uExitCode)
{

#ifdef _DEBUG
	LOG(INFO) << "程序退出 uExitCode: " << uExitCode;
#endif

	if (AppContext->ExitCallBack != 0) {
		UNKNOWFUN callback = (UNKNOWFUN)AppContext->ExitCallBack;
		callback();
	}

	FreeContext();

	ExitProcess(uExitCode);

}

void _cdecl krnl_MOtherHelp(DWORD lpCallBack)
{

#ifdef _DEBUG
	LOG(INFO) << "krnl_MOtherHelp lpCallBack: " << (DWORD)lpCallBack;
#endif

	AppContext->ExitCallBack = (void*)lpCallBack;
}

void* _cdecl krnl_MRealloc(void* lpMem, DWORD dwSize)
{

#ifdef _DEBUG
	LOG(INFO) << "重分配内存 lpMem: " << (DWORD)lpMem << " dwSize: " << dwSize;
#endif

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

#ifdef _DEBUG
	LOG(INFO) << "易语言抛出异常 nMsg: " << nMsg << " dwMethodId: " << dwMethodId << " dwPos: " << dwPos;
#endif

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

void* _cdecl krnl_GetDllCmdAddress(DWORD DllCmdNO)
{

	PDLLCMD DllCmd = AppContext->DllCmdHead;
	DllCmd += DllCmdNO;

	void* pfn = NULL;

	if ((*DllCmd->DllFileName) == NULL)
	{
		DWORD i = 0;
		while ((*DefaultSystemAPI[i]) != NULL)
		{
			HMODULE Library = LoadLibrary(DefaultSystemAPI[i]);

			if (Library)
				pfn = GetProcAddress(Library, DllCmd->DllCmdName);

			if (pfn != NULL)
			{
#ifdef _DEBUG
				LOG(INFO) << "调用api dll: " << DefaultSystemAPI[i] << " symbol: " << DllCmd->DllCmdName;
#endif
				return pfn;
			}
			else
			{
				if (Library)
					FreeLibrary(Library);
				i++;
			}
		}

		if (pfn == NULL) {
			MessageBoxA(0, "dll引用失败", "error", MB_ICONERROR);
			krnl_MExitProcess(0);
		}
	}
	else
	{
		HMODULE Library = LoadLibrary(DllCmd->DllFileName);

		if (Library)
			pfn = GetProcAddress(Library, DllCmd->DllCmdName);

		if (pfn != NULL)
		{
#ifdef _DEBUG
			LOG(INFO) << "调用api dll: " << DllCmd->DllFileName << " symbol: " << DllCmd->DllCmdName;
#endif
			return pfn;
		}
		else
		{
			MessageBoxA(0, "dll引用失败", "error", MB_ICONERROR);
			krnl_MExitProcess(0);
		}
	}

	return NULL;
}

__declspec(naked) void _stdcall krnl_MCallDllCmd() {

	__asm {
		push eax
		call krnl_GetDllCmdAddress
		add esp, 4
		jmp eax
	}

}

void* _cdecl krnl_GetLibCmdAddress(DWORD LibCmdNO) {

	PLIBINFO LibInfo = AppContext->LibInfoHead;
	LibInfo += LibCmdNO;

#ifdef _DEBUG
	LOG(INFO) << "调用支持库 name: " << LibInfo->LibName << " handle: " << LibInfo->LibHandle;
#endif

	if (LibInfo->LibHandle == NULL || LibInfo->LibInfo == NULL) {
		char ErrorString[256];

		sprintf(ErrorString, "%s\n\nIntra error string is \"%s\".", "无法调用支持库",
			LibInfo->LibName);
		MessageBoxA(0, ErrorString, "error", MB_ICONERROR);
		krnl_MExitProcess(0);
	}
	return &(LibInfo->LibInfo->m_pCmdsFunc);
}

__declspec(naked) void _cdecl krnl_MCallLibCmd(void) {

	__asm {

		push ebp
		mov ebp, esp

		push eax // 压入库文件偏移量
		call krnl_GetLibCmdAddress
		mov esp, ebp  // 清除栈中的参数
		mov edx, eax // 将地址传给edx

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

void* _cdecl krnl_GetKrnlnCmdAddress(DWORD LibCmdNO) {

#ifdef _DEBUG
	LOG(INFO) << "调用核心库 offset: " << LibCmdNO;
#endif

	auto it = KernelBaseCmd.find(LibCmdNO);
	if (it != KernelBaseCmd.end()) {
		return &(it->second);
	}

	char ErrorString[256];

	sprintf(ErrorString, "%s%d", "核心库函数未实现: ", LibCmdNO);
	MessageBoxA(0, ErrorString, "error", MB_ICONERROR);
	krnl_MExitProcess(0);

	return NULL;
}

__declspec(naked) void _cdecl krnl_MCallKrnlLibCmd() {

	__asm {

		push ebp
		mov ebp, esp

		// 获取地址
		push ebx // 压入函数偏移量
		call krnl_GetKrnlnCmdAddress
		mov esp, ebp // 清除栈中的参数
		mov ebx, eax // 将地址传给ebx

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
