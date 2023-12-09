#ifndef __HOST_H_
#define __HOST_H_

#include <stdlib.h>
#include <string.h>
#include <mutex>
#include <atomic>
#include <vector>
#include <queue>
#include <sys/types.h>
#include <bits/types/siginfo_t.h>
#include "../connections/connection.hpp"
#include "../proc_queue/proc_queue.hpp"

class Host {
private:

    ProcQueue<Message> inputMessages;
    ProcQueue<Message> outputMessages;
    std::atomic<pid_t> clientPid = -1;
    static Host hostInstance;
    std::atomic<bool> m_isRunning = true;

    Host(void);
    Host(const Host&) = delete;
    Host& operator=(const Host&) = delete;

    static void SignalHandler(int signum, siginfo_t* info, void *ptr);
    static void SendMessage(Message msg);
    static bool GetMessage(Message *msg);
    void ConnectionJob(void);
    bool PrepareCon(Connection **con, sem_t **sem_read, sem_t **sem_write);
    bool ConnectionGetMessages(Connection *con, sem_t *sem_read, sem_t *sem_write);
    bool ConnectionSendMessages(Connection *con, sem_t *sem_read, sem_t *sem_write);
    void ConnectionClose(Connection *con, sem_t *sem_read, sem_t *sem_write);
    void write(Message msg);
    void processMes();
    

public:

    static Host &GetInstance(void) { return hostInstance; }
    void Run(void);
    static bool IsRunning(void) { return GetInstance().m_isRunning.load(); }
    static pid_t GetClientPid(void) { return GetInstance().clientPid; }
    void Stop(void);
    ~Host();
};

#endif 