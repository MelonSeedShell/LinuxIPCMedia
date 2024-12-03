#pragma once
#include <string>

class Shm
{

public:
    Shm(const std::string& path, const unsigned long& size);
    ~Shm();

    int init();
    int deinit();

    int mmap(void** addr);
    int unmmap(void** addr);

private:
    std::string m_path;
    unsigned long m_size = 0;
    bool m_mapped = false;

    bool m_isCreator = false;
    int m_key = -1;
    int m_shmid = -1;
};

