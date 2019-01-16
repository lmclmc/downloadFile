#include "threadpool.h"
#include <sys/epoll.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/types.h>
       #include <sys/stat.h>
       #include <unistd.h>
#include "ypipe.hpp"

#define MAX_EPOLLSIZE (100000)
#define MAX_BUF_SIZE (65535)

class LWork{
public:
    LWork() = default;
    int init();
    int exec();

private:
    int epollFd;
    int sockFd;
};
//0x12345678
typedef struct{
    unsigned char PVersion;         //协议版本  0xFD
    unsigned char PFunction;        //协议功能  客户端发送 0：普通传输 1：断点续传 4:确认传输
                                           // 服务器发送 2:该文件不存在 3:该文件存在
    unsigned char PLength;               //数据体长度
}Protocol;

typedef struct{
    Protocol prot;              //
    int clientFd;
    char buf[64];
}Data;

zmq::ypipe_t<Data, 10000> memoryPool;
unsigned char buffer[2048];
int bufferCount = 0;
#include <signal.h>
int main(){
    signal(SIGPIPE, SIG_IGN);
    ThreadPool *t = ThreadPool::getInstance();
    t->addThread();
    t->addThread();
    t->addThread();
    t->addThread();
    t->addTask([](){
        Data data;
        while (1){
            if (memoryPool.read(&data)){
                if (data.prot.PFunction == 0){
                    printf("PFunction = %d, %s\n", data.prot.PFunction, "普通传输");
                    if (access(data.buf, F_OK) == 0){
                        printf("PFunction = %d, %s, %s\n", data.prot.PFunction, data.buf, "文件存在");
                        Protocol prot = {0xFD, 3, 12};
                        int fileFd = open(data.buf, O_RDWR, 0666);
                        struct stat buf;
                        bzero(&buf,sizeof(buf));
                        fstat(fileFd, &buf);
                        unsigned char tmp[20];
                        int c = 0x12345678;
                        long size = buf.st_size;
                        printf("filesize = %ld, %d\n", size, fileFd);
                        memcpy(tmp, &c, 4);
                        memcpy(tmp+4, &prot, 3);
                        memcpy(tmp+7, &size, 8);
                        memcpy(tmp+15, &fileFd, 4);
                        send(data.clientFd, tmp, sizeof(tmp), 0);
                    } else {
                        printf("PFunction = %d, %s, %s\n", data.prot.PFunction, data.buf, "文件不存在");
                        Protocol prot = {0xFD, 2, 0};
                        int c = 0x12345678;
                        unsigned char qwep[7];
                        memcpy(qwep, &c, 4);
                        memcpy(qwep+4, &prot, sizeof(Protocol));
                        printf("%d\n", send(data.clientFd, qwep, 7, 0));
                     //   close(data.clientFd);
                    }
                } else if (data.prot.PFunction == 4){
                    printf("PFunction = %d, %s, %s\n", data.prot.PFunction, data.buf, "确认传输");
                    ThreadPool *t = ThreadPool::getInstance();
                    t->addTask([](Data *data){
                        long fileSize = atol(data->buf);
                        printf("fileSize = %ld\n", fileSize);
                        unsigned char *tm = (unsigned char *)data->buf;
                        while (*(tm++) != 0){

                        }
                        int fileFd = atoi((const char *)tm);

                        int length;
                        long count = 0;

                        void *p = mmap(NULL, fileSize, PROT_READ|PROT_WRITE, MAP_SHARED, fileFd, 0);
                        while (1){
                            if (fileSize > MAX_BUF_SIZE){
                                length = send(data->clientFd, p+count, MAX_BUF_SIZE, 0);
                            } else {
                                length = send(data->clientFd, p+count, fileSize, 0);
                            }
                            if (errno == EPIPE){
                                munmap(p, 1024*1024*1024*4);
                                close(data->clientFd);
                                close(fileFd);
                                return;
                            }

                            if (length > 0){
                                count += length;
                                fileSize -= length;
                            }
                            if (fileSize <= 0){
                                munmap(p, 1024*1024*1024*4);
                                close(data->clientFd);
                                close(fileFd);
                                return;
                            }
                        }

                    }, &data);
                } else if (data.prot.PFunction == 1){
                    printf("PFunction = %d, %s, %s\n", data.prot.PFunction, data.buf, "断点传输");
                    if (access(data.buf, F_OK) == 0){
                        printf("PFunction = %d, %s, %s\n", data.prot.PFunction, data.buf, "文件存在");
                        Protocol prot = {0xFD, 4, 0};
                        ThreadPool *t = ThreadPool::getInstance();

                        unsigned char tmp[20];
                        int c = 0x12345678;

                        memcpy(tmp, &c, 4);
                        memcpy(tmp+4, &prot, 3);
                        send(data.clientFd, tmp, sizeof(tmp), 0);
                        t->addTask([](Data *data){
                            usleep(30000);
                            int fileFd = open(data->buf, O_RDWR, 0666);
                            struct stat buf;
                            bzero(&buf,sizeof(buf));
                            fstat(fileFd, &buf);
                            long fileSize = buf.st_size;

                            unsigned char *tm = (unsigned char *)data->buf;
                            while (*(tm++) != 0){

                            }
                            long count = atol((const char *)tm);

                            int length;

                            fileSize = fileSize-count;
                            void *p = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fileFd, 0);
                            while (1){
                                if (fileSize > MAX_BUF_SIZE){
                                    length = send(data->clientFd, p+count, MAX_BUF_SIZE, 0);
                                } else {
                                    length = send(data->clientFd, p+count, fileSize, 0);
                                }

                                if (errno == EPIPE){
                                    munmap(p, 1024*1024*1024*4);
                                    close(data->clientFd);
                                    close(fileFd);
                                    return;
                                }

                                if (length > 0){
                                    count += length;
                                    fileSize -= length;
                                }
                                if (fileSize <= 0){
                                    munmap(p, 1024*1024*1024*4);
                                    close(data->clientFd);
                                    close(fileFd);
                                    return;
                                }
                            }
                        }, &data);
                    } else {
                        printf("PFunction = %d, %s, %s\n", data.prot.PFunction, data.buf, "文件不存在");
                        Protocol prot = {0xFD, 2, 0};
                        int c = 0x12345678;
                        unsigned char qwep[7];
                        memcpy(qwep, &c, 4);
                        memcpy(qwep+4, &prot, sizeof(Protocol));
                        printf("%d\n", send(data.clientFd, qwep, 7, 0));
                     //   close(data.clientFd);
                    }
                }
            }
        }
    });

    LWork tLWork;
    int ret = tLWork.init();
    if (ret < 0){
        exit(0);
    }
    return tLWork.exec();
}


int LWork::init()
{
    epollFd = -120;
    sockFd = -120;
    int epollFd_ = epoll_create(MAX_EPOLLSIZE);
    int sockFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd_ < 0) {
        perror("socket");
        return -1;
    }  

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(20078);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockFd_, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
        perror("bind");
        return -2;
    }

    int opt = 1;
    setsockopt(sockFd_ ,SOL_SOCKET,SO_REUSEADDR, (char *)&opt, sizeof(opt));
    int sock_buf_size = MAX_BUF_SIZE;
    setsockopt( sockFd_, SOL_SOCKET, SO_SNDBUF,
                       (char *)&sock_buf_size, sizeof(sock_buf_size) );
    setsockopt( sockFd_, SOL_SOCKET, SO_RCVBUF,
                       (char *)&sock_buf_size, sizeof(sock_buf_size) );

    if (listen(sockFd_, 5) < 0) {
        perror("listen");
        return -3;
    }

    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLET; //EPOLLLT
    ev.data.fd = sockFd_;
    epoll_ctl(epollFd_, EPOLL_CTL_ADD, sockFd_, &ev);
    epollFd = epollFd_;
    sockFd = sockFd_;
    return 1;
}

int LWork::exec()
{
    struct epoll_event events[MAX_EPOLLSIZE];

    unsigned long count = 0;

    while (1){
        int nfds = epoll_wait(epollFd, events, MAX_EPOLLSIZE, 1);
        if (nfds == -1) {
            perror("epoll_wait");
            break;
        }

//        if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
//                        || !(events[i].events & EPOLLIN))
//                    {
//                        perror("error : socket fd error!\n");
//                        close(events[i].data.fd);
//                        continue;

//                    }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == sockFd){
                struct sockaddr_in client_addr;
                memset(&client_addr, 0, sizeof(struct sockaddr_in));
                socklen_t client_len = sizeof(client_addr);

                int clientfd = accept(sockFd, (struct sockaddr*)&client_addr, &client_len);
                if (clientfd < 0) {
                    perror("accept");
                    return -1;
                }

                int flags = fcntl(clientfd, F_GETFL, 0);
                flags |= O_NONBLOCK;
                fcntl(clientfd, F_SETFL, flags);

                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
                ev.data.fd = clientfd;
                epoll_ctl(epollFd, EPOLL_CTL_ADD, clientfd, &ev);
            } else {
                int clientFd = events[i].data.fd;
                int bufferSize = 2048;
                int length = recv(clientFd, buffer, bufferSize, 0);
                if (length <= 0){
                    continue;
                }

                unsigned char *pCheck = (unsigned char *)buffer;
                while (*(int *)(pCheck++) != 0x12345678){
                    if (pCheck >= buffer+2045){
                        break;
                    }
//                    bufferCount--;
//                    if (bufferCount < 0){
//                        bufferCount = 0;
//                        continue;
//                    }
                }
                pCheck += 3;
//                bufferCount -= 3;
//                if (bufferCount < 0){
//                    bufferCount = 0;
//                    continue;
//                }

                if (*pCheck++ == 0xFD){
                    Protocol prot = {0xFD, *pCheck, *(++pCheck)};
//                    bufferCount -= 3;
//                    bufferCount -= prot.PLength;
//                    if (bufferCount < 0){
//                        bufferCount = 0;
//                        continue;
//                    }

                    char buf[64] = {0};
                    pCheck++;
            //        memmove(buffer, pCheck+prot.PLength, buffer+2048-pCheck-prot.PLength);
                    memcpy(buf, pCheck, prot.PLength);
                    Data data;
                    memcpy(&data.prot, &prot, sizeof(prot));
                    data.clientFd = clientFd;
                    memcpy(data.buf, buf, sizeof(buf));
                    memoryPool.write(data, false);
                    memoryPool.flush();
                } else {
                    continue;
                }


//                while (1){
//                    if (size > MAX_BUF_SIZE){
//                        length = send(clientFd, p+count, MAX_BUF_SIZE, 0);
//                    } else {
//                        length = send(clientFd, p+count, size, 0);
//                    }

//                    if (length > 0){
//                        count += length;
//                        size -= length;
//                    }

//                    if (size <= 0){
//                        close(clientFd);
//                        struct epoll_event ev;
//                        ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
//                        ev.data.fd = clientFd;
//                        epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev);
//                    }

//                }
            }
        }
    }
}
