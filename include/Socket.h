    #pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <memory>
    
void setreusefd(int fd);//设置fd地址复用 强制使用处于time_wait的客户端socket

void setnonblock(int fd);//设置io为非阻塞 为后面的io复用epoll边沿触发做准备 

void setBlock(int fd);//设置为阻塞io

class Client;

class Sever{

public:
    ~Sever();

    void init(int port=12346);

    std::shared_ptr<Client> accept();

    inline int getfd(){
        return lfd;
    }

    int lfd;

private:
    void bind();

    void send();
    
    void listen();
    
    void close();

    void catch_error(bool error,const char*message);

    int mport;

    sockaddr_in addr;
};

class Client{
public:

    Client();

    socklen_t size;
    
    sockaddr_in addr;

    int cfd;

    void close();

    ~Client();

};