#include "hook.h"

#include <elf.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <sys/mman.h>
#include <map>
#include "log.h"

extern "C" {
#include <hybris/dlfcn.h>
#include "../../libs/hybris/src/jb/linker.h"
}

struct soinfo_hookinfo {
    struct hook_section {
        Elf32_Off addr, size;
    };
    std::vector<hook_section> hookSections;
};
std::map<soinfo*, soinfo_hookinfo> hookLibraries;

void addHookLibrary(void* ptr, std::string const& fileName) {
    soinfo* lib = (soinfo*) ptr;

    if (hookLibraries.count(lib) > 0)
        return;

    Elf32_Ehdr header;
    FILE* file = fopen(fileName.c_str(), "r");
    if (file == nullptr) {
        Log::error("Hook", "addHookLibrary: failed to open file");
        return;
    }
    if (fread(&header, sizeof(Elf32_Ehdr), 1, file) != 1) {
        Log::error("Hook", "addHookLibrary: failed to read header");
        fclose(file);
        return;
    }

    fseek(file, (long) header.e_shoff, SEEK_SET);

    char shdr[header.e_shentsize * header.e_shnum];
    if (fread(&shdr, header.e_shentsize, header.e_shnum, file) != header.e_shnum) {
        Log::error("Hook", "addHookLibrary: failed to read shdr");
        fclose(file);
        return;
    }

    // find strtab
    char* strtab = nullptr;
    for (int i = 0; i < header.e_shnum; i++) {
        Elf32_Shdr& entry = *((Elf32_Shdr*) &shdr[header.e_shentsize * i]);
        if (entry.sh_type == SHT_STRTAB) {
            strtab = new char[entry.sh_size];
            fseek(file, (long) entry.sh_offset, SEEK_SET);
            if (fread(strtab, 1, entry.sh_size, file) != entry.sh_size) {
                Log::error("Hook", "addHookLibrary: failed to read strtab");
                fclose(file);
                delete[] strtab;
                return;
            }
        }
    }
    if (strtab == nullptr) {
        Log::error("Hook", "addHookLibrary: couldn't find strtab");
        fclose(file);
        return;
    }
    soinfo_hookinfo hi;
    for (int i = 0; i < header.e_shnum; i++) {
        Elf32_Shdr& entry = *((Elf32_Shdr*) &shdr[header.e_shentsize * i]);
        char* entryName = &strtab[entry.sh_name];
        if (strcmp(entryName, ".got") == 0 || strcmp(entryName, ".got.plt") == 0 ||
                strcmp(entryName, ".data.rel.ro") == 0) {
            hi.hookSections.push_back({entry.sh_addr, entry.sh_size});
        }
    }
    hookLibraries[lib] = hi;
    fclose(file);
    delete[] strtab;
}

inline bool patchSection(Elf32_Addr base, Elf32_Word off, Elf32_Word size, void* sym, void* override) {
    bool foundEntry = false;
    unsigned long addr = base + off + 4;
    while (addr < base + off + size) {
        if (*((void**) addr) == sym) {
            *((void**) addr) = override;
            foundEntry = true;
        }
        addr += sizeof(void*);
    }
    return foundEntry;
}
bool patchLibrary(void* lib, void* sym, void* override) {
    soinfo* si = (soinfo*) lib;
    if (si == nullptr || hookLibraries.count(si) <= 0)
        return false;
    soinfo_hookinfo& hi = hookLibraries.at(si);
    bool foundEntry = false;
    for (auto& se : hi.hookSections) {
        if (patchSection(si->base, se.addr, se.size, sym, override))
            foundEntry = true;
    }
    return foundEntry;
}

int hookFunction(void* symbol, void* hook, void** original) {
    *original = symbol;
    bool foundEntry = false;
    for (auto& handle : hookLibraries)
        if (patchLibrary(handle.first, symbol, hook))
            foundEntry = true;

    if (!foundEntry) {
        Log::error("Hook", "Failed to hook a symbol (%llu)", (long long int) symbol);
        return -1;
    }

    return 0;
}