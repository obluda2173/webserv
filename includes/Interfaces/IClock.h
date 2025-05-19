#ifndef ICLOCK_H
#define ICLOCK_H

#include <sys/time.h>
class IClock {
  public:
    virtual ~IClock() {}
    virtual timeval now() const = 0;
};

#endif // ICLOCK_H
