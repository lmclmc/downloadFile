#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <list>
#include <functional>

#define MAX_SOCKET_BUF_SIZE (60000)

class Download
{
public:
    typedef struct {
        int fileFd;
        int clientFd;
        long fileSize;
        long offset;
        void *p;
    }downloadData;

    Download();

    void addDownTask(int fileFd_, int clientFd, long fileSize_, long offset_, void *p);

private:
    static bool run(int fileFd, int clientFd, long &fileSize, long &offset, void *p);

private:
    std::list<std::pair<downloadData, std::function<bool(int, int, long &, long &, void *)>>> downloadTask;
};

#endif // DOWNLOAD_H
