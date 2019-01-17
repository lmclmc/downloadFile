#ifndef STRUCTURE_H
#define STRUCTURE_H

class Structure{
public:
    typedef enum class : unsigned char {
        Normal = 0,
        BreakPointContinue = 1,
        FileNotExit = 2,
        FileExit = 3,
        Ensure = 4,
        Resolve = 5
    }Function;

    typedef struct{
        unsigned char PVersion;         //协议版本  0xFD
        Function PFunction;        //协议功能  客户端发送 0：普通传输 1：断点续传 4:确认传输
                                                   // 服务器发送 2:该文件不存在 3:该文件存在
        unsigned char PLength;               //数据体长度
    }Protocol;

    typedef struct{
        Protocol prot;              //
        int clientFd;
        char buf[64];
    }Data;
};

#endif // STRUCTURE_H
