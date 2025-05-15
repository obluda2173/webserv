#ifndef IIONOTIFIER_H
#define IIONOTIFIER_H

typedef enum e_notif { CLIENT_HUNG_UP, READY_TO_READ, READY_TO_WRITE, BROKEN_CONNECTION } e_notif;

class IIONotifier {
  public:
    virtual ~IIONotifier(void) {}
    virtual void add(int fd) = 0;
    virtual void del(int fd) = 0;
    virtual int wait(int* fds, e_notif* notif) = 0;
    virtual void modify(int fd, e_notif notif) = 0;
};

#endif // IIONOTIFIER_H
