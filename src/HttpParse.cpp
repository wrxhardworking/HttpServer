#include"../include/HttpParse.h"
#include"../include/HttpRequest.h"
#include"../include/Logger.h"

#include<string.h>
#include<algorithm>
#include<iostream>

std::unordered_map<std::string, HttpRequest::HTTP_HEADER> HttpRequest::headers={
        {"HOST",                      HttpRequest::Host},
        {"CONNECTION",                HttpRequest::Connection},
        {"USER-AGENT",                HttpRequest::User_Agent},
        {"ACCEPT-ENCODING",           HttpRequest::Accept_Encoding},
        {"ACCEPT-LANGUAGE",           HttpRequest::Accept_Language},
        {"ACCEPT",                    HttpRequest::Accept},
        {"CACHE-CONTROL",             HttpRequest::Cache_Control},
        {"UPGRADE-INSECURE-REQUESTS", HttpRequest::Upgrade_Insecure_Requests}
};

HttpParse*HttpParse::getparser(){
    static HttpParse parser;
    return &parser;
}
 HttpParse::LINE_STATUS 
 HttpParse::parse_line(char*buffer,int& check_index,int& read_index){
    std::string line(buffer);
    for(;check_index<read_index;check_index++)
    {
        if(buffer[check_index]=='\r')
        {
            //回车符号正好在当前buffer的最后
            if(check_index==read_index-1)
            {
                return LINE_OPEN;
            }
            if(buffer[check_index+1]=='\n')
            {
                buffer[check_index++]='\0';
                buffer[check_index++]='\0';
                return LINE_OK;
            }
        }
        else if(buffer=="\n"){
            if(check_index>1 && buffer[check_index-1]=='\r')
            {
                buffer[check_index-1]='\0';
                buffer[check_index++]='\0';
                return LINE_OK;
            }
        }
    }
    return LINE_BAD;
 }

HttpParse::HTTP_CODE 
HttpParse::parse_request(char* line,CHECK_STATE& check_state,HttpRequest& request)
{
    //检查请求行中是否有 \t的字符 返回第一个匹配的字符
    char*url=strpbrk(line," \t");
    //如果请求行中没有空白的字符或者\t字符 则http请求必然有问题
    if(!url)
    {
        return BAD_REQUEST;
    }

    *url++='\0';
 
    char * method = line;

    //忽略大小写比较字符串是否相同 相同返回0
    if(strcasecmp(method, "GET") == 0)
    {
        request.m_method = HttpRequest::GET;
    }
    else if(strcasecmp(method,"POST") == 0){
        request.m_method = HttpRequest::POST;
    }
    else if(strcasecmp(method, "PUT") == 0){
        request.m_method = HttpRequest::PUT;
    }

    else if(strcasecmp(method, "DELETE") == 0){
        request.m_method = HttpRequest::DELETE;
    }
    else{
        return BAD_REQUEST;//其他的方法不支持
    }

    //返回第一个与\t不匹配的字符串的下标所对应的位置
    url+=strspn(url," \t");

    char * version = strpbrk(url, " \t");

    if(!version)
    {
        return BAD_REQUEST;
    }
    *version++;
    version+=strspn(version," \t");
    //检查版本 仅支持HTTP1.1 相同返回0
    if(strcasecmp(version,"HTTP/1.1")==0)
    {
        request.m_version = HttpRequest::HTTP_11;
    }
    else if(strcasecmp(version,"HTTP/1.0")==0)
    {
        request.m_version = HttpRequest::HTTP_10;
    }
    else{
        return BAD_REQUEST;
    }
    //检查URL是否合法 相同返回0
    if(strncasecmp(url,"http://",7)==0)
    {
       url+=7;
       //在指定字符串中搜索第一个出现的/字符的位置（字符串的位置）;
       //不需要域名
       url=strchr(url,'/');
    }
    // else if(strncasecmp(url,"/")==0){

    // }

    if(!url||url[0]!='/')
    {
        return BAD_REQUEST;
    }


    LOG_INFO<<url;
    request.m_url = std::string(url);
    //不要后面的http协议版本号
    request.m_url=request.m_url.substr(0, request.m_url.find(' '));
    //HTTP请求处理完毕 状态转移到头部字段的分析
    check_state= CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HttpParse::HTTP_CODE
HttpParse::parse_header(char* line,CHECK_STATE& check_state,HttpRequest& request)
{
    if(*line== '\0')
    {
        //接受请求方式
        if(request.m_method==HttpRequest::GET)//get请求直接返回了
           return GET_REQUEST;
        else
            check_state=CHECK_STATE_BODY;
        return NO_REQUEST;//继续向下解析
    }

    char key[200],value[300];

    //正则表达式
    sscanf(line, "%[^:]:%[^:]", key, value);

    decltype(HttpRequest::headers)::iterator it;

    std::string key_s(key);
    std::transform(key_s.begin(), key_s.end(),key_s.begin(),::toupper);//转为大写 匹配map中的key
    std::string value_s(value);
    // std::cout <<key_s<<"?"<<value_s<<std::endl;   
    if((it=HttpRequest::headers.find(key_s)) != (HttpRequest::headers.end()))
    {
        request.m_header.insert(std::make_pair(it->second,value_s));
        // request.m_header.insert(std::make_pair(HttpRequest::Host,"SDJHBAHJ"));
    }

    return NO_REQUEST;//主状态机的状态
}

//得到内容
HttpParse::HTTP_CODE HttpParse::parse_body(char *line, HttpRequest &request) {
  //可以对得到的内容进行细分
  request.m_content = line;
  return GET_REQUEST;
}

HttpParse::HTTP_CODE 
HttpParse::parse_content(char *buffer,int&cheaked_index,CHECK_STATE&checkstate,int &read_index,int& start_line,HttpRequest& request)
{
    LINE_STATUS line_state=LINE_OK;
    HTTP_CODE retcode=NO_REQUEST;
    while((line_state=parse_line(buffer,cheaked_index,read_index))==LINE_OK)
    {
        char*line=buffer+start_line;//每次循环调的开始位置是在变化的
        start_line=cheaked_index;
        
        switch(checkstate)
        {
            case CHECK_STATE_REQUESTLINE:
            {

                retcode=parse_request(line,checkstate,request);
                if(retcode==BAD_REQUEST){
                return BAD_REQUEST;
                }
                break;
            }
            case CHECK_STATE_HEADER:
            {

                retcode=parse_header(line,checkstate,request); 
                if(retcode==BAD_REQUEST)
                {
                return BAD_REQUEST;
                }
                else if(retcode==GET_REQUEST)
                return GET_REQUEST;
                break;
            }
            case CHECK_STATE_BODY:
            {
                retcode=parse_body(line,request);
                if(retcode==GET_REQUEST)
                return GET_REQUEST;

                {
                return BAD_REQUEST;
                }
            }
            default:
                return INTERNAT_ERROR;
        }
    }
    if(line_state=LINE_OPEN)//没有读到一个完整的行
    {
        return NO_REQUEST;
    }
    else{
    {
    return BAD_REQUEST;
    }
    }
}

