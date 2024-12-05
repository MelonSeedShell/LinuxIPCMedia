#pragma once
#include <string>
class Sem
{

public:
    Sem(const std::string& path);
    ~Sem();
    int init();
    int deinit();
    int wait();
    int waitTmOut(const int& timeMS);
    int signal();

private:
    /* data */
    std::string m_path;
    bool m_isCreator = false;
    int m_key = -1;
    int m_semId = -1;
};



