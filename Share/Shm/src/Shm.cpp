#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include "Shm.h"
Shm::Shm(const std::string& path, const unsigned long& size):m_path(path),m_size(size)
{
}

Shm::~Shm()
{
}

int Shm::init()
{
    m_key = ftok(m_path.c_str(), 'a');
    if (m_key == -1) {
        // perror("ftok for shm");
        return -1;
    }

    // 获取共享内存段标识符
    m_shmid = shmget(m_key, m_size, 0666);
    if (m_shmid == -1) {
        // printf("need create shm\n");
        m_shmid = shmget(m_key, m_size, IPC_CREAT | 0666);
        if (m_shmid == -1) {
            // perror("shm create failed\n");
            return -1;
        }
        m_isCreator = true;
        // printf("create shm, key:%d, shmid:%d\n", m_key, m_shmid);

        void* addr = NULL;
        if (mmap(&addr) < 0) {
            // printf("shm addr init failed, shm mmap failed\n");
            return -1;
        }
        if (!addr) {
            // printf("shm addr init failed, shm mmap addr is null\n");
            return -1;
        }
        char* data = (char*)addr;
        memset(data, 0, m_size);

        if (unmmap((void**)&addr) < 0) {
            // printf("shm addr init failed, shm unmmap failed\n");
            return -1;
        }
    }

    return 0;
}
int Shm::deinit()
{
    if (m_shmid == -1) {
        return 0;
    }

    if (m_mapped) {
        // perror("shm deinit failed, not unmap\n");
        return -1;
    }

    if (!m_isCreator) {
        return 0;
    }
    
    // 标记共享内存段可删除
    if (shmctl(m_shmid, IPC_RMID, NULL) == -1) {
        // perror("shm deinit failed, rm shm failed\n");
        return -1;
    }

    m_isCreator = false;

    return 0;
}

int Shm::mmap(void** addr)
{
    if (*addr) {
        // perror("addr is not null\n");
        return -1;
    }

    if (m_shmid == -1) {
        // perror("shm not init\n");
        return -1;
    }

    *addr = shmat(m_shmid, NULL, 0);
    if (*addr == (void*)-1) {
        // perror("mmap failed\n");
        *addr = nullptr;
        return -1;
    }

    if (!*addr) {
        // perror("mmap failed, addr is null\n");
        return -1;
    }
    m_mapped = true;
    return 0;
}
int Shm::unmmap(void** addr)
{
    if (!*addr) {
        // perror("addr is null\n");
        return -1;
    }

    if (m_shmid == -1) {
        // perror("shm not init\n");
        return -1;
    }

    // 将共享内存段从进程地址空间分离
    if (shmdt(*addr) == -1) {
        // perror("unmmap failed, shmdt failed\n");
        return -1;
    }
    *addr = nullptr;
    m_mapped = false;
    return 0;
}