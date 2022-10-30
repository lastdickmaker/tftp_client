#include"head.h"
FILE* log_file;//��־�ļ�
time_t t;//����ʱ��
clock_t start, end;//��¼����ʱ��
int main() {
	//��ʼ����־�ļ�
	log_file = fopen("log.txt", "w+");
	if (log_file == NULL) {
		printf("������־�ļ�ʧ�ܣ�\n");
		return 0;
	}
	char filename[128];//�ļ���
	char buffer[BUFFER_SIZE];//���淢�͵�����
	int Result;//���淵��ֵ
	//����Winsocket
	WSADATA wsaData;
	Result = WSAStartup(0x0101, &wsaData);
	if (Result)
	{
		printf("WSAStartup failed with error: %d", Result);
		fprintf(log_file, "ERROR�޷�����Winsocket	������:%d	%s", WSAGetLastError(), asctime(localtime(&(t = time(NULL)))));
		return 0;
	}
	fprintf(log_file, "����Winsocket			%s", asctime(localtime(&(t = time(NULL)))));
	//�����׽���
	SOCKET client_sock;
	client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client_sock == INVALID_SOCKET) {
		printf("�����׽���ʧ��\n");
		fprintf(log_file, "ERROR:�޷������׽���	������:%d	%s", WSAGetLastError(), asctime(localtime(&(t = time(NULL)))));
		return 0;
	}
	fprintf(log_file, "�����׽���			%s", asctime(localtime(&(t = time(NULL)))));
	//����� ip�Ͷ˿�
	sockaddr_in server_addr;
	char serverip[20] = "10.12.180.43";
	int serverport = 69;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serverport);
	server_addr.sin_addr.S_un.S_addr = inet_addr(serverip);
	//�ͻ���ip�Ͷ˿�
	sockaddr_in client_addr;
	char clientip[20] = "10.12.181.1";
	int clientport = 0;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(clientport);
	client_addr.sin_addr.S_un.S_addr = inet_addr(clientip);
	//����Ϊ������ģʽ
	unsigned long Opt = 1;
	Result = ioctlsocket(client_sock, FIONBIO, &Opt);
	if (Result == SOCKET_ERROR) {
		printf("���÷�����ģʽʧ��\n");
		fprintf(log_file, "ERROR:�޷����÷�����ģʽ	������:%d	%s", WSAGetLastError(), asctime(localtime(&(t = time(NULL)))));
		return 0;
	}
	//�󶨿ͻ��˽ӿ�
	Result = bind(client_sock, (LPSOCKADDR)&client_addr, sizeof(client_addr));
	if (Result == SOCKET_ERROR)
	{
		// ��ʧ��
		printf("Client socket bind error!");
		printf("\n�����������...");
		Result = getch();
		fprintf(log_file, "ERROR:�޷��󶨽ӿ�	������:%d	%s", WSAGetLastError(), asctime(localtime(&(t = time(NULL)))));
		return 0;
	}
	//������
	char choice;
	char mode;
	while (1) {
		system("cls");
		printf("[TFTP�ļ�����]\n1���ϴ��ļ�\n2�������ļ�\n3����������\n");
		printf("��ѡ����:");
		choice = getch();
		system("cls");
		if (choice == '1') {
			printf("[�ϴ��ļ�]\n");
			printf("�������ļ���:");
			scanf("%s", filename);
			system("cls");
			printf("[�ϴ��ļ�]\n");
			printf("1��netascii\n2��octet\n��ѡ����ģʽ:");
			mode = getch();
			printf("\n");
			upload(mode - 48, filename, buffer, client_sock, server_addr, sizeof(sockaddr_in));
		}
		if (choice == '2') {
			printf("[�����ļ�]\n");
			printf("�������ļ���:");
			scanf("%s", filename);
			system("cls");
			printf("[�����ļ�]\n");
			printf("1��netascii\n2��octet\n��ѡ����ģʽ:");
			mode = getch();
			printf("\n");
			download(mode - 48, filename, buffer, client_sock, server_addr, sizeof(sockaddr_in));
		}
		if (choice == '3')
			return 0;
	}
	fclose(log_file);
}




