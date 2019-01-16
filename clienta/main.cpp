#include <sys/epoll.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
       #include <sys/stat.h>
       #include <unistd.h>
#include <iostream>
#include <signal.h>

#define MAX_BUF_SIZE (60000)

char *gHostIp = nullptr;
char *gHostStr = nullptr;
char *gLocatStr = nullptr;
int gFileFd;
int gSockFd;
long breakCount = 0;
void *p = nullptr;

void Stop(int signo)
{
    if (p == nullptr){
        exit(0);
    }
    ::system("rm -rf ./.tmpFile");
    munmap(p, 1024*1024*1024*4);
    close(gFileFd);
    close(gSockFd);

    int tmpFileFd = open("./.tmpFile", O_RDWR | O_CREAT, 0666);
    char buffer[512];
    printf("%s %s %s %ld\n", gHostIp, gHostStr, gLocatStr, breakCount);
    sprintf(buffer, "%s %s %s %ld", gHostIp, gHostStr, gLocatStr, breakCount);
    write(tmpFileFd, buffer, strlen(buffer));
     close(tmpFileFd);
    _exit(0);
}

//0x12345678
typedef struct{
    unsigned char PVersion;         //协议版本  0xFD
    unsigned char PFunction;        //协议功能  客户端发送 0：普通传输 1：断点续传 4:确认传输
                                           // 服务器发送 2:该文件不存在 3:该文件存在
    unsigned char PLength;               //数据体长度
}Protocol;

unsigned char breakBuffer[512] = {0};

int main(int argc, char **argv){
    if (argc < 3){
        printf("argv\n");
        return 0;
    }

    signal(SIGINT, Stop);

    Protocol prot = {0xFD, 0, 40};
    unsigned char tmp[47] = {0};
    int c = 0x12345678;

    memcpy(tmp, &c, 4);

    char *aa = argv[2];
    printf("%s\n", aa);
    char fileName[256] = {0};
    if (strcmp(argv[1], "-C") == 0){
        int breakFd = open("./.tmpFile", O_RDWR | O_CREAT, 0666);
        if (breakFd == -1){
            printf("tmpFile not exit\n");
            return 0;
        }

        read(breakFd, breakBuffer, sizeof(breakBuffer));
        unsigned char *tmp0 = breakBuffer, *tmp1, *tmp2, *tmp3;
        int a = 0;
        for (int i=0; i<512; i++){
            if (*tmp0 == ' '){
                *tmp0 = 0;
                if (a == 0){
                    tmp1 = tmp0+1;
                } else if (a == 1){
                    tmp2 = tmp0+1;
                } else if (a == 2){
                    tmp3 = tmp0+1;
                }

                a++;
            }
            tmp0++;
        }

        gHostIp = (char *)breakBuffer;
        prot.PFunction = 1;
        breakCount = atol((char *)tmp3);
        gLocatStr = (char *)tmp2;
        gHostStr = (char *)tmp1;
        memset(tmp+7, 0, 40);
        memcpy(tmp+7, gHostStr, strlen(gHostStr));
        memcpy(fileName, gLocatStr, strlen(gLocatStr));
        sprintf((char *)tmp+8+strlen(gHostStr), "%ld", breakCount);
    } else {
        breakCount = 0;
        gHostIp = argv[1];
        gHostStr = argv[2];
        gLocatStr = argv[3];
        memcpy(tmp+7, aa, strlen(argv[2]));
        memcpy(fileName, gLocatStr, strlen(gLocatStr));
        strcat(fileName, ".tmp");
        gLocatStr = fileName;
    }

    int sockfd;
    struct sockaddr_in server;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("socket()error\n");
        exit(0);
    }
    memcpy(tmp+4, &prot, 3);






    memset(&server, 0, sizeof(server));
    server.sin_family= AF_INET;
    server.sin_port = htons(20078);
    server.sin_addr.s_addr = inet_addr(gHostIp);
    printf("11 ip = %s\n", gHostIp);
    if(connect(sockfd,(struct sockaddr *)&server,sizeof(server))==-1){
        printf("connect()error\n");
     //   exit(1);
    }
    int epollFd = epoll_create(5);
    int flags = fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    int sock_buf_size = MAX_BUF_SIZE;
    setsockopt( sockfd, SOL_SOCKET, SO_SNDBUF,
                       (char *)&sock_buf_size, sizeof(sock_buf_size) );
    setsockopt( sockfd, SOL_SOCKET, SO_RCVBUF,
                       (char *)&sock_buf_size, sizeof(sock_buf_size) );

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
    ev.data.fd = sockfd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, sockfd, &ev);




    send(sockfd, tmp, sizeof(tmp), 0);
    struct epoll_event events[10];
    bool status = false;
    long fileSize = 0;
    char buffer[128];
    printf("22 ip = %s\n", gHostIp);
    while (1){
        if (status){
            break;
        }
        int nfds = epoll_wait(epollFd, events, 10, 1);
        for (int i=0; i<nfds; i++){
            if (events[i].data.fd == sockfd){

                int length;
                while (length = recv(sockfd, buffer, sizeof(buffer), 0) < 0){
                    if (errno != EAGAIN){
                        close(sockfd);printf("222 ip = %s\n", gHostIp);
                        break;
                    }

                }

                printf("wo chu lai le\n");

                unsigned char *pCheck = (unsigned char *)buffer;
                while (*(int *)(pCheck++) != 0x12345678){

                }

                pCheck += 3;
                Protocol *prot = (Protocol *)pCheck;
                if (prot->PVersion != 0xFD){
                    break;
                }
                switch (prot->PFunction){
                case 2:
                    printf("file not exit\n");
                    exit(0);
                    break;
                case 3:{
                    printf("file exit\n");
                    prot->PFunction = 4;
                    prot->PVersion = 0xFD;
                    char str[20] = {0};
                    fileSize = *(long *)(pCheck+3);
                    sprintf(str, "%ld %d", fileSize, *(int *)(pCheck+11));
                    prot->PLength = strlen(str);
                    unsigned char *str1 = (unsigned char *)str;
                    while (*str1++ != ' '){}
                    *(--str1) = '\0';

                    printf("%s\n", str);
                    memcpy(tmp, &c, 4);
                    memcpy(tmp+4, prot, 3);
                    Protocol *a = (Protocol *)(tmp+4);

                    memcpy(tmp+7, str, sizeof(str));
                    printf("fileSize = %ld, %d\n", *(long *)(pCheck+3), *(int *)(pCheck+11));
                    send(sockfd, tmp, sizeof(tmp), 0);
                    printf("ensure\n");
                    status = true;
                    break;}
                case 4:
                    printf("ensure\n");
                    status = true;
                    break;
                }
            }
        }

    }

    int fd = open(fileName, O_RDWR | O_CREAT, 0666);
    if (fd < 0){
        perror("open");
    exit(1);
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    long time = tv.tv_sec;

struct stat buf;
        bzero(&buf,sizeof(buf));
        if (strcmp(argv[1], "-C") != 0) ftruncate(fd, fileSize);
        fstat(fd,&buf);
    fileSize = buf.st_size-breakCount;
    printf("fileSize = %ld\n", fileSize);
  p = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    perror("mmap");

    std::string strCount, strSpeed;
    while (1){
     //   int nfds = epoll_wait(epollFd, events, 100000, 1);
     //   if (nfds == -1) {
     //       perror("epoll_wait");
     //       break;
     //   }
  // send(sockfd, tmp, 1, 0);
     //   for (int i=0; i<nfds; i++){
        //    printf("nfds = %d\n", nfds);
     //       if (events[i].data.fd == sockfd){
            //    printf("recv\n");
                int length = recv(sockfd, p+breakCount, MAX_BUF_SIZE, 0);

                gFileFd = fd;
                gSockFd = sockfd;

             //   printf("length = %d, breakCount = %ld\n", length, breakCount);
                if (length < 0){
                    if (errno == EAGAIN) {
                     //   printf("%d\n", send(sockfd, p, 20, 0));
                        continue;
                    }
                    perror("recv");
                    munmap(p, 1024*1024*1024*4);
                    close(fd);
                    close(sockfd);
                    exit(0);
                } else {
                    breakCount = breakCount+length;
                }

                gettimeofday(&tv, NULL);
                if (tv.tv_sec != time){
                    time = tv.tv_sec;
                    static long lastCount = breakCount;

                    if (breakCount < 1024){
                        strCount = std::to_string(breakCount)+"B";
                    } else if (breakCount < 1024*1024){
                        strCount = std::to_string((double)breakCount/1024)+"K";
                    } else if (breakCount < 1024*1024*1024){
                        strCount = std::to_string((double)breakCount/(1024*1024))+"M";
                    } else {
                        strCount = std::to_string((double)breakCount/(1024*1024*1024))+"G";
                    }
                    long speed = breakCount-lastCount;
                    if (speed < 1024){
                        strSpeed = std::to_string(speed)+"B/S";
                    } else if (speed < 1024*1024){
                        strSpeed = std::to_string((double)speed/1024)+"K/S";
                    } else if (speed < 1024*1024*1024){
                        strSpeed = std::to_string((double)speed/(1024*1024))+"M/S";
                    }

                    printf("recvdata = %s, speed = %s\n", strCount.c_str(), strSpeed.c_str());
                    lastCount = breakCount;
                }

                fileSize -= length;
                if (fileSize <= 0){
                    munmap(p, 1024*1024*1024*4);
                    close(fd);
                    close(sockfd);
                    std::string sstr1 = "mv ";
            std::string sstr2 = fileName;
            std::string sstr3 = sstr2.substr(0,sstr2.find(".tmp", 1));
                    sstr1 = sstr1 + sstr2 + " " + sstr3;
                    ::system(sstr1.c_str());
                    exit(0);
                }
          //      send(sockfd, p, 20, 0);
           //     printf("send\n");
        //    }
      //  }

    }
    printf("Server Message: %s\n",buf);
    close(sockfd);
    return 0;
}
