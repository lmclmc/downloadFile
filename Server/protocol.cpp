#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>

Protocol::Protocol()
{

}

SocketProtocol::SocketProtocol(int epollFd_, int sockFd_) :
    epollFd(epollFd_),
    sockFd(sockFd_)
{
    printf("SocketProtocol\n");
}

SocketProtocol::~SocketProtocol()
{
    printf("~SocketProtocol\n");
}

void SocketProtocol::run()
{
    printf("SocketProtocol run\n");
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_len = sizeof(client_addr);

    int clientfd = accept(sockFd, (struct sockaddr*)&client_addr, &client_len);
    if (clientfd < 0) {
        perror("accept");
        return;
    }

    int flags = fcntl(clientfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(clientfd, F_SETFL, flags);

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
    ev.data.fd = clientfd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, clientfd, &ev);
}

ClientProtocol::ClientProtocol(int clientFd_, zmq::ypipe_t<Structure::Data, 10000> *m) :
    clientFd(clientFd_)
{
    mem = m;
}

ClientProtocol::~ClientProtocol()
{
}

void ClientProtocol::run()
{
    char buffer[1024];
    int length = recv(clientFd, buffer, sizeof(buffer), 0);
    if (length <= 0){
        if (errno != EAGAIN){
            close(clientFd);
        }
        return;
    }

    unsigned char *pCheck = (unsigned char *)buffer;
    while (*(int *)(pCheck++) != 0x12345678){
        if (pCheck >= (unsigned char *)buffer+2045){
            break;
        }

    }
    pCheck += 3;

    if (*pCheck++ == 0xFD){
        Structure::Protocol prot = {0xFD, (Structure::Function)*pCheck, *(++pCheck)};


        char buf[64] = {0};
        pCheck++;

        memcpy(buf, pCheck, prot.PLength);
        Structure::Data data;
        memcpy(&data.prot, &prot, sizeof(prot));
        data.clientFd = clientFd;
        memcpy(data.buf, buf, sizeof(buf));
        mem->write(data, false);
        mem->flush();
    }
}
