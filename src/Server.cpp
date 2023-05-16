#include "../include/Singleton.hpp"
#include "../include/Threadpool.hpp"
#include "../include/Epoll.h"
#include "../include/Logger.h"

#include <bits/types/FILE.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <ctime>
#define MAX_BUFFER_SIZE 4096

static const std::string ServerName = " WRXS/1.0";
static const std::string basepath = "..";
char NOT_FOUND_PAGE[] = "<html>\n"
                        "<head><title>404 Not Found</title></head>\n"
                        "<body bgcolor=\"white\">\n"
                        "<center><h1>404 Not Found</h1></center>\n"
                        "<hr><center>WRXS/1.0 (Linux)</center>\n"
                        "</body>\n"
                        "</html>";

char FORBIDDEN_PAGE[] = "<html>\n"
                        "<head><title>403 Forbidden</title></head>\n"
                        "<body bgcolor=\"white\">\n"
                        "<center><h1>403 Forbidden</h1></center>\n"
                        "<hr><center>WRXS/1.0(Linux)</center>\n"
                        "</body>\n"
                        "</html>";

char INDEX_PAGE[] =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "    <title>Welcome to LC WebServer!</title>\n"
    "    <style>\n"
    "        body {\n"
    "            width: 35em;\n"
    "            margin: 0 auto;\n"
    "            font-family: Tahoma, Verdana, Arial, sans-serif;\n"
    "        }\n"
    "    </style>\n"
    "</head>\n"
    "<body>\n"
    "<h1>Welcome to LC WebServer!</h1>\n"
    "<p>If you see this page, the lc webserver is successfully installed and\n"
    "    working. </p>\n"
    "\n"
    "<p>For online documentation and support please refer to\n"
    "    <a href=\"https://github.com/MarvinLe/WebServer\">LC "
    "WebServer</a>.<br/>\n"
    "\n"
    "<p><em>Thank you for using LC WebServer.</em></p>\n"
    "</body>\n"
    "</html>";

void Server::run(int port)
{


    //初始化客户端
    Singleton<Sever>::getInstance()->init(port);
    //传进去的是一个引用

    Singleton<Epoll>::getInstance()->addfd(Singleton<Sever>::getInstance()->getfd(), nullptr, false);
    Singleton<ThreadPool>::getInstance()->init();
    while (true)
    {

        //得到当前响应的事件对应的data
        LOG_INFO<<"poll begin";
        auto curdatas = Singleton<Epoll>::getInstance()->poll();
        //安排线程池处理事件
        if(!curdatas.empty()){

        for (const auto& curdata : curdatas)
        {
           Singleton<ThreadPool>::getInstance()->submit([this, curdata]()
             { this->deal(curdata);
            });
        }
        }

        Singleton<TimerManager>::getInstance()->handle_expired_event();
        LOG_INFO<<"poll end";
    }

    
}

HttpParse::HTTP_CODE Server::recv_request(std::shared_ptr<HttpData> data) {
  //初始状态
  char buffer[MAX_BUFFER_SIZE];
  int read_index = 0;
  int start_index = 0;
  int check_index = 0;
  //请求信息状态
  HttpParse::CHECK_STATE parse_state = HttpParse::CHECK_STATE_REQUESTLINE;

  read_index = readn(data->client->cfd, buffer);

  auto httpcode = Singleton<HttpParse>::getInstance()->parse_content(
      buffer, check_index, parse_state, read_index, start_index,
      *data->request);
//   std::cout << "httpcode successfully" << std::endl;

  return httpcode;
}

void Server::deal(std::shared_ptr<HttpData> data)
{
    auto code = recv_request(data);
    if(code==HttpParse::GET_REQUEST){
        analysis_request(data);
        auto code1 = handle_resource(data);
        switch(code1){
            case Server::RESOURCE::FORBIDDEN:{
                handle_fbdresponse(data);
            }
            case Server::RESOURCE::SUCCESS:{
                handle_normal_response(data);
            }
            case Server::RESOURCE::NOTFIND:{
                handle_nfdresponse(data);
            }
        }
    }
    else {
    }
}

void Server::analysis_request(std::shared_ptr<HttpData> data){

    //设置回应的版本
    if (data->request->m_version == HttpRequest::HTTP_11)
    {
        data->responce->setversion(HttpRequest::HTTP_11);
    }
    else
    {
        data->responce->setversion(HttpRequest::HTTP_10);
    }
    //将服务器添加到头部字段
    data->responce->addheader("Server", ServerName);

    //提取参数  文件路径
    std::string arg;//后续实现 外边需要看到的
    std::string filepath = data->request->m_url;
    if (data->request->m_url.rfind('?') != string::npos)
    {
        //找到文件路径 删除后面的参数
        filepath.erase(filepath.rfind('?'));
    }
    // std::cout << "filepath is" << filepath << std::endl;
    data->responce->setfilepath(filepath);

    //确定响应文件类型
    std::string filetype;
    if (filepath.rfind('.') != string::npos)
    {
        filetype = filepath.substr(filepath.rfind('.'));
    }
    auto type_it = mtypemap.find(filetype);
    if (type_it != mtypemap.end())
    {
        data->responce->setfiletype(type_it->second);
    }
    else
    {
        data->responce->setfiletype(mtypemap.find("default")->second);
    }
    // std::cout << "filetype is" << filetype << std::endl;

    //检查keep_value 是否要保活
    auto header_it = data->request->m_header.find(HttpRequest::Connection);

    if (header_it != data->request->m_header.end())
    {
        // std::cout << header_it->second << std::endl; //头部是有空格的
        //记住要加空格
        if (header_it->second == " keep-alive")
        {
            data->responce->setkeepalive(true);
            data->responce->addheader("Keep-Alive", std::string("timeout=20"));
        }
        else
        {
            data->responce->setkeepalive(false);
        }
    }
}

Server::RESOURCE
Server::handle_resource(std::shared_ptr<HttpData> data){
    std::string path = basepath;
    auto filepath = data->responce->getfilepath();
    path.append(filepath);
    //用来获取文件的状态
    struct stat statbuf{};
    //如果文件
    if (filepath == "/")
    {
        data->responce->setfiletype(MineType("text/html"));
        data->responce->setstatus(HttpResponce::OK);
        data->responce->setstatusmsg("Ok");
    }
    else if (stat(path.c_str(), &statbuf) == -1)
    {
        //加入文件不存在 返回404html文件
        data->responce->setfiletype(MineType("text/html"));
        data->responce->setstatus(HttpResponce::NOTFIND);
        data->responce->setstatusmsg("Not Found");
        LOG_INFO<<"NOT FOUND";
        return RESOURCE::NOTFIND;
    }
    //为特殊文件不能访问
    else if (!S_ISREG(statbuf.st_mode))
    {
        data->responce->setfiletype(MineType("text/html"));
        data->responce->setstatus(HttpResponce::FORBIDEN);
        data->responce->setstatusmsg("ForBidden");
        // std::cout << "FORBIEEDN" << std::endl;
        LOG_INFO<<"FORBIEEDN";
        return RESOURCE::FORBIDDEN;
    }
    //此处设置服务器内部出错的状态码和状态信息
    else
    {
        data->responce->setstatus(HttpResponce::OK);
        data->responce->setstatusmsg("Ok");
        data->responce->setfilepath(path);
        return RESOURCE::SUCCESS;
    }
}

void Server::handle_nfdresponse(std::shared_ptr<HttpData> data){
    char buffer[MAX_BUFFER_SIZE];
    bzero(buffer, '\0');
    data->responce->appenbuffer(buffer);
    sprintf(buffer, "%sContent-length: %d\r\n\r\n", buffer, strlen(NOT_FOUND_PAGE));
    sprintf(buffer, "%s%s", buffer, NOT_FOUND_PAGE);
    writen(data->client->cfd, buffer, strlen(buffer));
}

void Server::handle_fbdresponse(std::shared_ptr<HttpData> data){
    char buffer[MAX_BUFFER_SIZE];
    data->responce->appenbuffer(buffer);
    bzero(buffer, '\0');
    sprintf(buffer, "%sContent-length: %d\r\n\r\n", buffer, strlen(FORBIDDEN_PAGE));
    sprintf(buffer, "%s%s", buffer, FORBIDDEN_PAGE);
    writen(data->client->cfd, buffer, strlen(buffer));
}


void Server::handle_normal_response(std::shared_ptr<HttpData> data){
        char buffer[MAX_BUFFER_SIZE];
        bzero(buffer, '\0');
        data->responce->appenbuffer(buffer);
        struct stat file_stat{};
        int res = stat(data->responce->getfilepath().c_str(), &file_stat);
        if (res < 0)
        {
            // std::cout << "文件路径有问题" << std::endl;
            LOG_ERROR<<"file path error";
            return;
        }
        int fd = ::open(data->responce->getfilepath().c_str(), O_RDONLY);
        //文件打不开 服务器内部错误 将重新设置状态码
        if (fd < 0)
        {
            LOG_ERROR<<"file open failed";
            close(fd);
            return;
        }
        //将文件映射到进程内存中 成功返回内存的指针 失败返回MAP_FAILED
        char* filebuffer = static_cast<char *>(mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
        if (filebuffer == MAP_FAILED)
        {
            std::cerr << "mmap failed" << std::endl;
            close(fd);
            return;
        }
        sprintf(buffer, "%sContent-length: %d\r\n\r\n", buffer, file_stat.st_size);
        std::string s = std::string(buffer,buffer+strlen(buffer));
        s+=std::string(filebuffer,filebuffer+file_stat.st_size);
        s+=std::string("\r\n\r\n");

        //fixme
        writen(data->client->cfd,s.c_str(),s.length());

        close(fd);
        munmap(filebuffer, file_stat.st_size);

        //fixme
        // if(data->responce->keep_alive()){
        //     Singleton<Epoll>::getInstance()->modfd(data->client->cfd,data);
        //     Singleton<TimerManager>::getInstance()->addTimer(data,20* 100);
        // }
        return;
}


ssize_t Server::readn(int fd, char* buf)
{
    auto ptr = buf;
    ssize_t sum = 0;
    ssize_t tmplen = 0;
    while (true)
    {
        tmplen = ::recv(fd, ptr, MAX_BUFFER_SIZE - sum, 0);
        if (tmplen < 0)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
                return sum;
            else
            {
                perror("意外错误");
                return -1;
            }
        }
        else if (tmplen == 0)
        {
            //客户端断开连接
            break;
        }
        else
        {
            ptr += tmplen;
            sum += tmplen;
        }
    }
    return sum;
}

ssize_t Server::readn(int fd, std::string &buf)
{
    ssize_t tmplen = 0;
    ssize_t sum = 0;

    while (true)
    {
        char buff[MAX_BUFFER_SIZE];
        tmplen = ::recv(fd, buff, MAX_BUFFER_SIZE - sum, 0);
        if (tmplen < 0)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
                return sum;
            else
            {
                perror("意外错误");
            }
        }
        else if (tmplen == 0)
        {
            //客户端断开连接
            break;
        }
        else
        {
            sum += tmplen;
            buf += std::string(buff, buff + tmplen);
        }
    }

    return sum;
}

ssize_t Server::writen(int fd, const char *buf, size_t n)
{
    ssize_t size = n;
    ssize_t sum = 0;
    ssize_t tmplen = 0;
    auto ptr = (char *)buf;
    while (size > 0)
    {
        tmplen = ::write(fd, ptr, size);
        if (tmplen <= 0)
        {
            if (tmplen < 0)
            {
                if (errno == EINTR)
                {
                    tmplen = 0;
                    continue;
                }
                else if (errno == EAGAIN)
                { //写到不能再写
                    continue;
                }
                else
                {
                    return -1;
                }
            }
        }
        else
        {
            ptr += tmplen;
            sum += tmplen;
            size -= tmplen;
        }
    }
    return sum;
}

ssize_t Server::writen(int fd, std::string &buf, size_t n)
{
    ssize_t size = buf.length();
    ssize_t sum = 0;
    size_t tmplen = 0;
    char *ptr = (char *)buf.c_str();
    setBlock(fd);
    while (size > 0)
    {
        tmplen = ::write(fd, ptr, size);
        if (tmplen <= 0)
        {
            if (tmplen < 0)
            {
                // if (errno == EINTR)
                // {
                //     tmplen = 0;
                //     continue;
                // }
                // else if (errno == EAGAIN)
                // { //写到不能再写
                //   continue;
                // }
                // else
                // {
                //     return -1;
                // }
            }
            break;
        }
        else
        {
            size -= tmplen;
            ptr += tmplen;
            sum += tmplen;
        }
    }
    setnonblock(fd);
    return sum;
}