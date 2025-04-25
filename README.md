---
title: Readme
---

# <span class="todo TODO">TODO</span> 

- [x] Rename from socketfd/conn to fd
  ([EpollNotifier](includes/EpollIONotifier.h::void add(int socketfd, e_notif notif);))
- [x] Do implement Ipv6 alongside Ipv4
- [ ] Make <sub>ioNotifier</sub> not only notify for one but
  configurable amount of events
  ([Listener.cpp](src/Listener.cpp::int fd; // TODO: take not only one connection but #ready connections))
- [ ] Have a check on how many file-descriptors (connections) can be
  added
- [ ] send two httpMessage one after the other without cutting tcp
  connection
- [ ] make sure to treat EPOLLRDHUP and EPOLLHUP appropriately
  - [ ] implement a timeout mechanism
  - [ ] writing
- [ ] build server with some configurations
- [ ] figure out stategie for error reporting (system call errors)
- [ ] make the test<sub>fixtures</sub> more TypeAgnostic
  - add more templates (IIONotifier, IConnectionHandler) and so on
