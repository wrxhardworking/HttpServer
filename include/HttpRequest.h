#pragma once

#include<iostream>
#include<unordered_map>

class HttpRequest
{
public:
    // std::ostream& operator<<(std::ostream& , const HTTPRequest&);

    enum HTTP_VERSION {
        HTTP_10 = 0,HTTP_11, VERSION_NOT_SUPPORT//支持的http版本
    };

    enum HTTP_METHOD {GET = 0, POST, PUT, DELETE, METHOD_NOT_SUPPORT//支持的请求方式
    };

    enum HTTP_HEADER { //头部选项
      Host = 0,
      User_Agent,
      Connection,
      Accept_Encoding,
      Accept_Language,
      Accept,
      Cache_Control,
      Upgrade_Insecure_Requests
    };

    struct EnumClassHash {
      template <typename T> std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
      }};
        
    HTTP_VERSION m_version;

    HTTP_METHOD m_method;

    std::string m_url;

    char* m_content;
    
    static std::unordered_map<std::string,HTTP_HEADER> headers;//设置静态变量 以便于全局变量比较

    HttpRequest(HTTP_METHOD method=METHOD_NOT_SUPPORT,std::string url="",HTTP_VERSION version=VERSION_NOT_SUPPORT):
        m_method(method),m_url(url), m_version(version),
        m_content(nullptr),m_header(std::unordered_map<HTTP_HEADER,std::string,EnumClassHash>()) {};

    std::unordered_map<HTTP_HEADER,std::string,EnumClassHash>m_header;
};