import os
import sys
import struct

from lief import PE

os.chdir(os.path.dirname(os.path.abspath(__file__)))

class EpkReader:

    stream = None

    sections = []

    def __readbytes(self, n):
        if not self.stream == None:
            return self.stream.read(n)
        return None

    def __readuint32(self):
        if not self.stream == None:
            return struct.unpack("<I", self.stream.read(4))[0]
        return None
    
    def __readint32(self):
        if not self.stream == None:
            return struct.unpack("<i", self.stream.read(4))[0]
        return None

    def __seek(self, offset):
        if not self.stream == None:
            self.stream.seek(offset)

    def open(self, file):

        self.close()

        self.stream = open(file, "rb")

        self.__readuint32()                 # m_dwMark
        self.__readbytes(32)                # m_chMark
        self.__readuint32()                 # m_nHeaderSize
        self.__readuint32()                 # m_nVersion
        self.__readuint32()                 # m_nType
        self.__readuint32()                 # m_dwState
        self.__readuint32()                 # m_Reserved
        self.__readuint32()                 # m_nDllCmdCount

        start_code = self.__readuint32()    # m_nStartCodeOffset

        self.__readuint32()                 # m_nConstSectionOffset
        self.__readuint32()                 # m_nWinFormSectionOffset
        self.__readuint32()                 # m_nHelpFuncSectionOffset
        self.__readuint32()                 # m_nCodeSectionOffset
        self.__readuint32()                 # m_nVarSectionOffset

        begin_section = self.__readuint32() # m_nBeginSectionOffset

        next_section = begin_section

        while not next_section == 0xffffffff:
            self.__seek(next_section)

            section_offset = next_section

            self.__readuint32()                                # m_nSectionSize
            next_section = self.__readuint32()                 # m_nNextSectionOffset
            self.__readuint32()                                # m_dwState

            name = self.__readbytes(24).decode().strip('\x00') # m_szName

            loaded_size = self.__readuint32()                  # m_nLoadedSize
            record_size = self.__readuint32()                  # m_nLoadedSize
            record_offset = self.__readuint32()                # m_nRecordOffset
            
            entry = -1

            if name == "code":
                entry = start_code - record_offset

            reloc_count = self.__readuint32()                  # m_nLoadedSize
            self.__readuint32()                                # m_nExportSymbolCount

            reloc_infos = []

            for i in range(reloc_count):
                reloc_infos.append(self.__readuint32())        # m_nReLocationItemCount

            self.__seek(record_offset)

            data = self.__readbytes(record_size)               # m_szData

            self.sections.append({
                "section_offset": section_offset,
                "name": name,
                "loaded_size": loaded_size,
                "record_size": record_size,
                "record_offset": record_offset,
                "reloc_infos": reloc_infos,
                "data": data,
                "entry": entry,
            })

    def close(self):
        
        self.sections = []

        if not self.stream == None:
            self.stream.close()
            self.stream = None

epk = EpkReader()

epk.open(sys.argv[1])

output = PE.Binary(PE.PE_TYPE.PE32)

help_va = 0
const_va = 0
var_va = 0
code_va = 0

for section in epk.sections:
    
    section_types = PE.SECTION_TYPES.DATA

    if section["name"] == "code":
        section_types = PE.SECTION_TYPES.TEXT        

    section_pe = PE.Section()
    section_pe.name = "." + section["name"]
    section_pe.content = bytearray(section["data"])

    section_pe = output.add_section(section_pe, section_types)

    if not section["entry"] == -1:
        output.optional_header.addressof_entrypoint = section_pe.virtual_address + section["entry"]

    if section["name"] == "help":
        help_va = section_pe.virtual_address
    if section["name"] == "const":
        const_va = section_pe.virtual_address
    if section["name"] == "var":
        var_va = section_pe.virtual_address
    if section["name"] == "code":
        code_va = section_pe.virtual_address

# 重定位
for section_pe in output.sections:
    
    for section in epk.sections:
        if section["name"] == section_pe.name[1:]:

            content = bytearray(section_pe.content.tobytes())

            for reloc in section["reloc_infos"]:
                reloc_type = reloc & 7 # m_btType
                reloc_offset = reloc >> 3

                value = struct.unpack("<i", content[reloc_offset:reloc_offset + 4])[0]

                if (reloc_type == 0): # RT_HELP_FUNC
                    value = value + help_va + output.imagebase
                if (reloc_type == 1): # RT_CONST
                    value = value + const_va + output.imagebase
                if (reloc_type == 2): # RT_GLOBAL_VAR
                    value = value + var_va + output.imagebase
                if (reloc_type == 3): # RT_CODE
                    value = value + code_va + output.imagebase

                reloc_value = struct.pack("<i", value)

                content[reloc_offset] = reloc_value[0]
                content[reloc_offset + 1] = reloc_value[1]
                content[reloc_offset + 2] = reloc_value[2]
                content[reloc_offset + 3] = reloc_value[3]
            
            section_pe.content = content


# 修复lief的bug
output.header.sizeof_optional_header = 0xe0

builder = PE.Builder(output)
builder.build()
builder.write("output.exe")

