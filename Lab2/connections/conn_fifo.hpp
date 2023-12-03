#ifndef __CONN_FIFO_H_
#define __CONN_FIFO_H_

#include <string>

#include "connection.hpp"

class fifo : public Connection {
public:
    fifo(pid_t clientPid, bool isHost);
    std::string GetName(void) const { return m_fifoName; }
    void Open(size_t hostPid, bool isCreator) override;
    void Get(void *buf, size_t count) override;
    void Send(void *buf, size_t count) override;
    void Close(void) override;

private:
    std::string m_fifoName;
    bool m_isHost;
    int m_file;
};

#endif 