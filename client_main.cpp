#include"head.h"
int main() {
	char file_path[128];
	char buffer[BUFFER_SIZE];
	//��ʼ��socket
	WSADATA wsaData;
	int Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Result != 0)
	{
		printf("WSAStartup failed with error: %d", Result);
		return 0;
	}
	//�����׽���
	SOCKET tftpsock;
	tftpsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (tftpsock == INVALID_SOCKET) {
		printf("�����׽���ʧ��\n");
		return 0;
	}
	//����� ip�Ͷ˿�
	sockaddr_in server_addr;
	char serverip[20] = "10.12.181.168";
	int serverport = 69;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serverport);
	server_addr.sin_addr.S_un.S_addr = inet_addr(serverip);
	//�ͻ���ip�Ͷ˿�
	sockaddr_in client_addr;
	char clientip[20] = "10.12.181.87";
	int clientport = 0;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(clientport);
	client_addr.sin_addr.S_un.S_addr = inet_addr(clientip);
	Result = bind(tftpsock, (LPSOCKADDR)&client_addr, sizeof(client_addr));
	if (Result == SOCKET_ERROR)
	{
		// ��ʧ��
		printf("Client socket bind error!\n");
		return 0;
	}
	//if (scanf("%s", file_path) == NULL) {
//	printf("��ȡ�ļ���ʧ��");
//	return 0;
//}
//�󶨿ͻ���ip�Ͷ˿�
	upload(2, "test.txt", buffer, tftpsock, server_addr, sizeof(server_addr));
	
	
}




