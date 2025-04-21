---
title: Readme
---

# <span class="todo TODO">TODO</span> 

- [x] Rename from socketfd/conn to fd
  ([EpollNotifier](includes/EpollIONotifier.h::void add(int socketfd, e_notif notif);))
- [ ] Do implement Ipv6 alongside Ipv4
- [ ] Make <sub>ioNotifier</sub> not only notify for one but
  configurable amount of events
  ([Listener.cpp](src/Listener.cpp::int fd; // TODO: take not only one connection but #ready connections))
- [ ] write a test for configuration and accepting multiple events
  ([epollNotifier
  test](tests/unittests/test_EpollIONotifier.cpp::// TODO: write a test for configuration and accepting multiple events))
