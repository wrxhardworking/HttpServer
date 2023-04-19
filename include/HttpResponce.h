#pragma once

#include "HttpRequest.h"

struct MineType{
    MineType (std::string type) : type (type) {};
    MineType(const char *str) : type(str) {};
    std::string type;
};

extern std::unordered_map<std::string,MineType> mtypemap;

class HttpResponce{
public:
    //http状态码
    enum HttpstatusCode{
        UNKNOW,
        OK=200,
        FORBIDEN=403,
        NOTFIND=404
    };
    //简单的初始化
    explicit HttpResponce(bool mkeep=true):
    mstatus(UNKNOW),mstatusmsg(std::string("")),
    mkeep_alive(mkeep),mversion(HttpRequest::HTTP_11),
    mtype("text/html"),mbody(nullptr),
    mcontent_length(0),mfilepath(std::string("")),
    mheaders(std::unordered_map<std::string,std::string>()) {};

    void setstatus(const HttpstatusCode& code){
        mstatus = code;
    }

    void setstatusmsg(const std::string& msg){
        mstatusmsg = msg;  
    }

    void setbody(const char* body){
        mbody = body;
    }

    void setfilepath(const std::string& filepath){
        mfilepath = filepath;
    }

    void setversion(const HttpRequest::HTTP_VERSION version){
        mversion = version;
    }

    void setfiletype(const MineType& type){
        mtype = type;
    }

    void setcontent_length(const int& content_length){
        mcontent_length = content_length;
    }

    void setkeepalive(bool keep_alive){
        mkeep_alive = keep_alive;   
    }

    void addheader(const std::string& key,const std::string& value){
         mheaders.insert(std::make_pair(key,value));
    }

    bool keep_alive(){
        return mkeep_alive;
    }
    
    const HttpRequest::HTTP_VERSION& getversion()const{
        return mversion;
    }

    const MineType& gettype()const{
        return mtype;
    }

    const char* getbody()const{
        return mbody;
    }

    const std::string& getfilepath()const{
        return mfilepath;
    }

    const int& getcontent_length()const{
        return mcontent_length;
    }

    const HttpstatusCode& getstatus()const{
        return mstatus;
    }

    const std::string& getstatusmsg()const{
        return mstatusmsg;
    }

    void appenbuffer(char*buffer)const;

private:
    HttpstatusCode mstatus;//状态码
    std::string mstatusmsg;//状态信息
    HttpRequest::HTTP_VERSION mversion;//版本
    bool mkeep_alive;//等待其他请求
    MineType mtype;//文件类型
    const char* mbody;//响应信息主体
    int mcontent_length;//响应长度
    std::string mfilepath;//请求的文件路径
    std::unordered_map<std::string,std::string> mheaders;
};