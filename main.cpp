#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include <ctime>
#include <string>

#include "classes/resource.h"
#include "classes/dllmethods.hpp"
#include "classes/deletedFiles.hpp"

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

bool sendCommandUsingFile(std::string& cmd, const std::string& port, std::string& buff, std::string targetFile = "") {
    std::ofstream file("buff.cache");
    if (!file.is_open()) {
        return false;
    }

    if (targetFile != "") {//Cmd becomes the contents of the file.
        std::vector<std::string> lines;
        int num = -1;
        int index = 0;
        bool firstTime = true;
        std::string newLine = "";
        for (char& c : cmd) {
            ++num;
            if (num % 5000 == 0 && !firstTime) {
                ++index;
                lines.push_back(newLine);
            }

            if (firstTime) {
                lines.push_back(newLine);
                firstTime = false;
            }

            lines[index] += c;
        }

        num = 0;
        index = 0;
        for (std::string l : lines) {
            ++num;
            file << "b" << num << " = '" << l << "'\n";
        }

        file << "f = open('" + targetFile + "', 'w'); f.write(";

        std::string delStr = "";
        for (int x = 0; x < num; x++) {
            if (x != 0) {
                file << " ";
                delStr += " ";
            }
            file << "b" << (x + 1);
            delStr += "del b" + std::to_string(x + 1) + ";";
            if (x != (num - 1)) {
                file << " +";
            }
        }

        file << "); f.close(); " << delStr;
        file.close();
    }
    else {
        file << cmd << "\n";
        file.close();
    }

    dllMethods.quietShell(("for /f \"usebackq delims=\" %a in (buff.cache) do @echo %a > COM" + port).c_str(), buff);
    createUsefulBuff(buff);
    if (buff == "") { return true; }
    return false;
}

void replaceStr(std::string &str, std::string oldSubstring, std::string newSubstring) {
    size_t pos = 0;
    while ((pos = str.find(oldSubstring, pos)) != std::string::npos) {
        str.replace(pos, oldSubstring.length(), newSubstring);
        pos += newSubstring.length();
    }
}

bool fileInList(const std::string& file, const std::vector<std::string>& vecList) {
    for (std::string s : vecList) {
        if (s == file) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> getMissingFiles(const std::vector<std::string>& files, const std::vector<std::string>& prevFiles) {
    std::vector<std::string> deletedFilesVec;

    for (int y = 0; y < prevFiles.size(); y++) {//For the previous vector, containing the missing files
        bool found = false;
        for (int x = 0; x < files.size(); x++) {//For the vector of files missing files
            if (prevFiles[y] == files[x]) {
                found = true;
                break;
            }
        }
        if (!found) {
            deletedFilesVec.push_back(prevFiles[y]);
        }
    }

    return deletedFilesVec;
}

deletedFiles filesGotDeleted(const std::vector<std::string>& files, const std::vector<std::string>& prevFiles) {
    std::vector<std::string> deletedFilesVec = getMissingFiles(files, prevFiles);

    if (prevFiles.size() < files.size()) {//A file got added
        return deletedFiles(false, deletedFilesVec);
    }

    return deletedFiles(deletedFilesVec.size() > 0, deletedFilesVec);
}

void theGreatEscape(std::string& content) {
    replaceStr(content, "\\n", "\\\n");
    replaceStr(content, "\\t", "\\\t");
    replaceStr(content, "\\r", "\\\r");
    replaceStr(content, "\\b", "\\\b");
    replaceStr(content, "\\f", "\\\f");
    replaceStr(content, "\\v", "\\\v");
    replaceStr(content, R"(\\x)", R"(\\\x)");
    replaceStr(content, "\\\\", "\\\\\\");
    replaceStr(content, "\\\"", "\\\\\"");
    replaceStr(content, "\\\'", "\\\\\'");
    replaceStr(content, "\n", "\\n");
    replaceStr(content, "\t", "\\t");
    replaceStr(content, "\r", "\\r");
    replaceStr(content, "\b", "\\b");
    replaceStr(content, "\f", "\\f");
    replaceStr(content, "\v", "\\v");
    replaceStr(content, R"(\x)", R"(\\x)");
    replaceStr(content, "\'", "\\'");
    replaceStr(content, "\"", "\\\"");
}

void couldNotSendToCOM(const std::string& serPort, const std::string& buff) {
    cls();
    std::cout << "Could not send command on COM" << serPort << ": " << buff << "\n\nPress enter to exit\n";
    wait();
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

    std::vector<std::string> prevFiles;//Files from prev loop
    std::string cacheBuff = "";
    dllMethods.quietShell("mkdir workspace", cacheBuff);
    cacheBuff = "";

    while (true) {
        dllMethods.quietShell("cd workspace && powershell \"Get-ChildItem -Recurse | ForEach-Object { $_.FullName.Substring($pwd.Path.Length + 1) }\"", buff);
        createUsefulBuff(buff);

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

        //Check for deleted files or folders
        deletedFiles delFiles = filesGotDeleted(files, prevFiles);
        if (delFiles.deleted()) {
            //Send command to delete files on esp32
            //std::cout << delFiles.files()[0] << "\n";
            for (std::string f : delFiles.files()) {
                theGreatEscape(f);
                std::string delCmd = "import uos; uos.remove('" + f + "');";
                //std::cout << delCmd << "\n";
                if (!sendCommandUsingFile(delCmd, serPort, buff)) {
                    couldNotSendToCOM(serPort, buff);
                    return 3;
                }
            }
            //
            //std::cout << "Files got deleted...\n";
        }
        //

        prevFiles = files;

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

            std::string cmdTargetFile = "";
            std::string cmd = "";

            if (!isFolder) {//Get content out of file
                while (std::getline(file, tmpLine)) {
                    fileContents += tmpLine + "\n";
                }

                //Escape escaping
                theGreatEscape(fileContents);
                //

                cmdTargetFile = filePath;
                cmd = fileContents;
            }
            else {
                if (filePath == "") { continue; }
                cmd = "import uos; uos.mkdir('" + filePath + "')";
            }

            //std::cout << cmd << "\n";

            //Timeout to give esp32 time to write the file
            size_t fileSize = strlen(fileContents.c_str());
            if (fileSize > 3000) {
                Sleep(3500);
            }
            else {
                Sleep(fileSize / 10);
            }
            //

            if (!sendCommandUsingFile(cmd, serPort, buff, cmdTargetFile)) {
                couldNotSendToCOM(serPort, buff);
                return 4;
            }

            file.close();
        }
        dllMethods.quietShell("del buff.cache", buff);
        buff = "";
        cacheBuff = "";
        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);
        std::cout << "Last updated: " << (localTime->tm_hour < 10 ? "0" : "") << localTime->tm_hour << ":" << (localTime->tm_min < 10 ? "0" : "") << localTime->tm_min << ":" << (localTime->tm_sec < 10 ? "0" : "") << localTime->tm_sec << "\n";
    }
}
