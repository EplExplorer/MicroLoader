#include "ENotifySys.h"

extern EContext *AppContext;

INT WINAPI ENotifySys(INT nMsg, DWORD dwParam1, DWORD dwParam2) {
    switch (nMsg) {
        case NRS_MALLOC:
            // 分配指定空间的内存，所有与易程序交互的内存都必须使用本通知分配。
            //   dwParam1为欲需求内存字节数。
            //   dwParam2如为0，则如果分配失败就自动报告运行时错并退出程序。
            // 如不为0，则如果分配失败就返回NULL。
            //   返回所分配内存的首地址。
            return (INT) krnl_MMalloc(dwParam1);
        case NRS_MFREE:
            // 释放已分配的指定内存。
            // dwParam1为欲释放内存的首地址。
            krnl_MFree((void *) dwParam1);
            break;
        case NRS_MREALLOC:
            // 重新分配内存。
            //   dwParam1为欲重新分配内存尺寸的首地址。
            //   dwParam2为欲重新分配的内存字节数。
            // 返回所重新分配内存的首地址，失败自动报告运行时错并退出程序。
            return (INT) krnl_MRealloc((void *) dwParam1, dwParam2);

        case NRS_FREE_ARY:
            // 释放指定数组数据。
            // dwParam1为该数据的DATA_TYPE，只能为系统数据类型。
            // dwParam2为指向该数组数据的指针。
            switch (dwParam1) {
                case SDT_TEXT:
                case SDT_BIN:
                    FreeAryElement((void *) dwParam2);
                    break;
                case SDT_INT:
                case SDT_BYTE:
                case SDT_SHORT:
                case SDT_FLOAT:
                case SDT_INT64:
                case SDT_DOUBLE:
                case SDT_SUB_PTR:
                case SDT_BOOL:
                case SDT_DATE_TIME:
                    krnl_MFree((void *) dwParam2);
                    break;
                default:
                    break;
            }
            break;

        case NRS_RUNTIME_ERR:
            // 通知系统已经产生运行时错误。
            // dwParam1为char*指针，说明错误文本。
        {
            char ErrorString[1024];
            wsprintfA(ErrorString, "BlackMoon RunTime Error:\r\n\r\n%s", dwParam1);
            MessageBoxA(0, ErrorString, "ERROR", MB_ICONERROR);
            krnl_MExitProcess(0);
        }
        break;
        case NRS_EXIT_PROGRAM:
            // 通知系统退出用户程序。
            // dwParam1为退出代码，该代码将被返回到操作系统。
            krnl_MExitProcess(dwParam1);
            break;
        case NRS_GET_PRG_TYPE:
            // 返回当前用户程序的类型，为PT_DEBUG_RUN_VER（调试版）或PT_RELEASE_RUN_VER（发布版）。
            return PT_RELEASE_RUN_VER;
        case NRS_DO_EVENTS:
            // 通知系统发送所有待处理事件。
        {
            MSG Msg;
            while (PeekMessage(&Msg, NULL, NULL, NULL, PM_NOREMOVE)) {
                if (Msg.message == WM_QUIT)
                    break;
                GetMessage(&Msg, NULL, 0, 0);
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
        break;
        case NRS_GET_CMD_LINE_STR:
            // 取当前命令行文本
            // 返回命令行文本指针，有可能为空串。
        {
            LPSTR p = GetCommandLineA();
            // 跳过调用程序名。
            char ch = ' ';
            if (*p++ == '\"')
                ch = '\"';
            while (*p++ != ch);
            if (ch != ' ' && *p == ' ') p++; // 跳过第一个空格。
            return (INT) p;
        }
        case NRS_GET_EXE_PATH_STR:
            // 取当前执行文件所处目录名称
            // 返回当前执行文件所处目录文本指针。
            if (::GetModuleFileNameA(NULL, AppContext->EFilePath, MAX_PATH)) {
                char *pFind = strrchr(AppContext->EFilePath, '\\');
                //	pFind++;
                *pFind = 0;
                return (INT) &AppContext->EFilePath;
            }
            break;

        case NRS_GET_EXE_NAME:
            // 取当前执行文件名称
            // 返回当前执行文件名称文本指针。
            if (::GetModuleFileNameA(NULL, AppContext->EFilePath, MAX_PATH)) {
                char *pFind = strrchr(AppContext->EFilePath, '\\');
                //pFind++;
                *pFind = 0;
                pFind++;
                return (INT) pFind;
            }
            break;
        case NRS_CONVERT_NUM_TO_INT: {
            // 转换其它数值格式到整数。
            // dwParam1为 PMDATA_INF 指针，其 m_dtDataType 必须为数值型。
            // 返回转换后的整数值。
            PMDATA_INF pArgInf = (PMDATA_INF) dwParam1;
            INT nNewVal = pArgInf->m_int;
            switch (pArgInf->m_dtDataType) {
                case SDT_INT:
                case SDT_BYTE:
                case SDT_SHORT:
                    break;
                case SDT_FLOAT:
                    nNewVal = (INT) pArgInf->m_float;
                    break;
                case SDT_INT64:
                    nNewVal = (INT) pArgInf->m_int64;
                    break;
                case SDT_DOUBLE:
                    nNewVal = (INT) pArgInf->m_double;
                    break;
                default:
                    break;
            }
            return nNewVal;
        }
        break;
        case NAS_GET_PATH:
            /* 返回当前开发或运行环境的某一类目录或文件名，目录名以“\”结束。
            dwParam1: 指定所需要的目录，可以为以下值：
            A、开发及运行环境下均有效的目录:
            1: 开发或运行环境系统所处的目录；
            B、开发环境下有效的目录(仅开发环境中有效):
            1001: 系统例程和支持库例程所在目录名
            1002: 系统工具所在目录
            1003: 系统帮助信息所在目录
            1004: 保存所有登记到系统中易模块的目录
            1005: 支持库所在的目录
            1006: 安装工具所在目录
            C、运行环境下有效的目录(仅运行环境中有效):
            2001: 用户EXE文件所处目录；
            2002: 用户EXE文件名；
            dwParam2: 接收缓冲区地址，尺寸必须为MAX_PATH。
            */
            if (::GetModuleFileNameA(NULL, AppContext->EFilePath, MAX_PATH)) {
                char *pFind = strrchr(AppContext->EFilePath, '\\');
                pFind++;
                *pFind = 0;
                pFind++;
                strcpy(AppContext->EFileName, pFind);
            } else {
                AppContext->EFilePath[0] = 0;
                AppContext->EFileName[0] = 0;
            }

            switch (dwParam1) {
                case 1:
                    strcpy((char *) dwParam2, AppContext->EFilePath);
                    return (INT) AppContext->EFilePath;
                    break;
                case 2001:
                    strcpy((char *) dwParam2, AppContext->EFilePath);
                    return (INT) AppContext->EFilePath;
                    break;
                case 2002:
                    strcpy((char *) dwParam2, AppContext->EFileName);
                    return (INT) AppContext->EFileName;
                    break;
                default:
                    return NULL;
            }
        case NAS_GET_LANG_ID:
            // 返回当前系统或运行环境所支持的语言ID，具体ID值请见lang.h
            return 1;
        case NAS_GET_VER:
            // 返回当前系统或运行环境的版本号，LOWORD为主版本号，HIWORD为次版本号。
            return 0x00000004;
        case NRS_GET_WINFORM_COUNT:
            // 返回当前程序的窗体数目。
            return 0;
        case NRS_GET_WINFORM_HWND:
            // 返回指定窗体的窗口句柄，如果该窗体尚未被载入，返回NULL。
            // dwParam1为窗体索引。
            return NULL;
        case NAS_GET_APP_ICON:
        // 通知系统创建并返回程序的图标。
        // dwParam1为PAPP_ICON指针。
        case NAS_GET_LIB_DATA_TYPE_INFO:
        // 返回指定库定义数据类型的PLIB_DATA_TYPE_INFO定义信息指针。
        // dwParam1为欲检查的数据类型。
        // 如果该数据类型无效或者不为库定义数据类型，则返回NULL，否则返回PLIB_DATA_TYPE_INFO指针。
        case NAS_GET_HBITMAP:
        // dwParam1为图片数据指针，dwParam2为图片数据尺寸。
        // 如果成功返回非NULL的HBITMAP句柄（注意使用完毕后释放），否则返回NULL。

        case NAS_CREATE_CWND_OBJECT_FROM_HWND:
        // 通过指定HWND句柄创建一个CWND对象，返回其指针，记住此指针必须通过调用NRS_DELETE_CWND_OBJECT来释放
        // dwParam1为HWND句柄
        // 成功返回CWnd*指针，失败返回NULL
        case NAS_DELETE_CWND_OBJECT:
        // 删除通过NRS_CREATE_CWND_OBJECT_FROM_HWND创建的CWND对象
        // dwParam1为欲删除的CWnd对象指针
        case NAS_DETACH_CWND_OBJECT:
        // 取消通过NRS_CREATE_CWND_OBJECT_FROM_HWND创建的CWND对象与其中HWND的绑定
        // dwParam1为CWnd对象指针
        // 成功返回HWND,失败返回0
        case NAS_GET_HWND_OF_CWND_OBJECT:
        // 获取通过NRS_CREATE_CWND_OBJECT_FROM_HWND创建的CWND对象中的HWND
        // dwParam1为CWnd对象指针
        // 成功返回HWND,失败返回0
        case NAS_ATTACH_CWND_OBJECT:
        // 将指定HWND与通过NRS_CREATE_CWND_OBJECT_FROM_HWND创建的CWND对象绑定起来
        // dwParam1为HWND
        // dwParam2为CWnd对象指针
        // 成功返回1,失败返回0
        case NAS_IS_EWIN:
        // 如果指定窗口为易语言窗口或易语言组件，返回真，否则返回假。
        // dwParam1为欲测试的HWND.
        // NRS_ 宏为仅能被易运行环境处理的通知。
        case NRS_UNIT_DESTROIED:
        // 通知系统指定的组件已经被销毁。
        // dwParam1为dwFormID
        // dwParam2为dwUnitID
        case NRS_GET_UNIT_PTR:
        // 取组件对象指针
        // dwParam1为WinForm的ID
        // dwParam2为WinUnit的ID
        // 成功返回有效的组件对象CWnd*指针，失败返回0。
        case NRS_GET_AND_CHECK_UNIT_PTR:
        // 取组件对象指针
        // dwParam1为WinForm的ID
        // dwParam2为WinUnit的ID
        // 成功返回有效的组件对象CWnd*指针，失败报告运行时错误并退出程序。
        case NRS_EVENT_NOTIFY:
        // 以第一类方式通知系统产生了事件。
        // dwParam1为PEVENT_NOTIFY指针。
        //   如果返回 0 ，表示此事件已被系统抛弃，否则表示系统已经成功传递此事件到用户
        // 事件处理子程序。
        case NRS_GET_UNIT_DATA_TYPE:
        // dwParam1为WinForm的ID
        // dwParam2为WinUnit的ID
        // 成功返回有效的 DATA_TYPE ，失败返回 0 。

        case NRS_EVENT_NOTIFY2:
        // 以第二类方式通知系统产生了事件。
        // dwParam1为PEVENT_NOTIFY2指针。
        //   如果返回 0 ，表示此事件已被系统抛弃，否则表示系统已经成功传递此事件到用户
        // 事件处理子程序。

        case NRS_GET_BITMAP_DATA:
        // 返回指定HBITMAP的图片数据，成功返回包含BMP图片数据的HGLOBAL句柄，失败返回NULL。
        // dwParam1为欲获取其图片数据的HBITMAP。
        case NRS_FREE_COMOBJECT:
        // 通知系统释放指定的DTP_COM_OBJECT类型COM对象。
        // dwParam1为该COM对象的地址指针。
        case NRS_CHK_TAB_VISIBLE:
        default: {
            char ErrorString[255];
            wsprintfA(ErrorString, "不支持系统功能函数%d. 请将此信息反馈给作者", nMsg);
            MessageBoxA(0, ErrorString, "blackmoon", MB_ICONERROR);
            break;
        }
    }
    return NULL;
}
