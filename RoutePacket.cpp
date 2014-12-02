#include "RoutePacket.h"
#include <string>
#include <strings.h>
#include <stdexcept>
#include <iostream>


RoutePacket::RoutePacket(Server source, std::vector<Route> routing_table)
{
    this->src_id = source.id;
    this->routing_table = routing_table;
}

RoutePacket::RoutePacket(const char* route_packet_string)
{
    std::string packet (route_packet_string);
    this->src_id = std::stoul(packet.substr(0, 1));

    std::cout << "Packet Length is " << packet.length() << std::endl;
    //Each client hop sequence is 6 in length
    for(unsigned i = 1; (i + 6) < packet.length(); i += 6)
    {
        Route client;
        client.client_name = packet.substr(i, 5);
        client.hop_count = std::stoul(packet.substr(i+5));
        std::cout << "Client name is: " << client.client_name << std::endl
                  << "Hop count is: " << client.hop_count << std::endl;
        this->routing_table.push_back(client);
    }
}

RoutePacket::~RoutePacket()
{

}

const char* RoutePacket::GetPacketString()
{
    std::string packet_string;

    packet_string += std::to_string(this->src_id);

    for(unsigned i = 0; i < this->routing_table.size(); i++)
    {
        packet_string += this->routing_table[i].client_name;
        packet_string += std::to_string(this->routing_table[i].hop_count);
    }

    return packet_string.c_str();
}

RoutePacket RoutePacket::Receive(int socket, sockaddr_in server_address)
{
    struct sockaddr_in from_address;
    char message_buffer[1500];
    bzero(message_buffer, 1500);
    socklen_t from_length = sizeof(from_address);
    int num_bytes_received = recvfrom( socket,
                                      message_buffer,
                                      1500,
                                      0,
                                      (struct sockaddr *)&from_address,
                                      &from_length);
    if(num_bytes_received < 0)
    {
        throw std::runtime_error("Receive failed.");
    }

    RoutePacket received_packet(message_buffer);

    return received_packet;
}
