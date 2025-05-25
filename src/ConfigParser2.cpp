#include <ConfigParser.h>
#include <cstdlib>
#include <stdexcept>
#include <arpa/inet.h>

void ConfigParser::_validateFilename() {
    if (_filename.size() <= 5 || _filename.find(".conf") == std::string::npos) {
        throw std::runtime_error("Invalid file name");
    }
}

void ConfigParser::_parseWorkerConnections(const Directive& directive, EventsConfig& config) {
    if (directive.args.size() != 1) {
        throw std::runtime_error("worker_connections requires exactly one argument");
    }
    config.workerConnections = static_cast< size_t >(std::strtoul(directive.args[0].c_str(), NULL, 10));
    if (config.workerConnections > MAX_WORKER_CONNECTIONS) {
        throw std::runtime_error("worker_connections exceeded the maximum value");
    }
}

void ConfigParser::_parseUse(const Directive& directive, EventsConfig& config) {
    if (directive.args.size() != 1) {
        throw std::runtime_error("use requires exactly one argument");
    }
    if (directive.args[0] != "select" && directive.args[0] != "poll" && directive.args[0] != "epoll" &&
        directive.args[0] != "kqueue") {
        throw std::runtime_error("Unknown use method: " + directive.name);
    }
    config.kernelMethod = directive.args[0];
}

void ConfigParser::_parseListen(const Directive& directive, ServerConfig& config) {
    if (directive.args.empty()) {
        throw std::runtime_error("Missing argument for listen directive");
    }
    for (std::vector<std::string>::const_iterator it = directive.args.begin(); 
         it != directive.args.end(); ++it) {
        const std::string& arg = *it;
        std::string addr;
        std::string port_str = "80";
        bool is_ipv6 = false;

        if (!arg.empty() && arg[0] == '[') { // IPv6
            is_ipv6 = true;
            size_t closing_bracket = arg.find(']', 1);
            
            if (closing_bracket == std::string::npos) {
                throw std::runtime_error("Invalid IPv6 format: missing closing ']'");
            }
            
            addr = arg.substr(1, closing_bracket - 1);
            
            if (arg.size() > closing_bracket + 1) {
                if (arg[closing_bracket + 1] != ':') {
                    throw std::runtime_error("Invalid character after IPv6 address");
                }
                port_str = arg.substr(closing_bracket + 2);
            }
        } else { // IPv4
            size_t colon_pos = arg.find_last_of(':');
            if (colon_pos != std::string::npos) {
                addr = arg.substr(0, colon_pos);
                port_str = arg.substr(colon_pos + 1);
            } else {
                if (arg.find_first_of(".:") != std::string::npos) {
                    addr = arg; 
                } else {
                    port_str = arg;
                }
            }
        }

        if (addr.empty()) {
            addr = is_ipv6 ? "::" : "0.0.0.0";
        }

        char buffer[sizeof(struct in6_addr)];
        int result = is_ipv6 ? inet_pton(AF_INET6, addr.c_str(), buffer) : inet_pton(AF_INET, addr.c_str(), buffer);
        
        if (result != 1) {
            throw std::runtime_error("Invalid IP address: " + addr);
        }
        char* end;
        long port = std::strtol(port_str.c_str(), &end, 10);
        if (*end != '\0' || port < 1 || port > 65535) {
            throw std::runtime_error("Invalid port: " + port_str);
        }
        config.listen.insert(std::make_pair(addr, static_cast<int>(port)));
    }
}

void ConfigParser::_parseServerNames(const Directive& directive, ServerConfig& config) {
    if (directive.args.empty()) {
        throw std::runtime_error("Missing argument for server_name directive");
    }
    for (size_t i = 0; i < directive.args.size(); ++i) {
        config.serverNames.push_back(directive.args[i]);
    }
}

void ConfigParser::_parseCgiExt(const Directive& directive, LocationConfig& config) {
    if (directive.args.size() != 2) {
        throw std::runtime_error("cgi_ext requires exactly two arguments");
    } else if (directive.args[0][0] != '.') {
        throw std::runtime_error("Invalid cgi_ext argument: " + directive.args[0]);
    } else if (directive.args[1][0] != '/') {
        throw std::runtime_error("Invalid cgi_ext argument: " + directive.args[1]);
    } else if (config.cgi.find(directive.args[0]) != config.cgi.end()) {
        throw std::runtime_error("Duplicate cgi_ext extension: " + directive.args[0]);
    }
    config.cgi.insert(std::pair< std::string, std::string >(directive.args[0], directive.args[1]));
}

void ConfigParser::_parseRedirection(const Directive& directive, LocationConfig& config) {
    if (directive.args.size() != 2) {
        throw std::runtime_error("return requires exactly two arguments");
    }
    
    char* end;
    unsigned long code;
    const std::vector< std::string >& args = directive.args;
    code = strtoul(args[0].c_str(), &end, 10);
    if (*end != '\0' || args[0].empty() || code < 300 || code > 399) {
        throw std::runtime_error("Invalid return status code value: " + args[0]);
    }
    if (args[1].find_first_of("^~") != std::string::npos) {
       throw std::runtime_error("Regexp is not supported: " + args[1]);
    }
    config.redirect = std::make_pair(code, args[1]);
}

void ConfigParser::_parseRoot(const Directive& directive, CommonConfig& config) {
    if (directive.args.size() != 1) {
        throw std::runtime_error("Invalid root directive: exactly one argument required");
    }
    config.root = directive.args[0];
}

void ConfigParser::_parseClientMaxBodySize(const Directive& directive, CommonConfig& config) {
    if (directive.args.size() != 1) {
        throw std::runtime_error("client_max_body_size requires exactly one argument");
    }
    char* end;
    const std::string& arg = directive.args[0];
    unsigned long value = std::strtoul(arg.c_str(), &end, 10);
    if (end == arg.c_str()) {
        throw std::runtime_error("Invalid client_max_body_size value: " + arg);
    }
    char unit = *end;
    if (unit != '\0') {
        if (end + 1 != arg.c_str() + arg.size()) {
            throw std::runtime_error("Invalid unit in client_max_body_size: " + std::string(end));
        }
        unit = tolower(unit);
        if (unit == 'k')
            value *= oneKB;
        else if (unit == 'm')
            value *= oneMB;
        else if (unit == 'g')
            value *= oneGB;
        else
            throw std::runtime_error("Invalid unit in client_max_body_size: " + std::string(1, unit));
    }
    config.clientMaxBody = static_cast< size_t >(value);
}

void ConfigParser::_parseAllowMethods(const Directive& directive, CommonConfig& config) {
    if (directive.args.size() == 0) {
        throw std::runtime_error("allow_methods requires more than 0 arguments");
    }
    for (size_t i = 0; i < directive.args.size(); ++i) {
        if (directive.args[i] != "GET" && directive.args[i] != "POST" && directive.args[i] != "DELETE") {
            throw std::runtime_error("invalid http method: " + directive.args[i]);
        }
        config.allowMethods.push_back(directive.args[i]);
    }
}

void ConfigParser::_parseIndex(const Directive& directive, CommonConfig& config) {
    if (directive.args.size() == 0) {
        throw std::runtime_error("index requires more than 0 arguments");
    }
    for (size_t i = 0; i < directive.args.size(); ++i) {
        config.index.push_back(directive.args[i]);
    }
}

void ConfigParser::_parseErrorPage(const Directive& directive, CommonConfig& config) {
    const std::vector< std::string >& args = directive.args;
    const size_t argCount = args.size();

    if (argCount < 2) {
        throw std::runtime_error("error_page directive requires at least 2 arguments");
    }

    const std::string& uri = args.back();
    if (uri.empty() || (uri[0] != '/' && uri.find("://") == std::string::npos)) {
        throw std::runtime_error("Invalid error_page URI: " + uri);
    }

    for (size_t i = 0; i < argCount - 1; ++i) {
        const std::string& code_str = args[i];
        const char* start = code_str.c_str();
        char* end;
        long code = std::strtoul(start, &end, 10);

        if (*end != '\0' || code_str.empty()) {
            throw std::runtime_error("Invalid error_page code format: " + args[i]);
        } else if (code < 300 || code > 599) {
            throw std::runtime_error("Invalid error_page code at line " + args[i]);
        } else if (config.errorPage.find(code) != config.errorPage.end()) {
            throw std::runtime_error("Duplicate error_page code: " + args[i]);
        }
        config.errorPage[static_cast< int >(code)] = uri;
    }
}

void ConfigParser::_parseAutoindex(const Directive& directive, CommonConfig& config) {
    if (directive.args.size() != 1) {
        throw std::runtime_error("autoindex requires exactly one argument");
    }
    if (directive.args[0] == "on") {
        config.autoindex = true;
    } else if (directive.args[0] != "off") {
        throw std::runtime_error("Invalid autoindex argument: " + directive.args[0]);
    }
}
