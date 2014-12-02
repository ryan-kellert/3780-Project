#ifndef ROUTEPACKET_H
#define ROUTEPACKET_H

#include <vector>
#include "Packet.h"
#include "Route.h"
#include "Server.h"


class RoutePacket : public Packet
{
    public:
        RoutePacket(Server source, std::vector<Route> routing_table);
        RoutePacket(const char* route_packet_string);
        ~RoutePacket();
        const char* GetPacketString();
        static RoutePacket Receive(int socket, sockaddr_in server_address);
        std::vector<Route> routing_table;
        unsigned src_id;
    protected:
    private:
};

#endif // ROUTEPACKET_H
