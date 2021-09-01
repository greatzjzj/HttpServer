#pragma once
#include<fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<iostream> 

class Epoll {
private:
	epoll_event* m_events;
	char* m_mesg_get;
	int m_epoll_fd;

public:
	Epoll(int BUF_LEN, int MAX_EVENTS);
	int Socket(int port, char* ip);
	void server_ctl();
	int get_ready_fds();
	const char* match_event(int i);
	void process_event(int e, int i);
	void m_close();







};
