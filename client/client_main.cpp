#include"head.h"
FILE* log_file;//��־�ļ�
char log[512];//д����־����
time_t t;//����ʱ��
int main() {
	//��ʼ����־�ļ�
	log_file = fopen("log.txt", "w+");
	printf("%s", asctime(localtime(&(t=time(NULL)))));
	if (log_file == NULL) {
		printf("������־�ļ�ʧ�ܣ�\n");
		return 0;
	}
	char file_path[128];//�ļ���
	char buffer[BUFFER_SIZE];//���淢�͵�����
	//��ʼ��socket
	WSADATA wsaData;
	int Result = WSAStartup(0x0101, &wsaData);
	if (Result)
	{
		printf("WSAStartup failed with error: %d", Result);
		return 0;
	}
	//�����׽���
	SOCKET client_sock;
	client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client_sock == INVALID_SOCKET) {
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
	//����Ϊ������ģʽ
	unsigned long Opt = 1;
	ioctlsocket(client_sock, FIONBIO, &Opt);
	//�󶨿ͻ���ip�Ͷ˿�
	Result = bind(client_sock, (LPSOCKADDR)&client_addr, sizeof(client_addr));
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

	upload(1, "test.txt", buffer, client_sock, server_addr, sizeof(sockaddr_in));
	
	
}




