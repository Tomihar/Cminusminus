#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <vector>
#include <netinet/in.h>
#include <string>
#include <string.h>
//#include <unistd.h> //read
#include <thread>
#include <iostream>


#define PORT 8080

std::vector<int> sockets;
void task1(int clientSocket);

char *hello = "You are connected !!!";

struct header
{
	int msgId;
	int message_size;
	char login[16];
	char password[16];
};

struct user
{
	char login[16];
	char password[16];
};

int main(int argc, char const *argv[])
{
    int server_fd, new_client_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);



      
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
			sockets.push_back(new_client_socket);
            std::cout << "Connection successful ClientID="<<new_client_socket << std::endl;
            
        }

        std::thread t1 = std::thread(task1, new_client_socket);
		for(int i = 0; i < sockets.size(); i++)
		{
			std::cout<<sockets[i]<<std::endl;
		}
        //std::thread t1(task1, new_client_socket);
        //  t1.join();


        t1.detach();            
    }
}

void task1(int clientSocket)
{   
	int counter = 0;
	std::vector<user> users;
		users.push_back(user());
		strcpy(users[counter].login, "tomihar");
		strcpy(users[counter].password, "q1w2e3r4t");
		counter++;
    send(clientSocket , hello , strlen(hello) , 0 );
	char buffer[40] = {0};
	while(true)
	{
		memset(buffer, 0, sizeof(buffer));
		recv(clientSocket, buffer, 40, 0);
		header* msg = (header*)buffer;

		if(msg -> msgId == 1)
		{
			int is_used = 0;
			for(int i = 0; i < users.size(); i++)
			{
				is_used = strcmp(users[i].login, msg->login);
				if(is_used == 0)
				{
					char *msgr = "use";
					send(clientSocket , msgr , strlen(msgr) , 0 );
					continue;
				}
			}
			if(is_used == 0)
			{
				continue;
			}
			else
			{
			users.push_back(user());
			strcpy(users[counter].login, msg->login);
			strcpy(users[counter].password, msg->password);
			std::cout << "Registered user: " << msg->login << std::endl;
			counter++;
			char *msgr = "reg";
			send(clientSocket , msgr , strlen(msgr) , 0 );
			continue;
			}
		}
		if(msg -> msgId == 2)
		{
			int is_login_correct = 0;
			int is_password_correct = 0;
			for(int i = 0; i < users.size(); i++)
			{   is_login_correct = strcmp(users[i].login, msg->login);
				is_password_correct = strcmp(users[i].password, msg->password);
				if(is_login_correct == 0 && is_password_correct == 0)
				{
					char *logged = "logged";
					send(clientSocket , logged , strlen(logged) , 0 );
					std::cout << "Logged user: " << msg->login << std::endl;
					break;
				}
			}
			if(is_login_correct == 0 && is_password_correct == 0)
			{
				continue;
			}
			else
			{
			char *msgr = "error";
			send(clientSocket , msgr , strlen(msgr) , 0 );
			continue;
			}
		}
		if(msg -> msgId == 3)
		{
			char *message = new char[msg->message_size];
			recv(clientSocket,message,msg->message_size,0);
			std::cout<<msg->login<<" send: "<<message<< std::endl;
			if(strcmp(message, "exit") == 0)
			{
				break;
			}
			delete []message;
		}
	}
}



