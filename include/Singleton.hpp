//单例模板类 为每一个单例的设置 静态函数中初始化静态成员是线程安全的并且不会重复创建
template<typename T>
struct Singleton
{
    static T*getInstance(){
        static T ins;
        return &ins;
    }
    T* operator->() const { 
        return getInstance();
    }
};
