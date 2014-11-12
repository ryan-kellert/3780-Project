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
using namespace std;

#define MAX_BUFFER_SIZE 255
struct Packet
{
      string Seq_No; // 1 char
      string Type; // 1 char
      string Source; // 12
      string Destination; // 12  
      string Payload;
};

int main()
{

   Packet packet;
  

   // We will store our messages here.
   vector<Packet> Message_Storage;
   // The buffer will hold the messages on the server side.
   // Any messages above the size of 256 will be incomplete
   // and will therefore produce an error message. 
   char buffer[MAX_BUFFER_SIZE +1 ];


   int socket_descriptor;
   // The socket descriptor will be the return value, which
   // will be used in later system calls. 
   //
   // \param AF_INENET = Domain IPv4
   // \param SOCK_DGRAM = Type Datagram
   // \param 0 = Protocol UDP
   socket_descriptor= socket(AF_INET, SOCK_DGRAM, 0);
   if (socket_descriptor < 0)
   {
     //printf("Error opening the socket.");
      cerr<< "Error creating the socket."<<endl;
      return -1;
   }
   
   // Socket Address struct. 
   int port_number= 3500; 
   struct sockaddr_in server_address;
   server_address.sin_family= AF_INET; // Domain IPv4 protocol
   server_address.sin_port= htons(port_number); // htons?
   // INADDR_ANY binds the socket to the local IP address
   server_address.sin_addr.s_addr=htonl(INADDR_ANY); // should this be htons or htonl?

  
   int server_length= sizeof(server_address);
   // Now that we have a socket, we have to associate this socket with a port on the
   // machine we are using.
   //
   // \param socket_descriptor = our socket file descriptor.
   // \param server_address = pointer to struct sockaddr that contains info about
   // the servers address: Port and IP address.
   // \param server_length is the length in bytes of the previous parameters address. 
   int bindCheck;
   bindCheck=bind(socket_descriptor, (struct sockaddr *)&server_address,server_length);
   if (bindCheck<0)
   {
      cerr<<"Binding error."<<endl;
      return -1;
   }

   // Checks to see if recvfrom returns error -1.
   int receive_error_check;
   // Checks to see if sendto returns error -1.
   int send_error_check;
   struct sockaddr_in client_address;
   socklen_t client_length= sizeof(struct sockaddr_in);
   
   while(1)
   {
      // Receives a message from a client. Stores the message in
      // a buffer, and gives the client_address an IP address, port
      // and length. 
      //
      // \param socket_descriptor
      // \param buffer: the buffer that will hold the received message
      // \param strlen(buffer): the size of our buffer. Max sz of msg.
      // \param 0: Flags
      // \param client_address: A pointer to a yet to be initialized sockaddr
      // recvfrom fills in the client's IP address and port, to be used in
      // send to.
      // \param client_length: The length of the client address.
      cout<< "Waiting for messages... " << endl;
         
      receive_error_check= recvfrom( socket_descriptor,buffer,MAX_BUFFER_SIZE, 0,
				     (struct sockaddr *)&client_address,
				     &client_length);
      
      if(receive_error_check < 0)
      {
	 cerr<<"Failed to receive."<< buffer <<  endl;
      }
      int s=0;
      int d=0;
      int p=0;
      	 
      packet.Seq_No = buffer[0] - '0';
      packet.Type = buffer[1];
      for(int i=2; i<= MAX_BUFFER_SIZE; i++)
      {

	 if(i>=2 && i<14)
	 {
	    packet.Source[s]=buffer[i];
	    s++;
	 }
	 if(i>14 && i<25)
	 {
	    packet.Destination[d]= buffer[i];
	    d++;
	 }
	 else
	 {
	    packet.Payload[p]=buffer[i];
	    p++;
	 }
      }
	       
      
      
      //For testing purposes only, delete once complete. 
      /* cout<< "I received the following message:" << endl;
      for(int i=0; i< strlen(buffer); i++)
      {
	 cout<< buffer[i];
      }
      
      send_error_check= sendto( socket_descriptor, "Message Received \n",19, 0,
				(struct sockaddr *)&client_address,
				client_length);
      
      */
      
      Message_Storage.push_back(packet);
      switch ( buffer[1] )
      {
	 case 'G':
	    for(unsigned int i=0; i< Message_Storage.size(); i++)
	    {
	       if( Message_Storage[i].Destination == packet.Source)  
	       {
		  
		  int s=0;
		  int d=0;
		  int p=0;
		  
		  buffer[0]=packet.Seq_No[0];
		  buffer[1]=packet.Type[0];
		  for(int i=2; i<= MAX_BUFFER_SIZE; i++)
		  {
		     
		     if(i>=2 && i<14)
		     {
			buffer[i]=packet.Source[s];
			s++;
		     }
		     if(i>14 && i<25)
		     {
			buffer[i]=packet.Destination[d];
			 d++;
		     }
		     else
		     {
			buffer[i]=packet.Payload[p];
			p++;
		     }
		  }
		  
		  send_error_check= sendto( socket_descriptor, buffer, MAX_BUFFER_SIZE, 0,
					     (struct sockaddr *)&client_address,
					     client_length);
		   if(send_error_check < 0)
		   {
		      cerr<< "Problems sending."<< endl;
		   }
		   //reset buffer; 
		   break;
	       }
	    }
	 case 'S':
	    break;
	    
	 case 'A':
	    break;
	 default:
	    cerr<<"Type not valid."<<endl;
	    break;
      }
      

      // Datagram sockets are not connected to a remote host, so we just need to
      // send the destination address with our packet.
      //
      // \param socket_descriptor
      // \param buffer: the buffer holds our message, so we are passing in
      // the buffer to send our message.
      // \param strlen(buffer) informs the function that our message size
      // is the size of the buffer. For simplicity we will just send the
      // entire buffer for every message.
      // \param 0: Flags
      // \param client_address: pointer to the client's address which
      // contains the clients IP address and port. This destination
      // address is determined from the recvfrom() function prior. 
      // \param client_length: is the size of the previous parameter. 
       send_error_check= sendto( socket_descriptor, buffer,strlen(buffer), 0,
				(struct sockaddr *)&client_address,
				 client_length);
       if(send_error_check < 0)
       {
	  cerr<< "Problems sending."<< endl;
       }
       


     
   
   }

      
      
   return 0;

   
}

   

   


    
