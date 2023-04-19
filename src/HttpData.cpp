#include"../include/HttpData.h"
#include"../include/Singleton.hpp"
#include <memory>
// void HttpData::closeTimer(){
//     if(node.lock()){
//         auto tempnode=node.lock();
//         //及时析构 httpdata 用来关闭文件描述符
//         tempnode->data.reset();
//         //删除此定时器
//         Singleton<Timer>::getInstance()->delTimer(tempnode);
//         node.reset();
//     }
// }
void HttpData::setTimer(std::shared_ptr<TimerNode>node){
    this->node=node;
}
void HttpData::closeTimer() {
  if (node.lock()) //判断是否超时被释放了
  {
    std::shared_ptr<TimerNode> tempTimer(node.lock());
    tempTimer->deleted();
    // 断开weak_ptr
    node.reset();
  }
}