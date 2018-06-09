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
#include <thread>
#define PORT 8080
//header wiadomosci
struct header
{
	int msgId;
	int message_size;
	char login[16];
	char password[16];
};
//wiadomosc od serwera
char srv_msg[3] = {0};

void task1(int clientSocket);
  
int main(int argc, char const *argv[])
{
	//deklaracja headera wiadomosci
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
	//utworzenie watku odbierajacego wiadomosci od serwera
    std::thread t_recieve = std::thread(task1, sock);
	t_recieve.detach();
// deklaracja zmiennej oznaczajacej dana opcje klienta
	int option = 0;
	while(true)
	{
		//opcja zerowa, podanie wlasciwej opcji
		if(option == 0)
		{
			printf("Register or login.\n For register write 1, for login write 3\n");	    
			std::cin>>option;
		}
		//opcja rejestracji
		if(option == 1)
		{
			//id = 1 - rejestracja
			msg.msgId = 1;
			printf("Write your login:\n");	    
			std::cin>>msg.login;
			printf("\nWrite your password:\n");	    
			std::cin>>msg.password;
			msg.message_size = 0;
			//wysylamy tylko header z danymi
			send(sock, &msg, sizeof(header), 0 );
			option = 2;
		}
		//sprawdzenie, czy rejestracja przebiegla pomyslnie
		if(option == 2)
		{
			int lol = strcmp(srv_msg, "err");
			if(lol == 0 && strlen(srv_msg) != 0)
			{
				std::cout<<"Login already in use"<<std::endl;
				option = 0;
				continue;
			}
			if(lol != 0 && strlen(srv_msg) != 0)
			{
				std::cout<<"You have been registered and logged"<<srv_msg<<std::endl;
				option = 5;
			}
		}
		//opcja logowania
		if(option == 3)
		{
			msg.msgId = 2;
			printf("Write your login:\n");	    
			std::cin>>msg.login;
			printf("\nWrite your password:\n");	    
			std::cin>>msg.password;
			msg.message_size = 0;
			send(sock, &msg, sizeof(header), 0 );
			option = 4;
		}
		//sprawdzenie, czy logowanie przebieglo pomyslnie
		if(option == 4)
		{
			int lol = strcmp(srv_msg, "err");
			if(lol == 0 && strlen(srv_msg) != 0)
			{
				std::cout<<"Wrong login or password"<<std::endl;
				option = 0;
			}
			if(lol != 0 && strlen(srv_msg) != 0)
			{
				std::cout<<"You have been logged"<<std::endl;
				option = 5;
			}
		}
		//wysylanie wiadomosci
		if(option == 5)
		{
			tekst = "";
			//ustawienie id wiadomosci na 3 = wiadomosc
			msg.msgId = 3;
			//wczytanie tresci wiadomosci do zmiennej
			std::getline( std::cin, tekst );
			//ustawienie wielkosci wiadomosci na dlugosc tekstu
			msg.message_size = tekst.length();
			//jezeli wpisano /exit, wyjdz z klienta
			if(tekst == "/exit")
			{
				send(sock, &msg, sizeof(header), 0 );
				send(sock , tekst.c_str() , tekst.length() , 0 );
				std::cout<<"Exiting client"<<std::endl;
				break;
			}
			//jezeli wiadomosci nie jest pusta, wyslij ja
			if(tekst.length() != 0)
			{
				//wyslanie headera
				send(sock, &msg, sizeof(header), 0 );
				//wyslanie wiadomosci (serwer odbierze to jako jedna wiadomosc)
				send(sock , tekst.c_str() , tekst.length() , 0 );
			}
			else
			{
				continue;
			}
		}
	}
}
//watek odbierania wiadomosci
void task1 (int sock)
{
	//ustawienie bufora do zaladowania headera
	char buffer[40] = {0};
	while(true)
	{
		//zerowanie headera
		memset(buffer, 0, sizeof(buffer));
		//odbieranie headera od serwera
		recv(sock, buffer, 40, 0);
		//rzutowanie headera na buffer
		header* srvmsg = (header*)buffer;
		//ustawienie wiadomosci od serwera w zaleznosci od ID wiadomosci, 1 = blad, 2 = dane poprawne
		if(srvmsg->msgId == 1)
		{
			strcpy(srv_msg, "err");
		}
		if(srvmsg->msgId == 2)
		{
			strcpy(srv_msg,"chk");
		}
		//odbieranie wiadomosci od serwera
		if(srvmsg->msgId == 3)
		{
			char *message = new char[srvmsg->message_size];
			recv(sock,message,srvmsg->message_size,0);
			std::cout<<srvmsg->login<<" send: "<<message<< std::endl;
			delete []message;
		}
	}
}




