#define MAX_BUFFER_SIZE 255

#include <iostream>
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
using namespace std;

int main()
{
   char buffer[MAX_BUFFER_SIZE + 1];

   struct packet{
	int Seq_No;
	char Type;
	string Source;
	string Destination;
	char Payload[MAX_BUFFER_SIZE - 25]

   // Messages > Buffer.size() will produce an error message

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
   host_ptr= gethostbyname("142.66.140.80"); //IP changes with machine. 
   if(host_ptr==0)
   {
      cerr<< "Wrong server address"<<endl;
      return 0;
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
   {  // user inputs their message, the buffer sets to 000000
      cout<< "Enter a message: "<< endl;
      bzero(buffer, 256);
      //could use fgets(buffer, 255, stdin). 255 + null terminator
      // setw limits the users input to 255 characters MAX. 
      // cin>> setw(255)>> buffer;

      //only sending buffer to the first space of the message. 
      fgets(buffer, 255, stdin);
    
    
      // We put server address in both because there will only be client to
      // server communication. 
      send_error_check= sendto(socket_descriptor, buffer, strlen(buffer),0,
			      (const struct sockaddr *)&server_address,
			       server_length);
      if (send_error_check<0)
	 cerr<< "Error sending."<<endl;
      // Only for testing, remove once complete. 
      cout<<"Message was sent..."<<endl;

      receive_error_check = recvfrom( socket_descriptor, buffer, 256, 0,
				      (struct sockaddr *)&server_address,
				      &server_length);
      
      if (receive_error_check<0)
	 cerr<< "Error receiving."<<endl;
      // Only for testing, remove once complete. 
      cout<<"The following message was received: "<<endl;
      for(int i=0; i< 256; i++)
	 cout<<buffer[i];
      // Upon receiving message from the server. Send another message with
      // acknowledgement. 
   }

   close(socket_descriptor);			      
   return 0;
}

