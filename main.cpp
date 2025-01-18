#include <iostream>
#include <windows.h>
#include <fstream>

#include "classes/resource.h"
#include "classes/dllmethods.hpp"

DLL_METHODS dllMethods;

void setTmpPath(std::string& path) {
    char buff[MAX_PATH];
    GetTempPath(MAX_PATH, buff);
    path = std::string(buff);
}

void createUsefulBuff(std::string& buff) {
    std::string finalBuff = "";
    for (char& c : buff) {
        if (std::isalnum(c) || c == '\\' || c == '/' || c == '_' || c == ' ' || c == '.' || c == '-' || c == '(' || c == ')' || c == '+' || c == '=' || c == ',' || c == ';' || c == '\'' || c == '!' || c == '$' || c == '&' || c == '@' || c == '^' || c == '~' || c == '[' || c == ']' || c == '{' || c == '}' || c == ':') {
            finalBuff += c;
        }
    }
    buff = finalBuff;
}

int unpackRCResource(int definedVar, std::string name, std::string dropLocation) {
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(definedVar), RT_RCDATA);
    if (!hRes) {
        return 1;
    }
    HGLOBAL hLoadRes = LoadResource(NULL, hRes);
    DWORD resSize = SizeofResource(NULL, hRes);
    void* pResData = LockResource(hLoadRes);
    if (!pResData || resSize == 0) {
        return 2;
    }
    std::ofstream tmpFile(dropLocation + name, std::ios::binary);
    tmpFile.write(reinterpret_cast<const char*>(pResData), resSize);
    tmpFile.close();
    return 0;
}

void wait() { system("set /p end="); }
void cls() { system("cls"); }

bool sendCmd(const std::string& cmd, const std::string& port, std::string& buff) {
    dllMethods.quietShell(("echo " + cmd + " > COM" + port).c_str(), buff);
    createUsefulBuff(buff);
    if (buff == "") { return true; }
    return false;
}

int main() {
    //echo f = open('test.txt', 'w'); f.write('test123'); f.close(); > COMx
    std::string dllName = "whiteavocado64.dll";
    std::string tmpPath = "";
    std::string buff = "";

    setTmpPath(tmpPath);
    if (unpackRCResource(ASSET_WHITEAVOCADO_DLL, dllName, tmpPath) != 0) {
        std::cout << "Could not unpack resources. Press enter to exit\n";
        wait();
        return 1;
    }

    HMODULE const DLL = LoadLibraryExA((tmpPath + dllName).c_str(), nullptr, 0);
    dllMethods = loadDllMethods(DLL);

    std::string serPort = "";
    std::cout << "What is the number of the COM port you want to connect with? (Example input: \"7\" for COM7)\n> ";
    std::cin >> serPort;
    std::cout << "\nTrying to connect with COM" << serPort << "...\n";

    std::string cmd = "f = open('E32MPS.txt', 'w'); f.write('createde'); f.close();";
    if (!sendCmd(cmd, serPort, buff)) {
        cls();
        std::cout << "Could not finish test command on COM" << serPort << ": " << buff << "\n\nPress enter to exit\n";
        wait();
        return 2;
    }

    cls();
    std::cout << "Connected to device trough COM" << serPort << ".\n\n";

    while (true) {

    }
}
