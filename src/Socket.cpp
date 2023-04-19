#include"../include/Socket.h"
#include"../include/Epoll.h"
#include"../include/Singleton.hpp"
#include"../include/Logger.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void setreusefd(int fd){
    int resue=1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,&resue,sizeof(resue));
}//设置fd地址复用 强制使用处于time_wait的客户端socket

void Sever::catch_error(bool error, const char *message) {
  if (error) {
    perror(message);
    // exit(1);
  }
}

void setnonblock(int fd){
    int old_option=fcntl(fd,F_GETFL);//得到文件描述符
    int new_option=old_option | O_NONBLOCK;//设置io为非阻塞
    int res=fcntl(fd,F_SETFL,new_option);//改变文件描述符的属性
}//设置io为非阻塞 为后面的io复用epoll边沿触发做准备 

void setBlock(int fd)
{
	int flags = fcntl(fd, F_GETFL);
	flags &= ~O_NONBLOCK;
	int ret = fcntl(fd, F_SETFL, flags);
}


void Sever::bind(){
    int res = ::bind(lfd,(sockaddr*)&addr,sizeof(addr));
    catch_error(res<0,"bind failed");
}

void Sever::listen(){
    int res = ::listen(lfd,5);
    catch_error(res<0,"listen failed");
}

void Sever::init(int port){
    this->mport=port;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(this->mport);
    lfd=socket(AF_INET, SOCK_STREAM, 0);
    catch_error(lfd<0,"create socket failed");
    setnonblock(lfd);//设置为非阻塞io
    setreusefd(lfd);//设置地址复用
    bind();
    listen();
}

void Sever::close(){
    if(lfd>0)
    ::close(lfd);
}

std::shared_ptr<Client> Sever::accept(){
    std::shared_ptr<Client> cli(new Client());
    
    cli->cfd=::accept(lfd,(struct sockaddr*)&cli->addr,&cli->size);
    setnonblock(cli->cfd);
    catch_error(cli->cfd<0,"accept failed");
    char ip[INET_ADDRSTRLEN];
    LOG_INFO<<"client info : ip:"<<inet_ntop(AF_INET,&cli->addr.sin_addr.s_addr,ip,INET_ADDRSTRLEN)<<"port:"<<ntohs(cli->addr.sin_port);
    return cli;
}

Sever::~Sever(){
    close();
} 

Client::Client(){
    cfd=-1;
    bzero(&this->addr,sizeof(struct sockaddr_in));
    this->size=sizeof(addr);
}

void Client::close(){
    if(cfd>0)
    ::close(cfd);
}

Client::~Client(){
    //fixme
    close();
}