#ifndef PACKET_H
#define PACKET_H
#include <string>
#include <netdb.h>


class Packet
{
    public:
        Packet(unsigned seq_number, char message_type, std::string src_name, std::string dest_name, std::string message);
        Packet(const char* packet_string);
        virtual const char* GetPacketString();
        unsigned GetSequenceNumber();
        char GetMessageType();
        void SetMessageType(char message_type);
        std::string GetSourceName();
        std::string GetDestinationName();
        std::string GetMessage();
        bool Send(int socket, sockaddr_in server_address);
        static Packet Receive(int socket, sockaddr_in server_address);
        struct sockaddr_in from_address;
        socklen_t from_length;
    protected:
        Packet(); //Only to be used by child classes to bypass creation requirements.
    private:
        unsigned seq_num;
        char message_type;
        std::string src_name;
        std::string dest_name;
        std::string message;
};

#endif // PACKET_H
