#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;
#pragma comment(lib, "Ws2_32.lib")


#define DEFAULT_PORT "11000"

void SEND_MESSAGE(const SOCKET s, const char* buff, int len)
{
	int iresult = send(s, buff, len, 0);
	if (iresult == SOCKET_ERROR) {
	
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		getchar();
		exit(1);
	}
	else if (iresult > 0)
	{
		cout << "Sent : " << iresult << " Bytes" << endl;
	}
}

void RECV_MESSAGE(const SOCKET s, char buff[], int len)
{
	int iresult = recv(s, buff, len, 0);
	if (iresult == 0)
	{
		cout << "Connection Closing" << endl;
	}
	else if (iresult < 0)
	{
		printf("recv failed: %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		getchar();
		exit(1);
	}
	else if (iresult > 0)
	{
		for (int i = 0; i < iresult; ++i)
			cout << buff[i];

		cout << endl;
	}
}




void initialize_WSA()
{
	WSADATA wsadata;
	int iresult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iresult != 0)
	{
		std::cout << "WSA Startup failed with iresult code" << iresult;
		exit(1);
	}

}

void client_things()
{
	int temp;

	addrinfo *ptr= nullptr, *result = nullptr, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	

	int iresult = getaddrinfo("localhost", DEFAULT_PORT, &hints, &result);
	if (iresult != 0)
	{
		cout << "get addrinfo failed " << endl;
		WSACleanup();
		//getchar();
		cin >> temp;
		exit(1);
	}

	SOCKET clientsocket = INVALID_SOCKET;

	//getting first addrinfo from linked list of addrinfo
	
	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{

		clientsocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (clientsocket == INVALID_SOCKET)
		{
			std::cout << "Invalid client socket" << std::endl;
			freeaddrinfo(result);
			WSACleanup();
			//getchar();
			cin >> temp;
			exit(1);
		}

		iresult = connect(clientsocket, ptr->ai_addr, ptr->ai_addrlen);

		if (iresult == SOCKET_ERROR)
		{
			std::cout << "Unable to connect to server " << WSAGetLastError()<<endl;
			closesocket(clientsocket);
			clientsocket = INVALID_SOCKET;
			//getchar();
			
		}
		//if we reach this place that means connection was successful
		break;
	}

	if (clientsocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		cin >> temp;
		exit(1);
	}

	char *recvbuffer = new char[512];

	
	//free the result linked list some other time lmao


	//
	while (1)
	{
		cout << "Waiting for message : " << endl;

		iresult = recv(clientsocket, recvbuffer, 512, 0);
		if (iresult > 0)
		{
			for (int i = 0; i < iresult; ++i)
				std::cout << recvbuffer[i];
			cout << endl;
		}
		else if (iresult == 0)
		{
			std::cout << "Connection Closed " << std::endl;
		}
		else
		{
			std::cout << "RECV failed " << WSAGetLastError() << std::endl;
		}

		bool send_intention = true;
		cout << "Want to send message (0 /1) ?" << endl;
		cin >> send_intention;
		if (send_intention == true)
		{
			char send_buffer[512];
			cout << "Enter Message : " << endl;
			cin.ignore();
			cin.getline(send_buffer, 511);
			iresult = send(clientsocket, send_buffer, (int)strlen(send_buffer) , 0);
			if (iresult == SOCKET_ERROR)
			{
				cout << "SEND Failed " << WSAGetLastError();
			}
		}

	}
}

void server_things()
{
	int temp;

	addrinfo *ptr, *results, hint;
	ptr = nullptr;
	results = nullptr;

	ZeroMemory(&hint, sizeof(hint));

	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = IPPROTO_TCP;
	
	//socket will be used in bind() call 
	hint.ai_flags = AI_PASSIVE;

	int iresult = getaddrinfo(NULL, DEFAULT_PORT, &hint, &results);
	if (iresult != 0)
	{
		cout << "Failed to get address info" << endl;
		WSACleanup();
		cin >> temp;
		exit(1);
	}

	SOCKET serversocket = INVALID_SOCKET;

	serversocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

	if (serversocket == INVALID_SOCKET)
	{
		cout << "Failed at socket creation call" << endl;
		freeaddrinfo(results);
		WSACleanup();
		cin >> temp;
		exit(1);
	}
	
	//socket bind
	iresult = bind(serversocket, results->ai_addr, (int)results->ai_addrlen);

	if (iresult == SOCKET_ERROR)
	{
		cout << "BIND error " << WSAGetLastError();
		freeaddrinfo(results);
		closesocket(serversocket);
		WSACleanup();
		cin >> temp;
		exit(1);
	}


	if (listen(serversocket, SOMAXCONN) == SOCKET_ERROR)
	{
		cout << " LISTEN called failed " <<WSAGetLastError();
		closesocket(serversocket);
		WSACleanup();
		getchar();
		exit(1);
	}

	//create a new socket for accepting calls lmao
	SOCKET acceptor_socket = INVALID_SOCKET;

	//optional paarameter to recieve the details of connecting entity . 
	acceptor_socket = accept(serversocket, nullptr,nullptr);
	if (acceptor_socket == INVALID_SOCKET)
	{
		cout << "ACCEPT CALL FAILED " << WSAGetLastError();
		closesocket(serversocket);
		WSACleanup();
		getchar();

	}

	//buffer for sending and recieving
	char sendbuffer[512];
	char recvbuffer[512];

	char hello_str[] = "SERVER ECHO";

	SEND_MESSAGE(acceptor_socket, hello_str, (int)strlen(hello_str));
	
	//start chatting
	while (1)
	{
		RECV_MESSAGE(acceptor_socket, recvbuffer, 512);
			
		bool send_mood = false;
		cout << "Want to send something (0 / 1) ? ";
		cin >> send_mood;

		if (send_mood == true)
		{
			
			//cin >> sendbuffer;
			cin.ignore();
			cin.getline(sendbuffer, 511);
	
			SEND_MESSAGE(acceptor_socket, sendbuffer, strlen(sendbuffer));
		}

	}


}

int main() {

	initialize_WSA();
	enum APPSTATE{CLIENT=0,SERVER};

	APPSTATE cur_app_state = APPSTATE::CLIENT;
	cout << "Enter the state of application : " << endl;
	cout << "1 : Client" << endl;
	cout << "2 : Server" << endl;

	int choice=0;
	cin >> choice;
	if (choice == 1)
	{
		cur_app_state = APPSTATE::CLIENT;
	}
	else if(choice == 2)
	{
		cur_app_state = APPSTATE::SERVER;
	}
	
	switch (cur_app_state)
	{
	case APPSTATE::CLIENT:
		client_things();
		break;
	case APPSTATE::SERVER:
		server_things();
		break;
	default:
		break;
	}

	return 0;
}