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
#include <bitset>
#include <thread>
#include <future>
#include <atomic>
#include "Packet.h"
#include "Server.h"
#include "Route.h"
#include "RoutePacket.h"


Server self, left_server, right_server;

std::vector<Route> routing_table;

void DVR_Receive();
void DVR_Send();
void DVR_Client_Connected(std::string client_name);
void DVR_Forward(Server forward_to, Packet message);

int main(int argc, char *argv[])
{
    if(argc != 6)
    {
        std::cout << "Usage: Server my_server_id left_server_id left_server_ip right_server_id right_server_ip\n";
        exit(1);
    }

    //Assign neighbour server details for routing.
    self.id = atoi(argv[1]);
    self.ip_addr = "127.0.0.1";
    left_server.id = atoi(argv[2]);
    left_server.ip_addr.assign(argv[3]);
    right_server.id = atoi(argv[4]);
    right_server.ip_addr.assign(argv[5]);


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
    unsigned last_seq_num = 0; //Keep track of used sequence numbers.

    //Start up the Distance Vector Routing thread.
    std::future<void> receive = std::async(std::launch::async, DVR_Receive);

    //Wait for messages.
    while(true)
    {
        std::cout << "Waiting for Messages\n";
        Packet received_message = Packet::Receive(socket_descriptor, client_address);
        std::cout << "Message Received: " <<  received_message.GetMessage() << " of type " << received_message.GetMessageType() << std::endl;

        //Update Routing table if it was not a forwarded packet.
        if(received_message.GetMessageType() != 'F')
        {
            DVR_Client_Connected(received_message.GetSourceName());
        }

        //Handle different types of packets
        switch (received_message.GetMessageType())
        {
        case 'A': //ACK Packet
        {
            //The ACKed sequence number is stored in the message portion of the packet.
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
        case 'F': //Forwarded Packet
        case 'S': //Send Packet
            for(unsigned i = 0; i < routing_table.size(); i++)
            {
                if(routing_table[i].client_name == received_message.GetSourceName())
                {
                    if(routing_table[i].pass_to.id == self.id)
                    {
                        client_messages[received_message.GetDestinationName()].push_back(received_message);
                    }
                    else
                    {
                        DVR_Forward(routing_table[i].pass_to, received_message);
                    }
                }
            }
            break;
        case 'G': //Get Packet
        {
            for(unsigned i = 0; i < client_messages[received_message.GetSourceName()].size(); i++)
            {
                client_messages[received_message.GetSourceName()][i].Send(socket_descriptor, received_message.from_address);

                //If we are sending an ACK packet then we don't expect an ACK
                //for the ACK so just discard it right away.
                if(client_messages[received_message.GetSourceName()][i].GetMessageType() == 'A')
                {
                    client_messages[received_message.GetSourceName()].erase(client_messages[received_message.GetSourceName()].begin()+i);
                }
            }

            //Send a notice that we are done sending messages so the client doesn't get stuck in receive mode.
            Packet end_of_messages(++last_seq_num, 'D', "servr", received_message.GetSourceName(), "All messages sent");
            end_of_messages.Send(socket_descriptor, received_message.from_address);
        }
        break;
        default:
            std::cerr << "Invalid packet received.\n";
            break;
        }
    }
    close(socket_descriptor);
}

void DVR_Receive()
{
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
    unsigned port_number = 4500;
    server_address.sin_port= htons(port_number);
    server_address.sin_addr.s_addr=htonl(INADDR_ANY);
    int bind_error = bind(socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address));
    if(bind_error)
    {
        std::cerr << "Server cannot bind socket on port " << port_number << std::endl;
        exit(1);
    }


    while (true)
    {
        RoutePacket packet_received = RoutePacket::Receive(socket_descriptor, server_address);
        sleep(rand() & 10);
        for(unsigned i = 0; i < packet_received.routing_table.size(); i++)
        {
            unsigned j;
            for(j = 0; j < routing_table.size(); j++)
            {
                //Check if we both have entries for the same client.
                if(routing_table[j].client_name == packet_received.routing_table[i].client_name)
                {
                    //Only update if the new hop count is less than the path we already know.
                    if((packet_received.routing_table[i].hop_count + 1) < routing_table[j].hop_count)
                    {
                        std::cout << "New Client should be added.\n";
                        routing_table[j].hop_count = packet_received.routing_table[i].hop_count + 1;
                        //Update entry with which server to pass it through.
                        if(packet_received.src_id == left_server.id)
                        {
                            routing_table[j].pass_to = right_server;
                        }
                        else
                        {
                            routing_table[j].pass_to = left_server;
                        }
                    }
                    break; //Leave the inner loop after the entry is found.
                }
            }
            //If we didn't find an entry then j will be equal to the size of our routing table.
            //The new client and related information is added to our routing table.
            std::cout << "J is " << j << std::endl;
            if(j == routing_table.size())
            {
                Route new_client;
                new_client.client_name = packet_received.routing_table[i].client_name;
                new_client.hop_count = packet_received.routing_table[i].hop_count;
                if(packet_received.src_id == left_server.id)
                {
                    new_client.pass_to = left_server;
                }
                else
                {
                    new_client.pass_to = right_server;
                }
                routing_table.push_back(new_client);
            }
        }
        std::cout << "Routing Table Updated!! New Client List:\n\n";
        for (unsigned i = 0; i < routing_table.size(); i++)
        {
            std::cout << routing_table[i].client_name << routing_table[i].pass_to.ip_addr << std::endl;
        }
        DVR_Send();
    }
    close(socket_descriptor);
}

void DVR_Send()
{
    //Set up send parameters
    struct hostent* host_ptr;
    int socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor < 0)
    {
        std::cerr << "Error creating the socket.\n";
        exit(1);
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(4500);

    //Send update to server on logical left
    host_ptr = gethostbyname(left_server.ip_addr.c_str());
    bcopy((char *)host_ptr->h_addr,
          (char *)&server_address.sin_addr, host_ptr->h_length);

    RoutePacket packet_to_left(self, routing_table);
    packet_to_left.Send(socket_descriptor, server_address);

    //Send update to server on logical right
    host_ptr = gethostbyname(right_server.ip_addr.c_str());
    bcopy((char *)host_ptr->h_addr,
          (char *)&server_address.sin_addr, host_ptr->h_length);

    RoutePacket packet_to_right(self, routing_table);
    packet_to_right.Send(socket_descriptor, server_address);
    close(socket_descriptor);
}

void DVR_Client_Connected(std::string client_name)
{
    unsigned i;
    for(i = 0; i < routing_table.size(); i++)
    {
        if (routing_table[i].client_name == client_name)
        {
            routing_table[i].hop_count = 0;
            routing_table[i].pass_to = self;
            break;
        }
    }
    //i will be equal to the routing table size if the client does not exist in our routing table.
    if (i == routing_table.size())
    {
        Route new_client;
        new_client.client_name = client_name;
        new_client.hop_count = 0;
        new_client.pass_to = self;
        routing_table.push_back(new_client);
    }
    DVR_Send();
}

void DVR_Forward(Server forward_to, Packet message)
{
    //Set up send parameters
    struct hostent* host_ptr;
    int socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor < 0)
    {
        std::cerr << "Error creating the socket.\n";
        exit(1);
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(3500);

    //Send update to server on logical left
    host_ptr = gethostbyname(forward_to.ip_addr.c_str());
    bcopy((char *)host_ptr->h_addr,
          (char *)&server_address.sin_addr, host_ptr->h_length);

    message.SetMessageType('F');
    message.Send(socket_descriptor, server_address);
}
