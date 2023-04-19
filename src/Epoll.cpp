#include "../include/Epoll.h"
#include "../include/Socket.h"
#include "../include/Singleton.hpp"
#include "../include/HttpData.h"
#include "../include/Logger.h"

#include <memory>
#include <unordered_map>

void Epoll::catch_error(bool error, const char *message)
{
    if (error)
    {
        perror(message);
        // exit(1);
    }
}

Epoll::Epoll():httpdatas()//
{
    //创建底层红黑树
    epoll_fd = epoll_create(10000);
    catch_error(epoll_fd < 0, "epoll_fd crate failed");
}

int Epoll::addfd(int cfd, std::shared_ptr<HttpData> httpdata,bool isshot)
{
    epoll_event event;
    event.data.fd = cfd;
    // ET模式边沿触发 设置写事件
    event.events = EPOLLIN | EPOLLET ;
    if(isshot){
        //保证socket在任何时刻 只会被一个线程处理 不能listenfd是不能设置 否则只能处理一个客户端连接
        //因为后续的客户请求将不再触发listenfd上的epoll in 事件
        //保存客户端的收发消息 
        //添加文件描述符到 map
        setnonblock(cfd);
        addData(cfd,httpdata);
        event.events |= EPOLLONESHOT;
    }
    //将接收到的client设为非阻塞fd

    int res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cfd, &event);
    catch_error(res < 0, "add ctr failed");
    return 0;
}

int Epoll::delfd(int cfd )
{
    int res = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cfd, NULL);
    catch_error(res < 0, "del ctr failed");
    //删除存储map中的cfd和data
    delData(cfd);
    return 0;
}
//此处应该可以改为 响应自己传入的事件
int Epoll::modfd(int cfd, std::shared_ptr<HttpData> httpdata)
{
    epoll_event event;
    event.data.fd = cfd;
    event.events=EPOLLIN | EPOLLET | EPOLLONESHOT;
    int res = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, cfd, &event);
    httpdatas[cfd]=httpdata;
    catch_error(res < 0, "mod ctr failed");
    return 0;
}

int Epoll::reset_oneshot(int cfd){
    epoll_event event;
    event.data.fd=cfd;
    event.events=EPOLLIN |  EPOLLET | EPOLLONESHOT;
    epoll_ctl(epoll_fd,EPOLL_CTL_MOD,cfd,&event);
}

void Epoll::handle_connection(){
    //遇到的难题：不能返回一个值对象 使得文件描述符在析构函数调用的时候关闭 用智能指针
    auto client=Singleton<Sever>::getInstance()->accept();
    if(client->cfd>MAX_EVENTS_NUMBER){
        close(client->cfd);
        return;
    }
    using std::shared_ptr;
    shared_ptr<Client> sharedclient(client);
    //可在这里做限制并发    定时器操作
    shared_ptr<HttpRequest> sharedrequest=std::make_shared<HttpRequest>();
    shared_ptr<HttpResponce> sharedresponce=std::make_shared<HttpResponce>();

    shared_ptr<HttpData> data = std::make_shared<HttpData>();

    data->client=sharedclient;
    data->responce=sharedresponce;
    data->request=sharedrequest;
    //将fd添加红黑树中
    addfd(sharedclient->cfd,data,true);
    // Singleton<TimerManager>::getInstance()->addTimer(data,20*100);
    
}

//用智能指针而不是锁来保证线程安全!!!!!!!!
std::vector<std::shared_ptr<HttpData>> Epoll::poll()
{
    std::vector<std::shared_ptr<HttpData>> curdatas;

    //一直阻塞直到有连接到来 

    int num = epoll_wait(epoll_fd, events, MAX_EVENTS_NUMBER, -1);
    catch_error(num < 0, "epoll failed");

    for (int i = 0; i < num; i++)
    {
        int tempfd = events[i].data.fd;
        //监听文件描述符事件响应
        if (Singleton<Sever>::getInstance()->getfd() == tempfd)
        {
            handle_connection();
        }
        //客户端文件描述符事件响应
        else if((events[i].events & EPOLLERR) || (events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLHUP))
        {
            auto it =httpdatas.find(tempfd);
            if(it != httpdatas.end()){
                //在map中将文件描述符删除
                //fixme
                //it->second->closeTimer();
                //fixme 
                //将定时器和httpdata分离 
                // it->second->closeTimer();
                this->delData(it->second->client->cfd);
            }    
                continue;
            
        }
        else if(events[i].events & EPOLLIN)
        {
            LOG_INFO<<"have read client";
            auto it = httpdatas.find(tempfd);
            if(it != httpdatas.end()){
                //返回的是一个pair队 将其存入要处理的数据的数组中
                //fixme
                curdatas.push_back(it->second);
                // //防止标记删除
                this->delData(it->second->client->cfd);
            }
            else{
            ::close(tempfd);
            LOG_INFO<<"a bad fd";
            }
        }
    }
    return curdatas;
}


void Epoll::addData(int fd,std::shared_ptr<HttpData> data){
        //加入data事件        
        // httpdatas.insert(std::make_pair(fd,data));
        httpdatas[fd] = data;
}

void Epoll::delData(int fd){
    //智能指针内部自动加锁解锁 所以是线程安全的
        auto it = httpdatas.find(fd);
        if (it != httpdatas.end()) {
          httpdatas.erase(fd);
        }
}

