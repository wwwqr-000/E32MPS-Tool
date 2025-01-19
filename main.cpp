#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include <ctime>

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
        if (std::isalnum(c) || c == '\\' || c == '/' || c == '_' || c == ' ' || c == '.' || c == '-' || c == '(' || c == ')' || c == '+' || c == '=' || c == ',' || c == ';' || c == '\'' || c == '!' || c == '$' || c == '&' || c == '@' || c == '^' || c == '~' || c == '[' || c == ']' || c == '{' || c == '}' || c == ':' || c == '\n') {
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

bool sendCommandUsingFile(std::string& cmd, const std::string& port, std::string& buff) {
    std::ofstream file("buff.cache");
    if (!file.is_open()) {
        return false;
    }

    file << cmd << "\n";
    file.close();
    dllMethods.quietShell(("for /f \"usebackq delims=\" %a in (buff.cache) do @echo %a > COM" + port).c_str(), buff);
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

    std::string cmd = "f = open('E32MPS.txt', 'w'); f.write('live file sync by E32MPS-Tool (https://github.com/wwwqr-000/E32MPS-Tool)'); f.close();";
    if (!sendCmd(cmd, serPort, buff)) {
        cls();
        std::cout << "Could not finish test command on COM" << serPort << ": " << buff << "\n\nPress enter to exit\n";
        wait();
        return 2;
    }

    cls();
    std::cout << "Connected to device trough COM" << serPort << ".\n\n";

    while (true) {
        dllMethods.quietShell("cd workspace && powershell \"Get-ChildItem -Recurse | ForEach-Object { $_.FullName.Substring($pwd.Path.Length + 1) }\"", buff);
        createUsefulBuff(buff);
        if (buff == "File Not Found") {
            system("mkdir workspace");
            continue;
        }

        std::string lines = buff + "\n";// \n is added, to recieve the last item from the buff
        buff = "";

        std::string tmpLine = "";
        std::vector<std::string> files;
        for (char& c : lines) {
            if (c == '\n') {
                files.push_back(tmpLine);
                tmpLine = "";
                continue;
            }
            if (c == '\\') {
                c = '/';
            }
            tmpLine += c;
        }

        for (std::string& filePath : files) {
            std::string realFilePath = "./workspace/" + filePath;
            std::string fileContents = "";
            bool isFolder = false;
            tmpLine = "";
            std::ifstream file(realFilePath);
            if (!file.is_open()) {
                bool invertedFolderCheck = true;
                //Check if 'file' is folder
                for (char& c : filePath) {
                    if (c == '.') {
                        invertedFolderCheck = false;
                        break;
                    }
                }
                if (invertedFolderCheck) {
                    //std::cout << filePath << " is a folder...\n";
                    isFolder = true;
                }
                else {
                    std::cout << "Could not open file " << realFilePath << ". Skipping...\n";
                    continue;
                }
                //
            }

            std::string cmd = "";

            if (!isFolder) {//Get content out of file
                while (std::getline(file, tmpLine)) {
                    fileContents += tmpLine + "\n";
                }

                //W.I.P (format fileContents string so \n is \\n and \t is \\t, etc...)

                cmd = "f = open('" + filePath + "', 'w'); f.write('" + fileContents + "'); f.close();";
            }
            else {
                cmd = "import uos; uos.mkdir('" + filePath + "')";
            }

            std::cout << cmd << "\n";

            if (!sendCommandUsingFile(cmd, serPort, buff)) {
                cls();
                std::cout << "Could not send command on COM" << serPort << ": " << buff << "\n\nPress enter to exit\n";
                wait();
                return 2;
            }

            file.close();
        }
        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);
        //std::cout << "Last updated: " << (localTime->tm_hour < 10 ? "0" : "") << localTime->tm_hour << ":" << (localTime->tm_min < 10 ? "0" : "") << localTime->tm_min << ":" << (localTime->tm_sec < 10 ? "0" : "") << localTime->tm_sec << "\n";
    }
}
