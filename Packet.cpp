#include "Packet.h"
#include <bitset>

Packet::Packet(unsigned seq_number, char message_type, std::string src_name, std::string dest_name, std::string message)
            : seq_num{seq_number}, message_type{message_type}, src_name{src_name}, dest_name{dest_name}, message{message}
{
    //ctor
}

Packet::Packet(const char* packet_string)
{
    std::string packet (packet_string);

    std::bitset<8> seq_num_binary_str (packet.substr(0, 8));
    this->seq_num = seq_num_binary_str.to_ulong();

    this->message_type = packet[8];

    this->src_name = packet.substr(9, 5);

    this->dest_name = packet.substr(14, 5);

    this->message = packet.substr(19);
}

const char* Packet::GetPacketString()
{
    std::string packet_string;
    packet_string += std::bitset<8>(this->seq_num).to_string(); //Convert seq_num to 8-bit binary string for transmission
    packet_string += this->message_type;
    packet_string += this->src_name;
    packet_string += this->dest_name;
    packet_string += this->message;
    return packet_string.c_str();
}

unsigned Packet::GetSequenceNumber()
{
    return this->seq_num;
}

char Packet::GetMessageType()
{
    return this->message_type;
}

std::string Packet::GetSourceName()
{
    return this->src_name;
}

std::string Packet::GetDestinationName()
{
    return this->dest_name;
}

std::string Packet::GetMessage()
{
    return this->message;
}
