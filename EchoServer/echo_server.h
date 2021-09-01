#pragma once
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<iostream> 
#include <map>
#include<vector>

#define MAX_EVENTS_NUM 100
#define R 0
#define W 1
#define R_W 2

class Epoll;

class Handler {
public:
	virtual int ReadProcess(epoll_event ev) = 0;
	virtual int WriteProcess(epoll_event ev) = 0;
};

struct Processor {
	int fd;
	Handler* handler;
	char* msg_buf;
};


class Server :public Handler {
public:
	virtual int ReadProcess(epoll_event ev);
	virtual int WriteProcess(epoll_event ev);
	Server(char* listen_ip, char* listen_port, int buf_len, Epoll* epoll);
	int ListenFd();
	int Run();
	void DeleteProcessor(Processor* processor);
private:
	int m_server_fd;
	char* m_listen_ip;
	char* m_listen_port;
	int m_buf_len;
	Epoll* m_epoll;;
};

class Epoll {
public:
	Epoll();
	int Init();
	int Run();
	int AddFd(int event_type, Processor* processor);
	int ModifyFd(int event_type, epoll_event ev);
	int DeleteFd(int fd);
	void Close();
private:
	int m_epoll_fd;
	epoll_event* m_events;
};


