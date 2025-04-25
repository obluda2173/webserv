# <span class="todo TODO">TODO</span> 

  - \[X\] Rename from socketfd/conn to fd
    ([EpollNotifier](includes/EpollIONotifier.h::void%20add\(int%20socketfd,%20e_notif%20notif\);))
  - \[X\] Do implement Ipv6 alongside Ipv4
  - \[ \] Persistence: send two httpMessage one after the other without
    cutting tcp connection
      - \[ \] make sure that no information is lost (http in series, but
        cut at an inappropriate point)
      - \[ \] there is a security problem (what happens if the request
        is sent byte by byte and it only end CLRFs )
  - \[ \] handle maximum amount of connections
  - \[ \] Make <sub>ioNotifier</sub> not only notify for one but
    configurable amount of events
    ([Listener.cpp](src/Listener.cpp::int%20fd;%20//%20TODO:%20take%20not%20only%20one%20connection%20but%20#ready%20connections))
  - \[ \] Have a check on how many file-descriptors (connections) can be
    added
  - \[ \] make sure to treat EPOLLRDHUP and EPOLLHUP appropriately
      - \[ \] implement a timeout mechanism
      - \[ \] writing
  - \[ \] build server with some configurations
  - \[ \] figure out stategie for error reporting (system call errors)
  - \[ \] make the test<sub>fixtures</sub> more TypeAgnostic
      - add more templates (IIONotifier, IConnectionHandler) and so on
