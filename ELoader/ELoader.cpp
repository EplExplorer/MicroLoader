/*
 * 作者: 66hh futz12
 * 许可证: GPLV3
 */

#include "ELoader.h"

// 引入日志库
#include "logger/easylogging++.h"

// 引入gc
#ifdef USE_BDWGC
#include "gc.h"
#endif // USE_BDWGC

INITIALIZE_EASYLOGGINGPP

extern EContext* AppContext;

DWORD FindECode()
{

	// 获取当前进程基址，遍历SectionTable查找易格式原体
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);

	if (DosHeader == NULL) {
		return -1;
	}

	PIMAGE_NT_HEADERS NtHeader = (PIMAGE_NT_HEADERS)((UINT32)DosHeader + DosHeader->e_lfanew);

	UINT32 NumberOfSections = NtHeader->FileHeader.NumberOfSections;

	PIMAGE_SECTION_HEADER SectionHeader =
		(PIMAGE_SECTION_HEADER)((UINT32)NtHeader + sizeof(IMAGE_NT_HEADERS));

	bool FindOK = false;
	char SectionName[IMAGE_SIZEOF_SHORT_NAME + 1];

#ifdef _DEBUG
	LOG(INFO) << "开始寻找区段";
#endif

	for (int i = 1; i <= NumberOfSections; i++) {

		memcpy(SectionName, SectionHeader->Name, IMAGE_SIZEOF_SHORT_NAME);
		SectionName[IMAGE_SIZEOF_SHORT_NAME] = 0;

#ifdef _DEBUG
		LOG(INFO) << SectionName;
#endif

		// 寻找易格式所在节的方法是：简单的比较当前SectionName是否是“.ecode”
		if (strcmp(SectionName, ESECTIONNAME) == 0) {
			// 找到了易格式所在的节
			FindOK = true;
			break;
		}

		SectionHeader++;
	}

	if (FindOK == false) {
		return -1;
	}

	return (DWORD)DosHeader + (DWORD)SectionHeader->VirtualAddress;
}

ESections LoadSections(PAPP_HEADER_INFO lpHeader)
{
	ESections result;

	result.pConstSectionOffset = (void*)((UINT32)lpHeader +
		lpHeader->m_nConstSectionOffset);
	result.pWinFormSectionOffset = (void*)((UINT32)lpHeader +
		lpHeader->m_nWinFormSectionOffset);
	result.pHelpFuncSectionOffset = (void*)((UINT32)lpHeader +
		lpHeader->m_nHelpFuncSectionOffset);
	result.pCodeSectionOffset = (void*)((UINT32)lpHeader +
		lpHeader->m_nCodeSectionOffset);
	result.pVarSectionOffset = (void*)((UINT32)lpHeader +
		lpHeader->m_nVarSectionOffset);

#ifdef _DEBUG
	LOG(INFO) << "易格式体数据装载完成";
	LOG(INFO) << "pConstSectionOffset: " << (DWORD)result.pConstSectionOffset;
	LOG(INFO) << "pWinFormSectionOffset: " << (DWORD)result.pWinFormSectionOffset;
	LOG(INFO) << "pHelpFuncSectionOffset: " << (DWORD)result.pHelpFuncSectionOffset;
	LOG(INFO) << "pCodeSectionOffset: " << (DWORD)result.pCodeSectionOffset;
	LOG(INFO) << "pVarSectionOffset: " << (DWORD)result.pVarSectionOffset;
#endif

	return result;
}

PDLLCMD LoadImportDll(PAPP_HEADER_INFO lpHeader, ESections lpSections)
{

#ifdef _DEBUG
	LOG(INFO) << "开始读取dll引用";
#endif

	if (lpHeader->m_nDllCmdCount > 0)
	{
		PDLLCMD EDllCmd = (PDLLCMD)malloc(sizeof(DLLCMD) * lpHeader->m_nDllCmdCount);

		PDLLCMD result = EDllCmd;

		for (int i = 1; i <= (UINT32)lpHeader->m_nDllCmdCount; i++) {
			EDllCmd->DllFileName = (char*)((UINT32)lpHeader +
				((PSECTION_INFO)lpSections.pConstSectionOffset)->m_nRecordOffset +
				(*(UINT*)((UINT32)lpHeader +
					sizeof(APP_HEADER_INFO) +
					(i - 1) * sizeof(INT))));
			EDllCmd->DllCmdName =
				(char*)((UINT32)lpHeader +
					((PSECTION_INFO)lpSections.pConstSectionOffset)->m_nRecordOffset +
					(*(UINT*)((UINT32)lpHeader +
						sizeof(APP_HEADER_INFO) +
						(i + lpHeader->m_nDllCmdCount - 1) *
						sizeof(INT))));

#ifdef _DEBUG
			LOG(INFO) << "dll: " << EDllCmd->DllFileName;
			LOG(INFO) << "symbol: " << EDllCmd->DllCmdName;
#endif

			EDllCmd++;
		}

		return result;
	}

	return NULL;
}

PLIBINFO LoadImportLib(PAPP_HEADER_INFO lpHeader, DWORD* dwCount)
{

#ifdef _DEBUG
	LOG(INFO) << "开始装载支持库";
#endif

	// 获取需要的支持库
	char* LibStringHead = (char*)((DWORD)lpHeader + sizeof(APP_HEADER_INFO) +
		lpHeader->m_nDllCmdCount * sizeof(INT) * 2);

	// 统计需要加载的支持库数量
	char* LibString = LibStringHead;
	*dwCount = 0;
	while ((*LibString) != NULL) {
		(*dwCount)++;
		LibString += (strlen(LibString) + 1);
	}

	LIBSTRINGINFO LibStringInfo;

	// 库信息, 库被装载后句柄与库文件信息将被存储在这里
	PLIBINFO LibInfoHead = (PLIBINFO)malloc(sizeof(LIBINFO) * *dwCount);
	PLIBINFO LibInfo = LibInfoHead;

	LibString = LibStringHead;

	// 循环读取直到出现 0x00(即结束)
	while ((*LibString) != NULL) {

		DWORD temp = 0;

		// 读取第一个字段 库文件名
		while ((*(LibString + temp)) != 0x0d) {
			LibStringInfo.LibName[temp] = (*(LibString + temp));
			temp++;
		}
		LibStringInfo.LibName[temp] = 0;

		// 读取第二个字段 支持库uuid
		temp += 1;
		while ((*(LibString + temp)) != 0x0d) {
			LibStringInfo.GUID[temp - strlen(LibStringInfo.LibName) - 1] =
				(*(LibString + temp));
			temp++;
		}
		LibStringInfo.GUID[temp - strlen(LibStringInfo.LibName) - 1] = 0;

#ifdef _DEBUG
		LOG(INFO) << "name: " << LibStringInfo.LibName;
		LOG(INFO) << "guid: " << LibStringInfo.GUID;
#endif

		// 初始化装载信息
		LibInfo->LibHandle = NULL;
		LibInfo->LibInfo = NULL;
		strcpy(LibInfo->LibName, LibStringInfo.LibName);

		// 添加后缀
		char LibFileName[256];
		strcpy(LibFileName, LibStringInfo.LibName);
		strcat(LibFileName, ".fne");

		bool Success = false;

		// 加载库文件
		LibInfo->LibHandle = LoadLibrary(LibFileName);

		// fne如果不存在就尝试加载fnr
		if (LibInfo->LibHandle == NULL)
		{
			strcpy(LibFileName, LibStringInfo.LibName);
			strcat(LibFileName, ".fnr");

			// 再次加载库文件
			LibInfo->LibHandle = LoadLibrary(LibFileName);

			if (LibInfo->LibHandle == NULL) {
				Success = false;
			}
			else {
				Success = true;
			}
		}
		else
		{
			Success = true;
		}

#ifdef _DEBUG
		if (Success)
			LOG(INFO) << LibStringInfo.LibName << "装载成功";
		else
			LOG(INFO) << LibStringInfo.LibName << "装载失败";
#endif

		// 支持库被成功装载就开始获取信息
		if (Success)
		{
			// 获取支持库接口地址
			PFN_GET_LIB_INFO GetThisNewInfo = (PFN_GET_LIB_INFO)GetProcAddress(
				LibInfo->LibHandle, FUNCNAME_GET_LIB_INFO);

			if (GetThisNewInfo == NULL) {
				// 载入的库没有输出FUNCNAME_GET_LIB_INFO函数，需要卸载掉
				FreeLibrary(LibInfo->LibHandle);
				LibInfo->LibHandle = NULL;
			}
			else {
				LibInfo->LibInfo = GetThisNewInfo();

				if (LibInfo->LibInfo == NULL) {
					// 输出的PLIB_INFO结构为空，需要卸载这样的支持库
					FreeLibrary(LibInfo->LibHandle);
					LibInfo->LibHandle = NULL;
					LibInfo->LibInfo = NULL;
				}
				else {
					if (strcmp(LibInfo->LibInfo->m_szGuid,
						LibStringInfo.GUID) != 0) {
						// 加载的支持库和需要的支持库GUID不同，需要卸载支持库
						FreeLibrary(LibInfo->LibHandle);
						LibInfo->LibHandle = NULL;
						LibInfo->LibInfo = NULL;
					}
					else {
						// 为当前支持库提供Notify函数指针
						PFN_NOTIFY_LIB NotifyLib = LibInfo->LibInfo->m_pfnNotify;
						if (NotifyLib != NULL) {
							NotifyLib(NL_SYS_NOTIFY_FUNCTION, (DWORD)ENotifySys, 0);
						}
					}
				}
			}

		}

		LibInfo++;

		LibString += (strlen(LibString) + 1);

	}

	return LibInfoHead;
}

void RelocECode(PAPP_HEADER_INFO lpHeader, ESections lpSections)
{

#ifdef _DEBUG
	LOG(INFO) << "开始重定位";
#endif

	DWORD temp = (DWORD)lpHeader->m_nBeginSectionOffset;

	while (temp != 0xFFFFFFFF) {
		PSECTION_INFO SectionInfo = (PSECTION_INFO)((UINT32)lpHeader + temp);

		if (SectionInfo->m_nReLocationItemCount > 0) {
			PRELOCATION_INF RelocationInfo =
				(PRELOCATION_INF)((UINT32)SectionInfo + sizeof(SECTION_INFO));
			for (int i = 1; i <= (UINT32)SectionInfo->m_nReLocationItemCount; i++) {

				UINT32* ptemp = (UINT32*)((UINT32)lpHeader +
					SectionInfo->m_nRecordOffset +
					RelocationInfo->m_dwOffset);

#ifdef _DEBUG

				// 打印数据
				LOG(INFO) << "section: " << SectionInfo->m_szName;
				LOG(INFO) << "type: " << RelocationInfo->m_btType;
				LOG(INFO) << "offset: " << RelocationInfo->m_dwOffset;

				if (strcmp(SectionInfo->m_szName, "code") == 0 || strcmp(SectionInfo->m_szName, "@code") == 0)
				{
					switch (RelocationInfo->m_btType) {
					case RT_HELP_FUNC:
						// 打印核心服务表下标
						LOG(INFO) << "调用核心服务: " << (DWORD)(*ptemp) / 4;
						LOG(INFO) << "地址: " << (DWORD)ptemp;
						break;
					case RT_CONST: {
						void* data = (void*)((*ptemp) + (UINT32)lpHeader + ((PSECTION_INFO)lpSections.pConstSectionOffset)->m_nRecordOffset);

						// 不打印字节集和其他数据
						if (*(DWORD*)data != 1 && *(DWORD*)data != 0) {
							LOG(INFO) << "引用字符串: " << (char*)data;
							LOG(INFO) << "地址: " << (DWORD)ptemp;
						}
					}
						break;
					default:
						break;
					}
				}

				LOG(INFO) << "----------------";
#endif

				switch (RelocationInfo->m_btType) {
				case RT_HELP_FUNC:
					(*ptemp) = (*ptemp) + (UINT32)lpHeader +
						((PSECTION_INFO)lpSections.pHelpFuncSectionOffset)->m_nRecordOffset;
					break;
				case RT_CONST:
					(*ptemp) = (*ptemp) + (UINT32)lpHeader +
						((PSECTION_INFO)lpSections.pConstSectionOffset)->m_nRecordOffset;
					break;
				case RT_GLOBAL_VAR:
					(*ptemp) = (*ptemp) + (UINT32)lpHeader +
						((PSECTION_INFO)lpSections.pVarSectionOffset)->m_nRecordOffset;
					break;
				case RT_CODE:
					(*ptemp) = (*ptemp) + (UINT32)lpHeader +
						((PSECTION_INFO)lpSections.pCodeSectionOffset)->m_nRecordOffset;
					break;
				default:
					break;
				}

				RelocationInfo++;
			}
		}
		temp = (UINT32)SectionInfo->m_nNextSectionOffset;
	}
}

int main()
{

#ifdef USE_BDWGC
	GC_init();
#endif

	// 初始化上下文
	InitContext(ENotifySys);

	// 寻找易数据段
	DWORD ESectionVA = FindECode();
	if (ESectionVA == -1)
	{
		MessageBoxA(0, "无法定位易代码数据", "error", MB_ICONERROR);
		return 0;
	}

	PAPP_HEADER_INFO ECodeHeaderInfo = (PAPP_HEADER_INFO)ESectionVA;

	// 读取易格式体区段数据
	ESections ECodeSections = LoadSections(ECodeHeaderInfo);

	// 加载依赖
	AppContext->DllCount = ECodeHeaderInfo->m_nDllCmdCount;
	AppContext->DllCmdHead = LoadImportDll(ECodeHeaderInfo, ECodeSections);
	AppContext->LibInfoHead = LoadImportLib(ECodeHeaderInfo, &AppContext->LibCount);

	// 获取易格式代码的起始指令地址,重定位完就可以移交控制权到易代码了
	ECODESTART ECodeStart = (ECODESTART)((UINT32)ECodeHeaderInfo +
		(UINT32)ECodeHeaderInfo->m_nStartCodeOffset);

	if (ECodeStart == NULL) {
		MessageBoxA(0, "无法定位易代码入口", "error", MB_ICONERROR);
		return 0;
	}

	// 重定位代码
	RelocECode(ECodeHeaderInfo, ECodeSections);

	// 初始化服务指针表
	InitServerPointTable();
	UpdataServerPointTable(ECodeHeaderInfo, ((PSECTION_INFO)ECodeSections.pHelpFuncSectionOffset)->m_nRecordOffset);


	// 至此初始化操作全部完成，转交控制权给易程序
	INT nProtectESP = 0;
	INT nProtectEBP = 0;

#ifdef _DEBUG
	LOG(INFO) << "开始执行";
	LOG(INFO) << "ECodeStart: " << (DWORD)ECodeStart;
#endif

	// 备份 esp 与 ebp
	_asm {
		mov nProtectESP, esp;
		mov nProtectEBP, ebp;
		call ECodeStart;
		mov ebp, nProtectEBP;
		mov esp, nProtectESP;
	}

#ifdef _DEBUG
	LOG(INFO) << "程序正常退出";
#endif

	krnl_MExitProcess(0);

	return 0;

}
