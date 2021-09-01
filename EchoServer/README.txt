#engine

对Epoll进行封装抽象，我们将其称为Engine，还有回写服务器进行抽象。先自由抽象，
之后要求使用如下接口:

epoll接口：  
```C
    virtual ErrCode AddIoEvent(FD fd, int32_t mask, IIoHandler* handler, void* user_data) = 0;
    virtual void    DeleteIoEvent(FD fd, int32_t mask) = 0;
``` 
回调函数接口：  
```C
    virtual void OnRead(FD fd, void* data, int32_t mask) = 0;  
    virtual void OnWrite(FD fd, void* data, int32_t mask) = 0;  
```  