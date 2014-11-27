#define MAX_BUFFER_SIZE 255

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
#include <string.h>
#include <cstring>
#include <vector>
#include <limits>
#include "Packet.h"

using namespace std;

unsigned seq_num = 0;
vector<Packet> sent_messages;
vector<Packet> received_messages;

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("Server address and/or client name not given.\n");
        exit(1);
    }


    char buffer[MAX_BUFFER_SIZE + 1];

    int socket_descriptor;
    socket_descriptor= socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor < 0)
    {
        cerr<< "Error creating the socket." <<endl;
        return 0;
    }

    int port_number=3500;
    unsigned int server_length= sizeof(struct sockaddr_in);
    struct sockaddr_in server_address;
    struct hostent *host_ptr;

    host_ptr= gethostbyname(argv[1]); //IP changes with machine.
    if(host_ptr==0)
    {
        cerr<< "Wrong server address"<<endl;
        exit(1);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    // Make sure we understand what bcopy is doing.
    bcopy((char *)host_ptr->h_addr,
          (char *)&server_address.sin_addr, host_ptr->h_length);

    // Why don't we need server_address.sin_addr.s_addr=htonl(INADDR_ANY);


    int receive_error_check;
    int send_error_check;

    //struct sockaddr_int client_address;

    while(1)
    {
        // user inputs their message, the buffer sets to 000000
        bzero(buffer, 256);
        //could use fgets(buffer, 255, stdin). 255 + null terminator
        // setw limits the users input to 255 characters MAX.
        // cin>> setw(255)>> buffer;

        //only sending buffer to the first space of the message.
        char action;

        cout << "Awesome Networking program:\n\n\n"
             << "S: Send a message.\n"
             << "G: Retrieve messages from the server.\n"
             << "Q: Quit the program.\n"
             << "\nChoose an action: ";

        cin >> action;
        switch (toupper(action))
        {
        case 'S':
        {
            std::string message;
            std::string client_name;
            std::cout << "\n\nEnter a message: ";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::getline(std::cin, message);
            std::cout << "\nWho do you want to send to? ";
            std::cin >> client_name;
            Packet packet_to_send(++seq_num, 'S', argv[2], client_name, message);
            strcpy(buffer, packet_to_send.GetPacketString());

            //Send packet
            send_error_check= sendto( socket_descriptor, buffer, MAX_BUFFER_SIZE, 0,
                                      (struct sockaddr *)&server_address,
                                      server_length);
            if (send_error_check < 0)
            {
                cerr << "Error sending." << endl;
            }
            else
            {
                cout << "Message was sent..." << endl;
                sent_messages.push_back(packet_to_send);
            }
        }
        break;
        case 'G':
        {
            static unsigned last_seq_num;
            Packet packet_to_send(++seq_num, 'G', argv[2], "     ", " ");
            strcpy(buffer, packet_to_send.GetPacketString());

            //Send packet
            send_error_check= sendto( socket_descriptor, buffer, MAX_BUFFER_SIZE, 0,
                                      (struct sockaddr *)&server_address,
                                      server_length);

            if (send_error_check < 0)
            {
                cerr << "Error sending request." << endl;
            }
            else
            {
                Packet* current_packet;
                cout << "Get in Progress" << endl;
                do
                {
                    bzero(buffer, MAX_BUFFER_SIZE);
                    receive_error_check = recvfrom( socket_descriptor, buffer, 256, 0,
                                                    (struct sockaddr *)&server_address,
                                                    &server_length);

                    if (receive_error_check < 0)
                    {
                        cerr<< "Error receiving."<<endl;
                    }
                    else
                    {
                        Packet packet_received(buffer);
                        received_messages.push_back(packet_received);

                        for(unsigned i = 0; i < packet_received.size(); i++)
                        {
                            if(packet_received.GetSequenceNumber() > last_seq_num);
                            {
                                cout << packet_received.GetMessage() << endl;
                                break;
                            }
                            if(received_messages[i].GetSequenceNumber() == packet_received.GetSequenceNumber())
                            {

                            }
                        }
                        cout << "The following message was received: " << endl;

                        Packet ACK(++seq_num, 'A', argv[2], packet_received.GetSourceName(), itoa(packet_received.GetSequenceNumber());
                        current_packet = &packet_received;
                        send_error_check= sendto( socket_descriptor, ACK.GetPacketString(), MAX_BUFFER_SIZE, 0,
                                      (struct sockaddr *)&server_address,
                                      server_length);
                    }

                }while(current_packet->GetMessageType() != 'D');
            }
        }
        break;
        }



        // We put server address in both because there will only be client to
        // server communication.
        /*Packet packet_to_send(++seq_num, )

        for(unsigned i = 0; i < send_string.size(); i++)
        {
            buffer[i] = send_string[i];
        }


        // Only for testing, remove once complete.


        receive_error_check = recvfrom( socket_descriptor, buffer, 256, 0,
                                        (struct sockaddr *)&server_address,
                                        &server_length);

        if (receive_error_check<0)
        {
            cerr<< "Error receiving."<<endl;
            // Only for testing, remove once complete.
            cout<<"The following message was received: "<<endl;
            for (unsigned i = 26; i < MAX_BUFFER_SIZE; i++)
            {
                cout<<buffer[i];
            }
            cout << endl;
        }
        // Upon receiving message from the server. Send another message with
        // acknowledgement.*/
    }

    close(socket_descriptor);
    return 0;
}

