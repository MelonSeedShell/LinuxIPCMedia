#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <semaphore.h>

#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "Sem.h"

// 操作信号量的结构体
union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
    struct seminfo* __buf;
};

Sem::Sem(const std::string& path):m_path(path)
{
}

Sem::~Sem()
{
}

int Sem::init()
{
    int ret = 0;

    m_key = ftok(m_path.c_str(), 'b');
    if (m_key == -1) {
        perror("ftok for sem");
        return -1;
    }

    // 获取信号量标识符
    m_semId = semget(m_key, 1, 0666);
    if (m_semId == -1) {
        printf("need create sem\n");
        m_semId = semget(m_key, 1, IPC_CREAT | 0666);
        if (m_semId == -1) {
            perror("create sem failed\n");
            return -1;
        }
        m_isCreator = true;
        // 初始化信号量的值为1
        union semun arg;
        arg.val = 1;
        if (semctl(m_semId, 0, SETVAL, arg) == -1) {
            perror("semctl SETVAL failed\n");
            return -1;
        }
        
        printf("create sem, key:%d, semid:%d\n", m_key, m_semId);
    }

    return 0;
}

int Sem::deinit()
{
    if (m_semId == -1) {
        return 0;
    }
    if (!m_isCreator) {
        return 0;
    }
    // 删除信号量
    if (semctl(m_semId, 0, IPC_RMID, NULL) == -1) {
        perror("semctl IPC_RMID failed\n");
        return -1;
    }
    
    m_isCreator = false;
    return 0;
}

int Sem::wait()
{
    if (m_semId == -1) {
        perror("sem not init\n");
        return -1;
    }

    struct sembuf sops = {0, -1, 0};
    if (semop(m_semId, &sops, 1) == -1) {
        perror("wait, semop failed\n");
        return -1;
    }

    return 0;
}

int Sem::waitTmOut(const int& timeSec)
{
    if (m_semId == -1) {
        perror("sem not init\n");
        return -1;
    }

    // 设置超时时间
    struct timespec ts;
    struct sembuf sops = {0, -1, 0};
    // clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec = timeSec;
    ts.tv_nsec = 0;
    // 尝试进行带有超时的信号量等待
    int result = semtimedop(m_semId, &sops, 1, &ts);
    if (result == -1) {
        if (errno == ETIMEDOUT) {
            // printf("信号量等待超时。\n");
            return -1;
        }

        // perror("semtimedop failed\n");
        return -1;
    }

    return 0;
}
int Sem::signal()
{
    if (m_semId == -1) {
        perror("sem not init\n");
        return -1;
    }

    struct sembuf sops = {0, 1, 0};
    if (semop(m_semId, &sops, 1) == -1) {
        perror("signal, semop failed\n");
        return -1;
    }

    return 0;
}