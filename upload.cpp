#include"head.h"

void upload(int mode, const char* filename, char* buffer, SOCKET sock, sockaddr_in addr, int addrlen) {
	int result;//��¼����ֵ
	int data_size;//ÿ�γɹ����ļ���ȡ���ֽ���
	int block_num = 0;
	char data[DATA_SIZE];
	char recv_buffer[BUFFER_SIZE];
	FILE* fp;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("���ļ�ʧ��\n");
		return;
	}
	write_request(mode, filename, buffer, sock, addr, addrlen);
	while (1) {
			result = receive(recv_buffer, sock, addr,addrlen);
		if (result==1) {
			block_num += recv_buffer[2];
			block_num = (block_num << 8) + recv_buffer[3];
			memset(data, 0, DATA_SIZE);
			data_size= fread(data, 1, DATA_SIZE, fp);
			printf("��ȡ�ļ�����size:%d\n", data_size);
			send_data(sock,addr,addrlen,fp, buffer,data, data_size, block_num+1);
		}
	}
}

int write_request(int mode, const char* filename, char* buffer, SOCKET sock, sockaddr_in addr, int addrlen) {
	int send_size = 0;//������������ݴ�С
	int result;//��¼����ֵ
	memset(buffer, 0, sizeof(buffer));//��ʼ������
	buffer[++send_size] = WRQ;//operation codeͷ��
	send_size++;
	memcpy(buffer + send_size, filename, sizeof(filename));//д���ļ���
	send_size += sizeof(filename);
	buffer[send_size++] = 0;
	//ascii��
	if (mode == NETASCII) {
		strcpy(buffer + send_size, "netascii");
		send_size += 9;
	}
	//��������
	else
	{
		strcpy(buffer + send_size, "octet");
		send_size += 6;
	}
	result = sendto(sock, buffer, send_size, 0, (struct sockaddr*)&addr, addrlen);
	if (result == SOCKET_ERROR) {
		printf("����д����ʧ��\n");
	}
	else
		printf("����д����ɹ�send:%dbytes\n", result);
	return result;
}

int receive(char* buffer, SOCKET sock, sockaddr_in addr, int addrlen) {
	memset(buffer, 0, sizeof(buffer));
	int len = addrlen;
	printf("...�ȴ�����...\n");
	int result = recvfrom(sock, buffer, 4, 0, (struct sockaddr*)&addr, &len);
	if (result == SOCKET_ERROR) {
		printf("��������ʧ��\n");
		return -1;
	}
	else if (result < 4) {
		printf("bad packet\n");
	}
	else {
		if (buffer[1] == ACK) {
			printf("���ձ��ĳɹ�");
			printf("recv:%dbytes ", result);
			printf("blocknum:%d\n", buffer[3]);
			return 1;
		}
		else if (buffer[1] == ERROR) {
			printf("ERROR!\n");
			return -2;
		}
	}
	return 0;
}

int send_data(SOCKET sock, sockaddr_in addr, int addrlen,FILE* fp, char* buffer,char* data, int data_size, unsigned short block_num) {
	int result;
	int send_size = 0;
	memset(buffer, 0, sizeof(buffer));
	buffer[++send_size] = DATA;
	buffer[++send_size] = (char)(block_num >> 8);
	buffer[++send_size] = (char)block_num;
	send_size++;
	memcpy(buffer + send_size, data, data_size);
	send_size += data_size;
	buffer[send_size] = 0;
	result = sendto(sock,buffer,send_size,0, (struct sockaddr*)&addr,addrlen);
	if (result == SOCKET_ERROR) {
		printf("��������ʧ��\n");
		return -1;
	}
	else {
		printf("�������ݳɹ�send:%dbytes\n", result);
		return result;
	}
}