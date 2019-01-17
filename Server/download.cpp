#include "download.h"
#include "threadpool/threadpool.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <unistd.h>

Download::Download()
{
    ThreadPool *t = ThreadPool::getInstance();
    t->addTask([this](){
        while (1){
            std::list<std::pair<downloadData, std::function<bool(int, int, long &, long &, void *)>>>::iterator iter = this->downloadTask.begin();
            if (this->downloadTask.empty()) continue;

            for (auto &tmp : this->downloadTask){
                if (!tmp.second(tmp.first.fileFd, tmp.first.clientFd, tmp.first.fileSize, tmp.first.offset, tmp.first.p)){
                   this->downloadTask.erase(iter);
                   break;
                }
                iter++;
            }
        }
    });
}

void Download::addDownTask(int fileFd_, int clientFd, long fileSize_, long offset_, void *p)
{
    downloadData data = {fileFd_, clientFd, fileSize_, offset_, p};
    downloadTask.emplace_back(data, run);
}

bool Download::run(int fileFd, int clientFd, long &fileSize, long &offset, void *p)
{
    int length;
    if (fileSize > MAX_SOCKET_BUF_SIZE){
        length = send(clientFd, (unsigned char *)p+offset, MAX_SOCKET_BUF_SIZE, 0);
    } else {
        length = send(clientFd, (unsigned char *)p+offset, fileSize, 0);
    }
    if (EPIPE == errno || EBADF == errno){
        munmap(p, 4294967295);
        close(clientFd);
        close(fileFd);
        return false;
    }

    if (length > 0){
        offset += length;
        fileSize -= length;
    }
    if (fileSize <= 0){
        munmap(p, 4294967295);
        close(clientFd);
        close(fileFd);
        return false;
    }

    return true;
}
