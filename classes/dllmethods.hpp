#pragma once

using QS = bool (__cdecl*)(std::string, std::string&);

struct DLL_METHODS {
    QS quietShell;
};

DLL_METHODS loadDllMethods(HMODULE dll) {
    DLL_METHODS methods = {};

    methods.quietShell = reinterpret_cast<QS>(GetProcAddress(dll, "quietShell"));

    return methods;
}
