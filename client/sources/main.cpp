// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <string.h>

#include <iostream>
#define PORT 8080

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
	std::string tekst;
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;

    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
	else
	{
		valread = read( sock , buffer, 1024);
		printf("Serwer: %s\n",buffer );
	}
	char option;
	while(true)
	{
		printf("Register or login.\n For register write 'r', for login write 'l'\n");	    
		std::cin>>option;
		if(option == 'r')
		{
			msg.msgId = 1;
			printf("Write your login:\n");	    
			std::cin>>msg.login;
			printf("\nWrite your password:\n");	    
			std::cin>>msg.password;
			msg.message_size = 0;
			send(sock, &msg, sizeof(header), 0 );
			memset(buffer, 0, sizeof(buffer));
			recv(sock, buffer, 1024, 0);
			int lol = strcmp(buffer, "reg");
			if(lol == 0)
			{
				std::cout<<"You have been registered and logged"<<std::endl;
				break;
			}
			else
			{
				std::cout<<"Login already in use"<<std::endl;
				continue;
			}
		}
		if(option == 'l')
		{
			msg.msgId = 2;
			printf("Write your login:\n");	    
			std::cin>>msg.login;
			printf("\nWrite your password:\n");	    
			std::cin>>msg.password;
			msg.message_size = 0;
			send(sock, &msg, sizeof(header), 0 );
			memset(buffer, 0, sizeof(buffer));
			recv(sock, buffer, 1024, 0);
			int lol = strcmp(buffer, "logged");
			std::cout<<lol<<buffer<<std::endl;
			if(lol == 0)
			{
				std::cout<<"You have been logged"<<std::endl;
				break;
			}
			else
			{
			std::cout<<"Wrong login or password"<<std::endl;
			}
		}
	}
	
	while(true)
	{
    std::getline( std::cin, tekst );
	if(tekst == "exit")
	{
		send(sock , tekst.c_str() , tekst.length() , 0 );
		printf("Exiting client\n\n");
		tekst = "Exiting server";
		break;
	}
    
    send(sock , tekst.c_str() , tekst.length() , 0 );
    printf("Message sent\n\n");
   
    //memset(buffer, 0, sizeof(buffer));
    //valread = read( sock , buffer, 1024);
    //printf("Serwer: %s\n",buffer );
	}
}




