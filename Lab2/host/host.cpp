#include <sys/syslog.h>
#include <thread>
#include <algorithm>
#include <csignal>
#include <chrono>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <iostream>
#include <ostream>
#include <future>
#include "host.hpp"

Host Host::hostInstance;

int main(int argc, char *argv[]) {
  openlog("host_log", LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);
  try {
    Host::GetInstance().Run();
  }
  catch (std::exception &err) {
    syslog(LOG_ERR, "An err %s", err.what());
	closelog();
	return 1;
  }
  closelog();
  return 0;
}

void Host::SignalHandler(int signum, siginfo_t *info, void *ptr) {
  switch (signum) {
    case SIGUSR1: {
      syslog(LOG_INFO, "Client %d request connection to host", info->si_pid);
      if (Host::GetInstance().clientPid == -1){
        Host::GetInstance().clientPid = info->si_pid;
      }
      else{
        syslog(LOG_INFO, "There is another client %d", Host::GetInstance().GetClientPid());
      }
      break;
    }
    case SIGTERM: {
      Host::GetInstance().Stop();
      break;
    }
  }
}

Host::Host(void) {
  struct sigaction sig{};
  memset(&sig, 0, sizeof(sig));
  sig.sa_sigaction = SignalHandler;
  sig.sa_flags = SA_SIGINFO;
  sigaction(SIGTERM, &sig, nullptr);
  sigaction(SIGUSR1, &sig, nullptr);
}

void Host::Run(void) {
  syslog(LOG_INFO, "Host started");
  m_isRunning = true;
  std::thread connectionThread(&Host::ConnectionJob, this);
  while (m_isRunning) {
        processMes();
    }
  connectionThread.join();
}

void Host::SendMessage(Message msg){
  Host::GetInstance().outputMessages.Push(msg);
}

bool Host::GetMessage(Message *msg) {
  return Host::GetInstance().inputMessages.GetAndRemove(msg);
}

static std::string getMessage() {    
    std::string answer;
    std::cin >> std::ws;
    std::getline(std::cin, answer);
    return answer;
}

void Host::processMes(){
  std::future<std::string> future = std::async(getMessage);
        std::string answer;
        Message msg = {0};
        std::string ans = future.get();
        strncpy(msg.m_message, ans.c_str(), ans.size());
        write(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void Host::write(Message msg) {
    Host::GetInstance().outputMessages.Push(msg);
}

void Host::Stop(){
  if (m_isRunning.load()) {
    m_isRunning = false;
    syslog(LOG_INFO, "Start terminating host");
  }
}

Host::~Host(void){}

bool Host::PrepareCon(Connection **con, sem_t **sem_read, sem_t **sem_write){
  syslog(LOG_INFO, "Start connect to the client %d", GetClientPid());
  *con = Connection::CreateConnection(clientPid, true);
  if (!*con) {
    syslog(LOG_ERR, "Error while creating the connection");
    clientPid = -1;
    return false;
  }
  std::string semNameRead = "/host_" + std::to_string(clientPid);
  std::string semNameWrite = "/client_" + std::to_string(clientPid);
  *sem_read = sem_open(semNameRead.c_str(), O_CREAT | O_EXCL, 0777, 0);
  if (*sem_read == SEM_FAILED){
    syslog(LOG_ERR, "Semaphore creation error");
    delete *con;
    clientPid = -1;
    return false;
  }
  syslog(LOG_INFO, "created semaphore %s", semNameRead.c_str());
  *sem_write = sem_open(semNameWrite.c_str(), O_CREAT | O_EXCL, 0777, 0);
  if (*sem_write == SEM_FAILED){
    syslog(LOG_ERR, "Semaphore creation error");
    sem_close(*sem_read);
    delete *con;
    clientPid = -1;
    return false;
  }
  syslog(LOG_INFO, "created semaphore %s", semNameWrite.c_str());
  if (kill(clientPid, SIGUSR1) != 0)
    syslog(LOG_ERR, "Cannot send signal to %d", GetClientPid());

  syslog(LOG_INFO, "Signal sent");
  try {  
    (*con)->Open(0, true);
  }
  catch (const char *err) {
    syslog(LOG_ERR, "Connection open error: %s", err);
    delete *con;
    sem_close(*sem_write);
    sem_close(*sem_read);
    return false;
  }
  syslog(LOG_INFO, "Opened connection!\n\n");
  syslog(LOG_INFO, "You can start chatting:\n");
  return true;

}

bool Host::ConnectionGetMessages(Connection *con, sem_t *sem_read, sem_t *sem_write){
  {
    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec += 15;
    int s = sem_timedwait(sem_read, &t);
    if (s == -1) {
      syslog(LOG_ERR, "Read semaphore timeout");
      m_isRunning = false;
      return false;
    }
  }

  inputMessages.GetFromConnection(con);
  Message inMsg;
  if (GetMessage(&inMsg)){
    std::cout << "Client: " << inMsg.m_message << std::endl;
  }
  return true;
}

bool Host::ConnectionSendMessages(Connection *con, sem_t *sem_read, sem_t *sem_write) {
  bool res = outputMessages.SendToConnection(con);
  sem_post(sem_write);
  return res;
}

void Host::ConnectionClose(Connection *con, sem_t *sem_read, sem_t *sem_write){
  con->Close();
  syslog(LOG_INFO, "Connection was closed");
  sem_close(sem_write);
  sem_close(sem_read);
  system("pkill gnome-terminal");
  delete con;
}

void Host::ConnectionJob(){
  try {
    printf("host pid = %i\n", getpid());
    auto lastTimeWeHadClient = std::chrono::high_resolution_clock::now();
    while (m_isRunning.load()) {
      if (clientPid.load() == -1) {
        auto minutesPassed = std::chrono::duration_cast<std::chrono::minutes>(
          std::chrono::high_resolution_clock::now() - lastTimeWeHadClient).count();

        if (minutesPassed >= 1) {
          syslog(LOG_INFO, "STOP");
          Stop();
        }
        continue;
      }
      lastTimeWeHadClient = std::chrono::high_resolution_clock::now();

      Connection *currentConnection;
      sem_t *semaphoreRead, *semaphoreWrite;
      if (!PrepareCon(&currentConnection, &semaphoreRead, &semaphoreWrite))
        continue;

      auto clock = std::chrono::high_resolution_clock::now();
      while (m_isRunning.load()) {
        double minutes_passed =
          std::chrono::duration_cast<std::chrono::minutes>(
            std::chrono::high_resolution_clock::now() - clock).count();

        if (minutes_passed >= 1) {
          syslog(LOG_INFO, "Terminate client for too long lack of action...");
          kill(clientPid, SIGTERM);
          clientPid = -1;
          break;
        }
        if (!ConnectionGetMessages(currentConnection, semaphoreRead, semaphoreWrite))
          break;
        if (!ConnectionSendMessages(currentConnection, semaphoreRead, semaphoreWrite))
          break;

        std::this_thread::sleep_for(std::chrono::milliseconds(30)); 
      }
      ConnectionClose(currentConnection, semaphoreRead, semaphoreWrite);
    }

    if (clientPid != -1)
      kill(clientPid, SIGTERM);
  }
  catch (std::exception &e){
    syslog(LOG_ERR, "An error %s", e.what());
  }
  catch (const char *e) {
    syslog(LOG_ERR, "An error %s", e);
  }
}

