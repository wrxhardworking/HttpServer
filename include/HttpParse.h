#pragma once

#include<iostream>
#include<string>
#include<unordered_map>

class HttpRequest;

class HttpParse
{
public:

    static HttpParse*getparser();//获取单例模式

    //将状态机分为主状态机和从状态机
    //主状态机
    enum CHECK_STATE{
    //两种情况 分别是当前处于请求行状态 当前处于请求头部信息状态  请求具体信息
    CHECK_STATE_REQUESTLINE=0,CHECK_STATE_HEADER,CHECK_STATE_BODY
    };
    //从状态机
    enum LINE_STATUS{
    //三种状态分别表示：读取到一个完整的行，行出错，行数据尚不完整
    LINE_OK=0,
    LINE_BAD,
    LINE_OPEN
    };
    //以下是服务器处理http服务请求的结果
    enum HTTP_CODE{
    NO_REQUEST,//表示请求不完整，需要继续拂去客户数据
    GET_REQUEST,//表示获得了一个完整的客户请求
    BAD_REQUEST,//表示客户请求有语法错误
    FORBIDDEN_REQUEST,//表示客户没有足够的权限访问
    INTERNAT_ERROR,//表示服务器内部错误
    LOSE_CONNECTION //表示失去了连接 
    };

    LINE_STATUS parse_line(char*buffer,int& check_index,int& read_index);

    HTTP_CODE parse_request(char* line,CHECK_STATE& check_state,HttpRequest& request);

    HTTP_CODE parse_header(char* line,CHECK_STATE& check_state,HttpRequest& request);

    HTTP_CODE parse_body(char* line,HttpRequest& request);

    HTTP_CODE parse_content(char *buffer,int&cheaked_index,CHECK_STATE&checkstate,int &read_index,int& start_line,HttpRequest& request);
};

