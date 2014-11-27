#include <iostream>
#include <string>
#include <iomanip>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <vector>
#include <limits>
#include "Packet.h"

int main(int argc, char *argv[])
{
    //Storage for sent messages until they are acknowledged
    std::vector<Packet> sent_messages;


    // Should have the IP address of the server to connect to and the chosen
    // 5 character client name provided on the command line as program arguments.
    if (argc != 3 || strlen(argv[2]) != 5)
    {
        std::cout << "Usage: Client server_address 5_character_client_name\n";
        exit(1);
    }


    //Creating the socket. Program exits if an error occurs.
    int socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor < 0)
    {
        std::cerr << "Error creating the socket.\n";
        exit(1);
    }

    //Set up server connection stuff.
    struct sockaddr_in server_address;
    struct hostent* host_ptr = gethostbyname(argv[1]);
    if(host_ptr == 0)
    {
        std::cerr << "Could not establish connection to server.\n";
    }
    server_address.sin_family = AF_INET;
    unsigned port_number = 3500;
    server_address.sin_port = htons(port_number);
    bcopy((char *)host_ptr->h_addr,
          (char *)&server_address.sin_addr, host_ptr->h_length);


    //Main Program loop.
    char action = 'Z';
    unsigned last_seq_num = 0; //Stores the sequence number last used.

    std::cout << "Awesome Networking program:\n\n\n";
    do
    {
        std::cout << "S: Send a message.\n"
                  << "G: Retrieve messages from the server.\n"
                  << "Q: Quit the program.\n"
                  << "\nChoose an action: ";
        std::cin >> action;
        action = toupper(action);

        switch(action)
        {
        //Send a message
        case 'S':
            {
                std::string message;
                std::string client_name;
                std::cout << "\n\nEnter a message: ";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::getline(std::cin, message);
                std::cout << "\nWho do you want to send to? ";
                std::cin >> client_name;
                Packet message_packet(++last_seq_num, 'S', argv[2], client_name, message);
                bool sent = message_packet.Send(socket_descriptor, server_address);
                if(sent)
                {
                    sent_messages.push_back(message_packet);
                }
                else
                {
                    std::cerr << "An error occurred while attempting to send this message.\n";
                }
            }
            break;
        //Get messages from server
        case 'G':
            {
                Packet get_messages(++last_seq_num, 'G', argv[2], "servr", "Gimme my messages!");
                get_messages.Send(socket_descriptor, server_address);

                while(true)
                {
                    Packet received_packet = Packet::Receive(socket_descriptor, server_address);
                    if(received_packet.GetMessageType() == 'D')
                    {
                        //all messages sent by server
                        std::cout << "End of Messages.\n\n";
                        break;
                    }
                    else if(received_packet.GetMessageType() == 'A')
                    {
                        std::cout << "Acknowledgement received for sequence number " << received_packet.GetMessage() << std::endl;
                        //ummm...do something with an ACK?
                    }
                    else
                    {
                        Packet ACK(++last_seq_num, 'A', argv[2],
                                   received_packet.GetSourceName(),
                                   std::to_string(received_packet.GetSequenceNumber()));
                        ACK.Send(socket_descriptor, server_address);
                        std::cout << received_packet.GetMessage() << std::endl;
                    }
                }
            }
            break;
        //Quit the program
        case 'Q':
            break;
        default:
            std::cout << "Invalid selection made. Try again\n"
            << "Press any key to continue..." << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
            break;
        }
    }
    while(action != 'Q');
}
