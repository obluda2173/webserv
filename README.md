# <span class="todo TODO">TODO</span> 

  - \[X\] Rename from socketfd/conn to fd
    ([EpollNotifier](includes/EpollIONotifier.h::void%20add\(int%20socketfd,%20e_notif%20notif\);))
  - \[X\] Do implement Ipv6 alongside Ipv4
  - \[ \] Make <sub>ioNotifier</sub> not only notify for one but
    configurable amount of events
    ([Listener.cpp](src/Listener.cpp::int%20fd;%20//%20TODO:%20take%20not%20only%20one%20connection%20but%20#ready%20connections))
  - \[ \] send two httpMessage one after the other without cutting tcp
    connection
  - \[ \] make sure to treat EPOLLRDHUP and EPOLLHUP appropriately
