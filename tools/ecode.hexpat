import std.string;

struct LIB_INFO {

    if (std::mem::read_unsigned($, 1) == 0x00) {
        char end_LibInfo;
        break;
    }
    
    char m_szFile[while(std::mem::read_unsigned($, 1) != 0x0D)];
    char end_File;
    char m_szGUID[while(std::mem::read_unsigned($, 1) != 0x0D)];
    char end_GUID;
    char m_nMajorVersion[while(std::mem::read_unsigned($, 1) != 0x0D)];
    char end_MajorVersion;
    char m_nMinorVersion[while(std::mem::read_unsigned($, 1) != 0x0D)];
    char end_MinorVersion;
    char m_szName[];
};

struct APP_HEADER_INFO {
    u32      m_dwMark;
    char     m_chMark[32];
    u32      m_nHeaderSize;
    u32      m_nVersion;
    u32      m_nType;
    u32      m_dwState;
    u32      m_Reserved;
    u32      m_nDllCmdCount;
    u32      m_nStartCodeOffset;
    u32      m_nConstSectionOffset;
    u32      m_nWinFormSectionOffset;
    u32      m_nHelpFuncSectionOffset;
    u32      m_nCodeSectionOffset;
    u32      m_nVarSectionOffset;
    u32      m_nBeginSectionOffset;
    u32      m_nDllFileNameConstOffset[m_nDllCmdCount];
    u32      m_nDllCmdNameConstOffset[m_nDllCmdCount];
    LIB_INFO m_szLibString[while($ < m_nHeaderSize)];
};

struct SECTION_INFO {
    u32  m_nSectionSize;
    u32  m_nNextSectionOffset;
    u32  m_dwState;
    char m_szName[24];
    u32  m_nLoadedSize;
    u32  m_nRecordSize;
    u32  m_nRecordOffset;
    u32  m_nReLocationItemCount;
    u32  m_nExportSymbolCount;
    u32  m_aryReLocationItem[m_nReLocationItemCount];
    u32  m_arySymbolDataOffset[m_nExportSymbolCount];
    u32  m_szarySymbolNameOffset[m_nExportSymbolCount];
    char m_szData[m_nRecordSize];
    
    if (m_nNextSectionOffset == 0xffffffff)
        break;
};

struct EAPP {
    APP_HEADER_INFO header;
    SECTION_INFO    sections[while(true)];
};

EAPP app @ 0x00;