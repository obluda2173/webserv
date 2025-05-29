#ifndef IIONOTIFIER_H
#define IIONOTIFIER_H

#include <vector>
typedef enum e_notif { CLIENT_HUNG_UP, READY_TO_READ, READY_TO_WRITE, BROKEN_CONNECTION, TIMEOUT } e_notif;

typedef struct t_notif {
    int fd;
    e_notif notif;
} t_notif;

class IIONotifier {
  public:
    virtual ~IIONotifier(void) {}
    virtual void add(int fd, long timeout_ms = 10000) = 0;
    virtual void addNoTimeout(int fd) = 0;
    virtual void del(int fd) = 0;
    virtual std::vector< t_notif > wait(void) = 0;
    virtual void modify(int fd, e_notif notif) = 0;
};

#endif // IIONOTIFIER_H
