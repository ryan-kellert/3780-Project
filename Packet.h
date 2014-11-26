#ifndef PACKET_H
#define PACKET_H
#include <string>
#include <netdb.h>


class Packet
{
    public:
        Packet(unsigned seq_number, char message_type, std::string src_name, std::string dest_name, std::string message);
        Packet(const char* packet_string);
        const char* GetPacketString();
        unsigned GetSequenceNumber();
        char GetMessageType();
        std::string GetSourceName();
        std::string GetDestinationName();
        std::string GetMessage();
        bool Send(int socket, sockaddr_in server_address);
        static Packet Receive(int socket, sockaddr_in server_address);
    protected:
    private:
        unsigned seq_num; //Sequence number of the packet
        //std::string s = std::bitset< 64 >( 12345 ).to_string();
        char message_type;
        std::string src_name;
        std::string dest_name;
        std::string message;
};

#endif // PACKET_H
