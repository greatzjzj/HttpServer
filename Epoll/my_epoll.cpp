#include "my_epoll.h"
#include <assert.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include<errno.h>
#include<fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<iostream> 


using namespace std;
epoll_event ev;

Epoll::Epoll(int BUF_LEN, int MAX_EVENTS) {
	m_mesg_get = new char[BUF_LEN];
	m_events = new epoll_event[MAX_EVENTS];
	m_epoll_fd = epoll_create1(0);
	if (m_epoll_fd == -1)
	{
		cout << "epoll_create\n";
		exit(-1);
	}
}

int  Epoll::Socket(int port, char* ip) {
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	//server socket address
	struct sockaddr_in  hostaddr;
	memset(&hostaddr, 0, sizeof(hostaddr));
	hostaddr.sin_family = AF_INET;
	hostaddr.sin_addr.s_addr = inet_addr(ip);
	hostaddr.sin_port = htons(port);
	//bind 
	int if_bind = bind(sd, (struct sockaddr*)&hostaddr, sizeof(hostaddr));
	if (if_bind == -1) {
		cout << "bind failed";
		exit(-1);
	}
	//listen
	int if_listen = listen(sd, 1024);
	if (if_listen == -1) {
		cout << "listen failed";
		exit(-1);
	}
	int opt = 1;  //设置为接口复用
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	return sd;
}

void Epoll::server_ctl(int server_sd) {
	ev.data.fd = server_sd;
	ev.events = EPOLLIN;
	epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, server_sd, &ev);
}

int Epoll::get_ready_fds() {
	int ready_fds = epoll_wait(m_epoll_fd, m_events, sizeof(m_events), -1);
	return ready_fds;
}

const char* Epoll::match_event(int i) {
	const char* variable;
	if (m_events[i].data.fd == m_server_sd) {
		variable = "new connect";
		return variable;
	}
	if (m_events[i].events & EPOLLIN) {
		variable = "read";
		return variable;
	}
	if (m_events[i].events & EPOLLOUT) {
		variable = "write";
		return variable;
	}
}

void Epoll::process_event(int e, int i) {
	int client_sd;
	int n;
	switch (e) {
	case 0:
		socklen_t len;
		struct sockaddr_in  client_addr;
		client_sd = accept(m_server_sd, (sockaddr*)&client_addr, &len); //accept这个连接
		ev.data.fd = client_sd;
		ev.events = EPOLLIN;
		epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, client_sd, &ev);
	case 1:
		client_sd = m_events[i].data.fd;
		n = read(client_sd, m_mesg_get, 100);   //读
		ev.events = EPOLLOUT;
		epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, client_sd, &ev);
	case 2:
		write(client_sd, m_mesg_get, strlen(m_mesg_get));        //发送数据
		memset(m_mesg_get, '\0', sizeof(m_mesg_get));
		ev.data.fd = client_sd;
		ev.events = EPOLLIN;
		epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, client_sd, &ev);
	}
}
void Epoll::m_close() {
	close(m_server_sd);
	close(m_epoll_fd);
}




int main(int argc, char* argv[]) {
	int port = atoi(argv[2]);
	char* ip = argv[1];
	int BUF_LEN = argv[3];
	int MAX_EVENTS = argv[4];

	Epoll epoll = new Epoll(BUF_LEN, MAX_EVENTS);

	int server_sd = epoll.Socket(port, ip);

	epoll.server_ctl(server_ctl);

	int ready_num = epoll.get_ready_fds();

	for (int i = ready_num; i < 20; i--)
		(while i > 0) {
		const char* event = epoll.match_event(i)

	}
}
