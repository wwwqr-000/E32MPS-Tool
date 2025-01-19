#pragma once

class deletedFiles {
    private:
        bool stat;
        std::vector<std::string> fileStrs;

    public:
        deletedFiles(bool stat, std::vector<std::string> fileStrs): stat(stat), fileStrs(fileStrs) {}

        bool deleted() { return this->stat; }
        std::vector<std::string>& files() { return this->fileStrs; }
};
