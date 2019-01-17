#include "wrapper.h"
#include "protocol.h"
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <memory>
#include "parsererror.h"
#include "download.h"

#define PORT (20078)

Wrapper::Wrapper() :
    bContinue(true)
{
    parsing = new Parsing();
    download = new Download();

    parsing->attach(std::make_shared<NormalAnalytical>());
    parsing->attach(std::make_shared<EnsureAnalytical>(download));
    parsing->attach(std::make_shared<ContinuinglyAnalytical>(download));
}

Wrapper::~Wrapper()
{
    if (nullptr != parsing) delete parsing;
    if (nullptr != download) delete download;
}

int Wrapper::init()
{
    epollFd = epoll_create(100);
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockFd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
        perror("bind");
        return -2;
    }

    int opt = 1;
    setsockopt(sockFd ,SOL_SOCKET,SO_REUSEADDR, (char *)&opt, sizeof(opt));
    int sock_buf_size = MAX_SOCKET_BUF_SIZE;
    setsockopt(sockFd, SOL_SOCKET, SO_SNDBUF,
                       (char *)&sock_buf_size, sizeof(sock_buf_size) );
    setsockopt(sockFd, SOL_SOCKET, SO_RCVBUF,
                       (char *)&sock_buf_size, sizeof(sock_buf_size) );

    if (listen(sockFd, 10) < 0) {
        perror("listen");
        return -3;
    }

    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLET; //EPOLLLT
    ev.data.fd = sockFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, sockFd, &ev);
}

int Wrapper::exec()
{
    struct epoll_event events[100];
    int nfds = 0;
    while (bContinue){
        try {
            nfds = epoll_wait(epollFd, events, 100, 1);
            if (nfds == -1){
                throw ParseError("epoll_wait");
            }

            for (int i=0; i<nfds; i++){
                if (events[i].data.fd == sockFd){
                    setProtocol(std::make_shared<SocketProtocol>(epollFd, sockFd));
                } else {
                    int clientFd = events[i].data.fd;
                    setProtocol(std::make_shared<ClientProtocol>(clientFd, &parsing->getMempool()));
                }
                protocol->run();
            }
        } catch (ParseError& e){
            std::cout << e.what() << std::endl;
            perror(" ");
        }

    }
}

void Wrapper::setProtocol(std::shared_ptr<Protocol> p)
{
    protocol = p;
}
