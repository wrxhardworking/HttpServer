#pragma once
#include "HttpParse.h"
#include "HttpRequest.h"
#include "HttpResponce.h"
#include "Socket.h"
#include "Timer.h"
#include <iostream>
#include <memory>
class TimerNode;
class HttpData{
public:
    //智能指针存在的方式 是为了完美利用raii机制
    std::shared_ptr<HttpRequest>request;
    std::shared_ptr<HttpResponce>responce;
    std::shared_ptr<Client>client;
    std::weak_ptr<TimerNode> node;
    ~HttpData() {
    }
    void setTimer(std::shared_ptr<TimerNode> node);
    // void closeTimer();
    void closeTimer();
};
