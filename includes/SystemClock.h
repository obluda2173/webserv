#ifndef SYSTEMCLOCK_H
#define SYSTEMCLOCK_H

#include "IClock.h"
#include <unistd.h>
#include <sys/time.h>

class SystemClock : public IClock {
  public:
    timeval now() const {
        timeval now_;
        gettimeofday(&now_, NULL);
        return now_;
    }
};

#endif // SYSTEMCLOCK_H
