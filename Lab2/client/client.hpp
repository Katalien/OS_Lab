#ifndef CLIENT_H
#define CLIENT_H
#include <sys/types.h>
#include <atomic>
#include <vector>
#include <queue>
#include <mutex>
#include <stdlib.h>
#include <string.h>
#include <bits/types/siginfo_t.h>
#include "../connections/connection.hpp"
#include "../proc_queue/proc_queue.hpp"

class Client {
private:
    ProcQueue<Message> inputMessages;
    ProcQueue<Message> outputMessages;
    std::atomic<pid_t> hostPid = -1;
    static Client clientInstance;
    std::atomic<bool> isRunning = true;
    std::atomic<bool> hostOk = false;

    Client(void);
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    static void SignalHandler(int signum, siginfo_t* info, void *ptr);
    void ConnectionJob(void);
    bool PrepareCon(Connection **con, sem_t **sem_read, sem_t **sem_write);
    bool ConnectionGetMessages(Connection *con, sem_t *sem_read, sem_t *sem_write);
    bool ConnectionSendMessages(Connection *con, sem_t *sem_read, sem_t *sem_write);
    void ConnectionClose(Connection *con, sem_t *sem_read, sem_t *sem_write);
    static void SendMessage(Message msg);
    static bool GetMessage(Message *msg);
    void write(Message msg);
    void processMes();

    
public:

    static Client &GetInstance(void) { return clientInstance; }
    static bool IsRunning(void) { return GetInstance().isRunning.load(); }
    void Run(pid_t host_id);
    void Stop(void);
    ~Client();
};

#endif 