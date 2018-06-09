#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <vector>
#include <netinet/in.h>
#include <string>
#include <string.h>
#include <thread>
#include <iostream>
#include <algorithm>
#include <mutex>
#include <fstream>


#define PORT 8080
//struktura wiadomosci 
struct header
{
	int msgId;
	int message_size;
	char login[16];
	char password[16];
};
//struktura uzytkownika
struct user
{
	char login[16];
	char password[16];
};
//wektor zawierajacy sockety
std::vector<int> sockets;
//wektor zawierajacy uzytkownikow
std::vector<user> users;
void task1(int clientSocket);
//mutex blokujacy dostep do pliku z uzytkownikami
std::mutex mtx;
//deklaracja headera wiadomosci
header srvmsg;
//filestream pliku z danymi uzytkownikow
std::fstream usersfile;
int main(int argc, char const *argv[])
{
    int server_fd, new_client_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
	//odczyt danych uzytkownikow zapisanych w pliku
	usersfile.open( "users.txt", std::ios::in );
	if( usersfile.good() )
    {
        std::string word;
        while( !usersfile.eof() )
        {
			//zapis danych z pliku do wektora z uzytkownikami
            getline(usersfile, word );
			users.push_back(user());
			strcpy(users[users.size()-1].login, word.c_str());
            getline(usersfile, word );
			strcpy(users[users.size()-1].password, word.c_str());
        }
        usersfile.close();
    } else std::cout << "Error! Unable to open a file!" << std::endl;


      
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
        t1.detach();            
    }
}
//fukcja obslugujaca watki klienckie
void task1(int clientSocket)
{   
//utworzenie bufora do odebrania headera
	char buffer[40] = {0};
	while(true)
	{
		//wyzerowanie bufora
		memset(buffer, 0, sizeof(buffer));
		//odebranie headera od klienta i rzutowanie go
		recv(clientSocket, buffer, 40, 0);
		header* msg = (header*)buffer;
//jezeli ID wiadomosci od klienta = 1, uzytkownik chce się zarejestrowac
		if(msg -> msgId == 1)
		{
			srvmsg.message_size = 0;
			int is_used = 0;
			//sprawdz, czy login nie jest juz zarejestrowany
			for(int i = 0; i < users.size(); i++)
			{
				is_used = strcmp(users[i].login, msg->login);
				if(is_used == 0)
				{
					//jezeli jest, id wiadomosci od serwera = 1, czyli blad.
					srvmsg.msgId = 1;
					send(clientSocket ,&srvmsg ,sizeof(header) , 0 );
					break;
				}
			}
			if(is_used == 0)
			{
				continue;
			}
			else
			{
				//jezeli login nie jest zarejestrowany, dodaj uzytkownika do wektora uzytkownikow
				users.push_back(user());
				strcpy(users[users.size()-1].login, msg->login);
				strcpy(users[users.size()-1].password, msg->password);
				std::cout << "Registered user: " << msg->login << std::endl;
				//wyslij wiadomosc o sukcesie
				srvmsg.msgId = 2;
				send(clientSocket ,&srvmsg ,sizeof(header) , 0 );
				//zapisz dane do pliku z uzytkownikami
				mtx.lock();
				usersfile.open("users.txt", std::ios::app );
				usersfile<<msg->login<<std::endl;
				usersfile<<msg->password<<std::endl;
				usersfile.close();
				mtx.unlock();
			continue;
			}
		}
		//jezeli id = 2 uzytkownik chce sie zalogowac
		if(msg -> msgId == 2)
		{
			srvmsg.message_size = 0;
			int is_login_correct = 0;
			int is_password_correct = 0;
			//sprawdz, czy uzytkownik podal poprawne dane
			for(int i = 0; i < users.size(); i++)
			{   is_login_correct = strcmp(users[i].login, msg->login);
				is_password_correct = strcmp(users[i].password, msg->password);
				if(is_login_correct == 0 && is_password_correct == 0)
				{
					//jezeli dane sa poprawne, wyslij wiadomosc o sukcesie (id = 2)
					srvmsg.msgId = 2;
					send(clientSocket ,&srvmsg ,sizeof(header) , 0 );
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
				//jezeli nie sa poprawne, wyslij wiadomosc o bledzie (id = 1)
				srvmsg.msgId = 1;
				send(clientSocket ,&srvmsg ,sizeof(header) , 0 );
				continue;
			}
		}
		//jezeli id = 3, uzytkownik wysyla wiadomosc
		if(msg -> msgId == 3)
		{
			//id wiadomosci od serwera = 3 - wyslij wiadomosc do wszystkich za wyjatkiem wysylajacego
			srvmsg.msgId = 3;
			//ustaw login wiadomosci os erwera na login wysylajacego
			strcpy(srvmsg.login, msg->login);
			//alokowanie pamieci na wiadomosc od klienta
			char *message = new char[msg->message_size];
			//odebranie wiadomosci
			recv(clientSocket,message,msg->message_size,0);
			//jezeli wiadomosc to "/exit", zakonccz watek klienta
			if(strcmp(message, "/exit") == 0)
			{
				//ustaw wiadomosc o wyjsciu klienta
				std::string left = msg->login + std::string(" left the server.");
				//ustaw wielkosc wiadomosci na dlugosc tekstu
				srvmsg.message_size = left.length();
				//wypisz wiadomosc o wyjsciu
				std::cout<<left<<std::endl;
				//wyslij do wszystkich za wyjatkiem wysylajacego
				for(int it = 0; it < sockets.size(); it++)
				{
					if(sockets[it] != clientSocket)
					{
						send(sockets[it] ,&srvmsg ,sizeof(header) , 0 );
						send(sockets[it] , left.c_str() ,left.length() , 0 );
					}
				}
				//uwolnij pamiec zaalokowana na wiadomosc
				delete []message;
				//usun socket klienta z wektora socketow
				for(int it = 0; it < sockets.size(); it++)
				{
					if(sockets[it] == clientSocket)
					{
						sockets.erase(sockets.begin() + it);
						break;
					}
				}
				break;
			}
			//jezeli uzytkownik wyslal wiadomosc, wypisz ja i wyslij do wszystkich za wyjatkiem jego
			srvmsg.message_size = strlen(message);
			std::cout<<msg->login<<" send: "<<message<< std::endl;
			for(int it = 0; it < sockets.size(); it++)
			{
				if(sockets[it] != clientSocket)
				{
					send(sockets[it] ,&srvmsg ,sizeof(header) , 0 );
					send(sockets[it] , message , strlen(message) , 0 );
				}
			}
			delete []message;
		}
	}
}




