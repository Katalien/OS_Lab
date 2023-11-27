#include <sys/syslog.h>
#include <thread>
#include <algorithm>
#include <csignal>
#include <chrono>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <future>
#include <iostream>
#include "client.hpp"
#include <fcntl.h>
#include <cstring>
#include <ostream>
#include <fstream>

Client Client::clientInstance;

int main(int argc, char *argv[]){
  openlog("client_log", LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);
  if (argc != 2) {
      syslog(LOG_ERR, "Ooooops. You need to enter host pid");
      closelog();
      return 1;
  }
  int pid;
  try {
      pid = std::stoi(argv[1]);
  }
  catch (std::exception &e) {
      syslog(LOG_ERR, "An error while reading pid %s", e.what());
      closelog();
      return 1;
  }
  try {
    Client::GetInstance().Run(pid);
  }
  catch (std::exception &e) {
    syslog(LOG_ERR, "An error %s", e.what());	
	closelog();
	return 1;
  }
  closelog();
  return 0;
}

Client::Client(void) {
  struct sigaction sig{};
  memset(&sig, 0, sizeof(sig));
  sig.sa_sigaction = SignalHandler;
  sig.sa_flags = SA_SIGINFO;
  sigaction(SIGTERM, &sig, nullptr);
  sigaction(SIGUSR1, &sig, nullptr);
}

void Client::SignalHandler(int signum, siginfo_t *info, void *ptr){
  switch (signum){
    case SIGUSR1:
    {
      syslog(LOG_INFO, "Got signal that host are ready");
      Client::GetInstance().hostOk = true;
      break;
    }
    case SIGTERM:
    {
      Client::GetInstance().Stop();
      break;
    }
  }
}

void Client::SendMessage(Message msg){
  Client::GetInstance().outputMessages.Push(msg);
}

static std::string getMessage() {    
    std::string answer;
    std::cin >> std::ws;
    std::getline(std::cin, answer);
    return answer;
}

bool Client::GetMessage(Message *msg){
  return Client::GetInstance().inputMessages.GetAndRemove(msg);
}

void Client::write(Message msg) {
    Client::GetInstance().outputMessages.Push(msg);
}

void Client::processMes(){
  std::future<std::string> future = std::async(getMessage);
  std::string answer;
  Message msg = {0};
  std::string ans = future.get();
  strncpy(msg.m_message, ans.c_str(), ans.size());
  write(msg);
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void Client::Run(pid_t hostPid){
  syslog(LOG_INFO, "Client started");
  hostPid = hostPid;
  isRunning = true;
  if (kill(hostPid, SIGUSR1) != 0){
    isRunning = false;
    syslog(LOG_ERR, "Host %d not present", hostPid);
    return;
  }
  std::thread connectionThread(&Client::ConnectionJob, this);
  while (isRunning.load()) {
        processMes();
    }
  connectionThread.join();
}

void Client::Stop(){
  if (isRunning){
    isRunning = false;
    syslog(LOG_INFO, "Start terminating client");
  }
}

Client::~Client(void){}

bool Client::PrepareCon(Connection **con, sem_t **sem_read, sem_t **sem_write) {
  syslog(LOG_INFO, "Prepare connection for %d", int(hostPid));
  *con = Connection::CreateConnection(getpid(), false);
  if (!*con){
    syslog(LOG_ERR, "Connection creation error. Creation failed\n");
    return false;
  }

  std::string semNameRead = "/client_" + std::to_string(getpid());
  std::string semNameWrite = "/host_" + std::to_string(getpid());
  *sem_read = sem_open(semNameRead.c_str(), 0);
  if (*sem_read == SEM_FAILED){
    syslog(LOG_ERR, "Semaphore creation error. Error sem_read\n");
    delete *con;
    return false;
  }
  syslog(LOG_INFO, "created semaphore %s", semNameRead.c_str());
  *sem_write = sem_open(semNameWrite.c_str(), 0);
  if (*sem_write == SEM_FAILED){
    syslog(LOG_ERR, "Semaphore creation error. Error sem_write\n");
    sem_close(*sem_read);
    delete *con;
    return false;
  }
  syslog(LOG_INFO, "Semaphore was created.  %s", semNameWrite.c_str());
  try{
    (*con)->Open(hostPid, false);
  }
  catch (const char *err){
    syslog(LOG_ERR, "Error while connection open: %s", err);
    delete *con;
    sem_close(*sem_write);
    sem_close(*sem_read);
    return false;
  }
  syslog(LOG_INFO, "Your connection was opened successfully!\n\n");
  syslog(LOG_INFO, "You can start chatting:\n");
  return true;
}

bool Client::ConnectionGetMessages(Connection *con, sem_t *sem_read, sem_t *sem_write){
  {
    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec += 5;
    int s = sem_timedwait(sem_read, &t);
    if (s == -1){
      syslog(LOG_ERR, "Read semaphore timeout");
      isRunning = false;
      return false;
    }
  }
  inputMessages.GetFromConnection(con);
  Message inMessage;
  if (GetMessage(&inMessage))
    std::cout << "Host: " << inMessage.m_message << std::endl;
  return true;
}

bool Client::ConnectionSendMessages(Connection *con, sem_t *sem_read, sem_t *sem_write){
  bool flag = outputMessages.SendToConnection(con);
  sem_post(sem_write);
  return flag;
}

void Client::ConnectionClose(Connection *con, sem_t *sem_read, sem_t *sem_write){
  con->Close();
  syslog(LOG_INFO, "Close connection");
  sem_close(sem_write);
  sem_close(sem_read);
  system("pkill gnome-terminal");
  delete con;
}

void Client::ConnectionJob(void){
  try{
    auto clock = std::chrono::high_resolution_clock::now();

    while (!hostOk.load()){
      double second = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::high_resolution_clock::now() - clock).count();

      if (second >= 5){
        hostOk = false;
        hostPid = false;
        syslog(LOG_ERR, "Host cannot prepare for 5 seconds. Exiting");
        Stop();
        return;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    Connection *currentConnection;
    sem_t *semaphoreWrite, *semaphoreRead;
    if (!PrepareCon(&currentConnection, &semaphoreRead, &semaphoreWrite))
      return;
 
    while (isRunning.load()){
      if (!ConnectionSendMessages(currentConnection, semaphoreRead, semaphoreWrite))
        break;

      std::this_thread::sleep_for(std::chrono::milliseconds(30));

      if (!ConnectionGetMessages(currentConnection, semaphoreRead, semaphoreWrite))
        break;
    }
    ConnectionClose(currentConnection, semaphoreRead, semaphoreWrite);
  }
  catch (std::exception &e){
    syslog(LOG_ERR, "An error %s", e.what());
  }
  catch (const char *e){
    syslog(LOG_ERR, "An error %s", e);
  }
}