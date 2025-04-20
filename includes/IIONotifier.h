#ifndef IIONOTIFIER_H
#define IIONOTIFIER_H

typedef enum e_notif {
    CLIENT_HUNG_UP,
	READY_TO_READ
} e_notif;

class IIONotifier {
  public:
	virtual ~IIONotifier(void) {}
	virtual void add(int socketfd, e_notif notif) = 0;
	virtual void del(int socketfd) = 0;
	virtual int wait(int* conn) = 0;
};


#endif // IIONOTIFIER_H
