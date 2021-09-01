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
#include "echo_server.h"
#include<vector>


Epoll::Epoll() {
	m_events = new epoll_event[MAX_EVENTS_NUM];
}

int Epoll::Init() {
	m_epoll_fd = epoll_create1(0);
	if (m_epoll_fd == -1) {
		return -1;
	}
	return 0;
}


int Epoll::AddFd(int event_type, Processor* processor) {
	epoll_event ev;
	int fd = processor->fd;
	if (fd > MAX_EVENTS_NUM) {
		return -1;
	}
	switch (event_type)
	{
	case R:
		ev.events = EPOLLIN;
		break;
	case W:
		ev.events = EPOLLOUT;
		break;
	case R_W:
		ev.events = EPOLLIN | EPOLLOUT;
	}
	ev.data.ptr = processor;
	int err = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &ev);
	if (err == -1) {
		return err;
	}
	return 0;
}


int Epoll::ModifyFd(int event_type, epoll_event ev) {
	int fd = ((Processor*)ev.data.ptr)->fd;
	switch (event_type)
	{
	case R:
		ev.events = EPOLLIN;
		break;
	case W:
		ev.events = EPOLLOUT;
		break;
	case R_W:
		ev.events = EPOLLIN | EPOLLOUT;
	}
	int err = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &ev);
	if (err == -1) {
		return err;
	}
	return 0;
}


int Epoll::DeleteFd(int fd) {
	int err = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	if (err == -1) {
		return err;
	}
	close(fd);
	return 0;
}


int Epoll::Run() {
	while (1) {
		int num_events = epoll_wait(m_epoll_fd, m_events, sizeof(m_events), -1);
		if (num_events < 0) {
			return -1;
		}
		for (int i = 0; i < num_events; i++) {
			if (m_events[i].events & EPOLLIN) {
				int err = ((Processor*)m_events[i].data.ptr)->handler->ReadProcess(m_events[i]);
				if (err == -1) {
					return err;
				}
			}
			if (m_events[i].events & EPOLLOUT) {
				int err = ((Processor*)m_events[i].data.ptr)->handler->WriteProcess(m_events[i]);
				if (err == -1) {
					return err;
				}
			}
		}
	}
	return 0;
}


void Epoll::Close() {
	close(m_epoll_fd);
}


Server::Server(char* listen_ip, char* listen_port, int buf_len, Epoll* epoll) {
	m_listen_ip = listen_ip;
	m_listen_port = listen_port;
	m_buf_len = buf_len;
	m_epoll = epoll;
}


int Server::ListenFd() {
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	//server socket address
	struct sockaddr_in  hostaddr;
	memset(&hostaddr, 0, sizeof(hostaddr));
	hostaddr.sin_family = AF_INET;
	hostaddr.sin_addr.s_addr = inet_addr(m_listen_ip);
	hostaddr.sin_port = htons(atoi(m_listen_port));
	int err;
	err = bind(listen_fd, (struct sockaddr*)&hostaddr, sizeof(hostaddr));
	if (err == -1) {
		return err;
	}
	err = listen(listen_fd, 1024);
	if (err == -1) {
		return err;
	}
	int opt = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	return listen_fd;
}


int Server::Run() {
	m_server_fd = ListenFd();
	if (m_server_fd == -1) {
		return -1;
	}
	Processor* processor = new Processor();
	processor->fd = m_server_fd;
	processor->handler = this;
	processor->msg_buf = new char[m_buf_len];
	int err = m_epoll->AddFd(R, processor);
	if (err == -1) {
		return err;
	}
	err = m_epoll->Run();
	if (err == -1) {
		return err;
	}
	DeleteProcessor(processor);
	err = m_epoll->DeleteFd(m_server_fd);
	if (err == -1) {
		return err;
	}
	return 0;
}


int Server::ReadProcess(epoll_event ev) {
	int fd = ((Processor*)ev.data.ptr)->fd;
	char* msg_buf = ((Processor*)ev.data.ptr)->msg_buf;
	Processor* processor = (Processor*)ev.data.ptr;
	if (fd == m_server_fd) {
		socklen_t len;
		struct sockaddr_in  client_addr;
		int client_sd = accept(m_server_fd, (sockaddr*)&client_addr, &len);
		if (client_sd == -1) {
			return client_sd;
		}
		Processor* processor = new Processor();
		processor->fd = client_sd;
		processor->handler = this;
		processor->msg_buf = new char[m_buf_len];
		m_epoll->AddFd(R, processor);
	}
	else {
		int n = read(fd, msg_buf, m_buf_len);
		if (n < 0) {
			return -1;
		}
		if (n == 0) {
			DeleteProcessor(processor);
			m_epoll->DeleteFd(fd);
			return 0;
		}
		m_epoll->ModifyFd(W, ev);
	}
	return 0;
}


int Server::WriteProcess(epoll_event ev) {
	int fd = ((Processor*)ev.data.ptr)->fd;
	char* msg_buf = ((Processor*)ev.data.ptr)->msg_buf;
	int n = write(fd, msg_buf, strlen(msg_buf));
	if (n < 0) {
		return n;
	}
	memset(msg_buf, '\0', sizeof(msg_buf));
	m_epoll->ModifyFd(R, ev);
	return 0;
}


void Server::DeleteProcessor(Processor* processor) {
	delete(processor->msg_buf);
	delete(processor);
}


int main(int argc, char* argv[]) {
	char* ip = argv[1];
	char* port = argv[2];
	Epoll* epoll = new Epoll();
	Server* server = new Server(ip, port, 5, epoll);
	int err = epoll->Init();
	if (err == -1) {
		return err;
	}
	err = server->Run();
	if (err == -1) {
		return err;
	}
	delete(server);
	epoll->Close();
	delete(epoll);
	return 0;
}








