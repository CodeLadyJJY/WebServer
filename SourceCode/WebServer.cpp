//--------------------------------------【程序说明】-------------------------------------------
//		程序描述：简单的Web服务器
//		开发测试所用操作系统： Windows 8.1 64bit
//		开发测试所用IDE版本：Visual Studio 2015
//		2015年11月10日 Created by @CodeLadyJJY
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
检查文件是否存在, 存在返回1，不存在返回0
*/
bool check_file(char *file)
{
	return (access(file, 0) == 0);
}

/*
发送文件
*/
void send_file(char *file, SOCKET s)
{
	char *response;
	char *content;
	int file_size;
	FILE *fp = fopen(file, "rb");

	//获得html文件
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);	//获取文件大小
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
发送404页面
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

	// 初始化Windows Socket 2.0
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
	{
		printf("sock init failed\n");
		WSACleanup();
		return -1;
	}
	printf("sock init succeed\n");

	// 创建一个新的Socket来响应客户端的连接请求
	ListeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ListeningSocket == SOCKET_ERROR)
	{
		printf("sock create failed\n");
		WSACleanup();
		return -1;
	}
	printf("sock create successed\n");

	// 填写服务器地址信息
	// 端口为81
	// IP地址为INADDR_ANY，注意使用htonl将IP地址转换为网络格式
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// 绑定监听端口
	if (bind(ListeningSocket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		printf("bind is failed\n");
		closesocket(ListeningSocket);
		WSACleanup();
		return -1;
	}
	printf("bind is successsed\n");

	// 开始监听，指定最大同时连接数为10
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
		// 接受新的连接
		NewConnection = accept(ListeningSocket, (SOCKADDR *)&ClientAddr, &ClientAddrLen);
		if (NewConnection == SOCKET_ERROR)
		{
			printf("sock create failed\n");
			closesocket(NewConnection);
			return -1;
		}

		//接受请求
		ZeroMemory(revBuff, 1000);
		if (recv(NewConnection, revBuff, 1000, 0) == SOCKET_ERROR)
		{
			printf("recv failed\n");
			closesocket(NewConnection);
			break;
		}

		printf(revBuff);

		//响应浏览器
		char temp[BUFSIZ];
		char file[BUFSIZ];

		strcpy(file, "./www");

		if (sscanf(revBuff, "%s %s", temp, file + 5) != 2)
			exit(1);

		if (check_file(file))
			send_file(file, NewConnection);
		else
			send_notfound(file, NewConnection);

		//关闭Socket
		closesocket(NewConnection);
	}

	// 关闭监听Socket，然后退出应用程序  
	closesocket(ListeningSocket);

	// 释放Windows Socket 的相关资源
	WSACleanup();

	return 0;
}