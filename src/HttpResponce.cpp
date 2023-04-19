
#include"../include/HttpResponce.h"

//用一个哈希表来维护所要上传的文件
std::unordered_map<std::string, MineType> mtypemap = {
        {".html", "text/html"},
        {".xml", "text/xml"},
        {".xhtml", "application/xhtml+xml"},
        {".txt", "text/plain"},
        {".ico","application/x-ico"},
        {".rtf", "application/rtf"},
        {".pdf", "application/pdf"},
        {".word", "application/msword"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".au", "audio/basic"},
        {".mpeg", "video/mpeg"},
        {".mpg", "video/mpeg"},
        {".avi", "video/x-msvideo"},
        {".gz", "application/x-gzip"},
        {".tar", "application/x-tar"},
        {".css", "text/css"},
        {".mp3","audio/mp3"},
        {"", "text/plain"},
        {"default","text/plain"}
};


void HttpResponce::appenbuffer(char*buffer)const{

    //版本
    if(mversion==HttpRequest::HTTP_11){
        sprintf(buffer,"HTTP/1.1 %d %s\r\n",mstatus,mstatusmsg.c_str());
    }else if(mversion==HttpRequest::HTTP_10){
        sprintf(buffer,"HTTP/1.0 %d %s\r\n",mstatus,mstatusmsg.c_str());
    }

    //头部字段
    for(auto& header : mheaders){
        sprintf(buffer,"%s%s: %s\r\n",buffer,header.first.c_str(),header.second.c_str());
    }   
    //类型单独拿出来 因为返回内容要做一个映射
    sprintf(buffer,"%sContent-type: %s\r\n",buffer,mtype.type.c_str()); 
    //keep value要单独拿出来 判断是否是长连接还是短连接
    if(mkeep_alive){
        sprintf(buffer,"%sConnection: keep-alive\r\n",buffer);//请求之后等待后续的连接
    }
    else{
        sprintf(buffer,"%sConnection: close\r\n",buffer);//请求之后关闭连接
    }
}
