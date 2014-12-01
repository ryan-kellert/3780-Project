#ifndef ROUTE_H
#define ROUTE_H

#include <string>
#include "Server.h"

class Route
{
    public:
        Route();
        virtual ~Route();
        std::string client_name;
        unsigned hop_count;
        Server pass_to;
    protected:
    private:
};

#endif // ROUTE_H
