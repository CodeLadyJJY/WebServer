//--------------------------------------������˵����-------------------------------------------
//		�����������򵥵�Web������
//		�����������ò���ϵͳ�� Windows 8.1 64bit
//		������������IDE�汾��Visual Studio 2015
//		2015��11��10�� Created by @CodeLadyJJY
//------------------------------------------------------------------------------------------------


#include<winsock2.h>
#include<iostream>
#include<string>
#include<io.h>

#pragma comment(lib, "ws2_32.lib")

#define HEADER_404 "\
HTTP/1.1 404 Not Found\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Server: http_v1.0.1\r\n\
Content-Length: %d\r\n\r\n%s\
"

#define HTML_404 "\
<html>\
    <head>\
        <title>Not Found</title>\
    <head>\
    <body>\
        <h2>Not Found</h2>\
		<hr>\
		<p>HTTP Error 404. The requested resource is not found.</p>\
    </body>\
</html>\
"

/*
����ļ��Ƿ����, ���ڷ���1�������ڷ���0
*/
bool check_file(char *file)
{
	return (access(file, 0) == 0);
}

/*
�����ļ�
*/
void send_file(char *file, SOCKET s)
{
	char *response;
	char *content;
	int file_size;
	FILE *fp = fopen(file, "rb");

	//���html�ļ�
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);	//��ȡ�ļ���С
	fseek(fp, 0, SEEK_SET);
	content = (char*)malloc(file_size + 1);
	fread(content, file_size, 1, fp);
	content[file_size] = 0;

	response = (char*)malloc(file_size + 100);
	sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s", file_size, content);

	if (send(s, response, strlen(response), 0) == SOCKET_ERROR)
	{
		printf("Send data failed!");
		closesocket(s);
	}

	free(content);
	free(response);
}

/*
����404ҳ��
*/
void send_notfound(char *file, SOCKET s)
{
	char response[BUFSIZ];
	memset(response, 0, BUFSIZ);
	sprintf(response, HEADER_404, strlen(HTML_404), HTML_404);

	if (send(s, response, strlen(response), 0) == SOCKET_ERROR)
	{
		printf("Send data failed!");
		closesocket(s);
	}
}

int main()
{
	WSADATA              wsaData;
	SOCKET               ListeningSocket;
	SOCKET               NewConnection;
	SOCKADDR_IN          ServerAddr;
	SOCKADDR_IN          ClientAddr;
	int                  Port = 81;
	int					 ClientAddrLen;
	char				 revBuff[1000];

	// ��ʼ��Windows Socket 2.0
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
	{
		printf("sock init failed\n");
		WSACleanup();
		return -1;
	}
	printf("sock init succeed\n");

	// ����һ���µ�Socket����Ӧ�ͻ��˵���������
	ListeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ListeningSocket == SOCKET_ERROR)
	{
		printf("sock create failed\n");
		WSACleanup();
		return -1;
	}
	printf("sock create successed\n");

	// ��д��������ַ��Ϣ
	// �˿�Ϊ81
	// IP��ַΪINADDR_ANY��ע��ʹ��htonl��IP��ַת��Ϊ�����ʽ
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// �󶨼����˿�
	if (bind(ListeningSocket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		printf("bind is failed\n");
		closesocket(ListeningSocket);
		WSACleanup();
		return -1;
	}
	printf("bind is successsed\n");

	// ��ʼ������ָ�����ͬʱ������Ϊ10
	if (listen(ListeningSocket, 10) == SOCKET_ERROR)
	{
		printf("listen failed\n");
		closesocket(ListeningSocket);
		WSACleanup();
		return -1;
	}
	printf("listening......\n");

	ClientAddrLen = sizeof(sockaddr_in);
	while (1)
	{
		// �����µ�����
		NewConnection = accept(ListeningSocket, (SOCKADDR *)&ClientAddr, &ClientAddrLen);
		if (NewConnection == SOCKET_ERROR)
		{
			printf("sock create failed\n");
			closesocket(NewConnection);
			return -1;
		}

		//��������
		ZeroMemory(revBuff, 1000);
		if (recv(NewConnection, revBuff, 1000, 0) == SOCKET_ERROR)
		{
			printf("recv failed\n");
			closesocket(NewConnection);
			break;
		}

		printf(revBuff);

		//��Ӧ�����
		char temp[BUFSIZ];
		char file[BUFSIZ];

		strcpy(file, "./www");

		if (sscanf(revBuff, "%s %s", temp, file + 5) != 2)
			exit(1);

		if (check_file(file))
			send_file(file, NewConnection);
		else
			send_notfound(file, NewConnection);

		//�ر�Socket
		closesocket(NewConnection);
	}

	// �رռ���Socket��Ȼ���˳�Ӧ�ó���  
	closesocket(ListeningSocket);

	// �ͷ�Windows Socket �������Դ
	WSACleanup();

	return 0;
}