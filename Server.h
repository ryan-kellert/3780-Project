#ifndef SERVER_H
#define SERVER_H

#include <string>

class Server
{
    public:
        Server();
        virtual ~Server();
        unsigned id;
        std::string ip_addr;
    protected:
    private:
};

#endif // SERVER_H
