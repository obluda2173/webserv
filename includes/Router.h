#ifndef IROUTER_H
#define IROUTER_H

// #include <>

class Router {
  const std::vector<ServerConfig>& _servers;  
  public:
    Router(const std::vector<ServerConfig>& servers) : _servers(servers) {}

    std::pair<const ServerConfig&, const LocationConfig*> match(const Request& req) {
        return {matched_server, matched_location};
    }
};

#endif // IROUTER_H
