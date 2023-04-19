#pragma once

#include "HttpData.h"

#include<unordered_map>//基于哈希表实现的map 空间换时间
#include <memory>
#include<vector>
#include <sys/epoll.h>

#define MAX_EVENTS_NUMBER 1024

class Epoll
{

public:
    Epoll();

    int addfd(int cfd, std::shared_ptr<HttpData> httpdata,bool isshot);

    int delfd(int cfd);

    int modfd(int cfd,std::shared_ptr<HttpData> httpdata);

    int reset_oneshot(int cfd);

    std::vector<std::shared_ptr<HttpData>> poll();

    void catch_error(bool error, const char *message);

    void handle_connection();

    void addData(int fd, std::shared_ptr<HttpData> data);

    void delData(int fd);


    std::unordered_map<int, std::shared_ptr<HttpData> > httpdatas;
    
private:

    //存储映射在红黑树中的fd


    int epoll_fd;

    epoll_event events[MAX_EVENTS_NUMBER];
};