#pragma once

#include "HttpData.h"
#include "Logger.h"
#include <memory>
#include <string>

class Server {
public:

    enum class RESOURCE{
        SUCCESS,
        NOTFIND,
        FORBIDDEN
    };

    void run(int port);
private:

    HttpParse::HTTP_CODE recv_request(std::shared_ptr<HttpData> data);

    void analysis_request(std::shared_ptr<HttpData> data);

    RESOURCE handle_resource(std::shared_ptr<HttpData> data);
    
    void handle_nfdresponse(std::shared_ptr<HttpData> data);

    void handle_fbdresponse(std::shared_ptr<HttpData> data);

    void handle_normal_response(std::shared_ptr<HttpData> data);

    void handle_close(std::shared_ptr<HttpData> data);

    

    
    void setstatusline(std::shared_ptr<HttpData> data);

    void setheadersline(std::shared_ptr<HttpData> data);

    void dealfilepath(std::shared_ptr<HttpData> data);

    void send(std::shared_ptr<HttpData> data);

    void deal(std::shared_ptr<HttpData> data);

    void sendfile(std::shared_ptr<HttpData> data);

    ssize_t readn(int fd,char buf[]);

    ssize_t readn(int fd,std::string & buf);

    ssize_t writen(int fd,const char*buf,size_t n);

    ssize_t writen(int fd,std::string & buf,size_t n);

};