#ifndef __SAFE_QUEUE_H_
#define __SAFE_QUEUE_H_

#include <queue>
#include <mutex>
#include "../connections/connection.hpp"

struct Message {
    char m_message[100];
};

template <typename T>
class ProcQueue {
private:

    std::queue<T> storage;
    mutable std::mutex m_mutex;
	
public:
	
	bool GetFromConnection(Connection *con) {
		m_mutex.lock();
		uint32_t amount = 0;
		con->Get(&amount, sizeof(uint32_t));
		for (uint32_t i = 0; i < amount; i++){
			T msg = {0};
			try{
				con->Get(&msg, sizeof(T));
			}
			catch (const char *err){
				m_mutex.unlock();
				syslog(LOG_ERR, "%s", err);
				return false;
			}
			storage.push(msg);
		}
		m_mutex.unlock();
		return true;
	}
	
	bool SendToConnection(Connection *con){
		m_mutex.lock();
		uint32_t amount = storage.size();
		con->Send(&amount, sizeof(uint32_t));
		while (!storage.empty()){
			try{
				con->Send(&storage.front(), sizeof(T));
			}
			catch (const char *err){
				m_mutex.unlock();
				syslog(LOG_ERR, "%s", err);
				return false;
			}
			storage.pop();
		}
		m_mutex.unlock();
		return true;
	}

	void Push(const T &val) {
		m_mutex.lock();
		storage.push(val);
		m_mutex.unlock();
	}
	
	bool GetAndRemove(T *msg) {
		m_mutex.lock();
		if (storage.empty()) {
			m_mutex.unlock();
			return false;
		}
		*msg = storage.front();
		storage.pop();
		m_mutex.unlock();
		return true;
	}
};

#endif 