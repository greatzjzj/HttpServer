#engine

��Epoll���з�װ�������ǽ����ΪEngine�����л�д���������г��������ɳ���
֮��Ҫ��ʹ�����½ӿ�:

epoll�ӿڣ�  
```C
    virtual ErrCode AddIoEvent(FD fd, int32_t mask, IIoHandler* handler, void* user_data) = 0;
    virtual void    DeleteIoEvent(FD fd, int32_t mask) = 0;
``` 
�ص������ӿڣ�  
```C
    virtual void OnRead(FD fd, void* data, int32_t mask) = 0;  
    virtual void OnWrite(FD fd, void* data, int32_t mask) = 0;  
```  