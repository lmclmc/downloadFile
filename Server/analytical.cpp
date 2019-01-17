#include "analytical.h"
#include "download.h"

#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

Analytical::Analytical(Structure::Function f_) :
    f(f_)
{

}

NormalAnalytical::NormalAnalytical() :
    Analytical(Structure::Function::Normal)
{

}

void NormalAnalytical::analytical(Structure::Data &data)
{
    printf("PFunction = %d, %s\n", data.prot.PFunction, "普通传输");
    if (access(data.buf, F_OK) == 0){
        printf("PFunction = %d, %s, %s\n", data.prot.PFunction, data.buf, "文件存在");
        Structure::Protocol prot = {0xFD, (Structure::Function)3, 12};
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
        Structure::Protocol prot = {0xFD, (Structure::Function)2, 0};
        int c = 0x12345678;
        unsigned char qwep[7];
        memcpy(qwep, &c, 4);
        memcpy(qwep+4, &prot, sizeof(Structure::Protocol));
        send(data.clientFd, qwep, 7, 0);
     //   close(data.clientFd);
    }
}

EnsureAnalytical::EnsureAnalytical(Download *d) :
    Analytical(Structure::Function::Ensure),
    pDownload(d)
{

}

void EnsureAnalytical::analytical(Structure::Data &data)
{
    printf("PFunction = %d, %s, %s\n", data.prot.PFunction, data.buf, "确认传输");
    long fileSize = atol(data.buf);
    unsigned char *tm = (unsigned char *)data.buf;
    while (*(tm++) != 0){

    }
    int fileFd = atoi((const char *)tm);

    long count = 0;

    void *p = mmap(NULL, fileSize, PROT_READ|PROT_WRITE, MAP_SHARED, fileFd, 0);

    pDownload->addDownTask(fileFd, data.clientFd, fileSize, count, p);
}

ContinuinglyAnalytical::ContinuinglyAnalytical(Download *d) :
    Analytical(Structure::Function::BreakPointContinue),
    pDownload(d)
{

}

void ContinuinglyAnalytical::analytical(Structure::Data &data)
{
    printf("PFunction = %d, %s, %s\n", data.prot.PFunction, data.buf, "断点传输");
    if (access(data.buf, F_OK) == 0){
        printf("PFunction = %d, %s, %s\n", data.prot.PFunction, data.buf, "文件存在");
        Structure::Protocol prot = {0xFD, (Structure::Function)4, 0};

        unsigned char tmp[20];
        int c = 0x12345678;

        memcpy(tmp, &c, 4);
        memcpy(tmp+4, &prot, 3);
        send(data.clientFd, tmp, sizeof(tmp), 0);
        usleep(30000);
        int fileFd = open(data.buf, O_RDWR, 0666);
        struct stat buf;
        bzero(&buf,sizeof(buf));
        fstat(fileFd, &buf);
        long fileSize = buf.st_size;

        unsigned char *tm = (unsigned char *)data.buf;
        while (*(tm++) != 0){

        }
        long count = atol((const char *)tm);

        fileSize = fileSize-count;
        void *p = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fileFd, 0);

        pDownload->addDownTask(fileFd, data.clientFd, fileSize, count, p);
    } else {
        printf("PFunction = %d, %s, %s\n", data.prot.PFunction, data.buf, "文件不存在");
        Structure::Protocol prot = {0xFD, (Structure::Function)2, 0};
        int c = 0x12345678;
        unsigned char qwep[7];
        memcpy(qwep, &c, 4);
        memcpy(qwep+4, &prot, sizeof(Structure::Protocol));
        send(data.clientFd, qwep, 7, 0);
     //   close(data.clientFd);
    }
}
