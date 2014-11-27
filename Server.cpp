#include <iostream>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <vector>
#include <map>
#include "Packet.h"

int main(int argc, char *argv[])
{
    //Storage for messages until clients request them.
    std::map<std::string,std::vector<Packet>> client_messages;

    //Creating the socket. Program exits if an error occurs.
    int socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor < 0)
    {
        std::cerr << "Error creating the socket.\n";
        exit(1);
    }

    //Set up server connection stuff.
    struct sockaddr_in server_address;
    server_address.sin_family= AF_INET;
    unsigned port_number = 3500;
    server_address.sin_port= htons(port_number);
    server_address.sin_addr.s_addr=htonl(INADDR_ANY);
    int bind_error = bind(socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address));
    if(bind_error)
    {
        std::cerr << "Server cannot bind socket on port " << port_number << std::endl;
        exit(1);
    }

    struct sockaddr_in client_address;
    unsigned last_seq_num = 0;

    while(true)
    {
        std::cout << "Waiting for Messages\n";
        Packet received_message = Packet::Receive(socket_descriptor, client_address);
        std::cout << "Message Received: " <<  received_message.GetMessage() << std::endl;
        switch (received_message.GetMessageType())
        {
        case 'A':
        {
            unsigned ACK_seq_num = std::stoul(received_message.GetMessage());
            for(unsigned i = 0; i < client_messages[received_message.GetSourceName()].size(); i++)
            {
                if(client_messages[received_message.GetSourceName()][i].GetSequenceNumber() == ACK_seq_num)
                {
                    client_messages[received_message.GetSourceName()].erase(client_messages[received_message.GetSourceName()].begin()+i);
                }
            }
            client_messages[received_message.GetDestinationName()].push_back(received_message);
        }
        break;
        case 'S':
            client_messages[received_message.GetDestinationName()].push_back(received_message);
            break;
        case 'G':
        {
            for(unsigned i = 0; i < client_messages[received_message.GetSourceName()].size(); i++)
            {
                client_messages[received_message.GetSourceName()][i].Send(socket_descriptor, received_message.from_address);
            }
            Packet end_of_messages(++last_seq_num, 'D', "servr", received_message.GetSourceName(), "All messages sent");
            end_of_messages.Send(socket_descriptor, received_message.from_address);
        }
        break;
        default:
            std::cerr << "Invalid packet received.";
            break;
        }
    }
}
