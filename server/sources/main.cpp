#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <string.h>
//#include <unistd.h> //read
#include <thread>
#include <iostream>


#define PORT 8080

void task1(int clientSocket);

char *hello = "You are connected !!!";

struct header
{
	int msgId;
	int message_size;
	char login[16];
	char password[16];
};

int main(int argc, char const *argv[])
{
	header msg;
    int server_fd, new_client_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
  
    const int noThread = 2;
    std::thread myThreads[noThread];



      
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    

    int Thread_counter = 0;

    while (true)
    {
        std::cout << "Listening" << std::endl;
        if ((new_client_socket = accept(server_fd, (struct sockaddr *)&address, 
                                                      (socklen_t*)&addrlen))<0)
        {
            perror("Cannot accept connection");
            exit(EXIT_FAILURE);
        }
        else 
        {
            std::cout << "Connection successful ClientID="<<new_client_socket << std::endl;
            
        }

        std::thread t1 = std::thread(task1, new_client_socket);
        //std::thread t1(task1, new_client_socket);
        //  t1.join();


        t1.detach();            
    }
}

void task1(int clientSocket)
{   
    send(clientSocket , hello , strlen(hello) , 0 );
	char buffer[1024] = {0};
	recv(clientSocket, buffer, 1024, 0);
	header* msg = (header*)buffer;
	if(msg -> msgId == 1)
	{
            std::cout << msg->login << std::endl;
	}
	while(true)
	{
	char buffer[1024] = {0};
   // int valread = read( clientSocket , buffer, 1024);
		int valread = recv(clientSocket,buffer,sizeof(buffer),0);
    //printf("%s\n",buffer );
		std::cout<<"Client_ID="<<clientSocket<<" send: "<<buffer<< std::endl;
		if(strcmp(buffer, "exit") == 0)
		{
			break;
		}
	}
}



