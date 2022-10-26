#include"head.h"
extern FILE* log_file;
extern time_t t;
void upload(int mode, const char* filename, char* buffer, SOCKET sock, sockaddr_in addr, int addrlen) {
	sockaddr_in serveraddr = { 0 };
	int max_send = 0;//��ʱ�ش�����
	int result;//��¼����ֵ
	int data_size;//ÿ�γɹ����ļ���ȡ���ֽ���
	int block_num = 0;//�������
	char data[DATA_SIZE];//��ȡ�ļ�����
	char recv_buffer[BUFFER_SIZE];
	BOOL send_flag = FALSE;//�����Ƿ����
	FILE* fp;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("���ļ�ʧ��\n");
		return;
	}
	write_request(mode, filename, buffer, sock, addr, addrlen);
	while (1) {
		result = receive_ACK(recv_buffer, sock, serveraddr, addrlen);
		if (result == 1) {
			max_send = 0;
			if (send_flag) {
				printf("�������\n");
				return;
			}
			block_num += recv_buffer[2];
			block_num = (block_num << 8) + recv_buffer[3];
			memset(data, 0, DATA_SIZE);
			data_size = fread(data, 1, DATA_SIZE, fp);
			printf("��ȡ�ļ�����size:%d\n", data_size);
			result = send_data(sock, serveraddr, addrlen, fp, buffer, data, data_size, ++block_num);
			if (data_size < 512) {
				send_flag = TRUE;
			}
		}
		//��ʱ�ش�
		else if (result == -1) {
			max_send++;
			if (max_send > MAX_RETRANSMISSION) {
				printf("�ش�ʧ��\n");
				return;
			}
			if (block_num > 0)
				send_data(sock, serveraddr, addrlen, fp, buffer, data, data_size, block_num + 1);
			else
				return;
		}
		else return;
	}
}
//����д����
int write_request(int mode, const char* filename, char* buffer, SOCKET sock, sockaddr_in addr, int addrlen) {
	int send_size = 0;//��������ݴ�С
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
//��������
int receive_ACK(char* recv_buffer, SOCKET sock, sockaddr_in& addr, int addrlen) {
	memset(recv_buffer, 0, sizeof(recv_buffer));
	struct timeval tv;
	fd_set readfds;
	int result;
	int wait_time;
	for (wait_time = 0; wait_time < TIME_OUT; wait_time++) {
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		tv.tv_sec = 4;
		tv.tv_usec = 0;
		select(sock + 1, &readfds, NULL, NULL, &tv);
		result = recvfrom(sock, recv_buffer, 4, 0, (struct sockaddr*)&addr, (int*)&addrlen);
		if (result > 0 && result < 4) {
			printf("bad packet\n");
			return 0;
		}
		else if (result >= 4) {
			if (recv_buffer[1] == ERROR_CODE) {
				printf("ERROR!\n");
				return -2;
			}
			printf("���ձ��ĳɹ�");
			printf("recv:%dbytes ", result);
			printf("blocknum:%d\n", recv_buffer[3]);
			return 1;
		}
	}
	if (wait_time >= TIME_OUT) {
		printf("���յȴ���ʱ\n");
		fprintf(log_file, "ERROR:�ȴ����ճ�ʱ %s", asctime(localtime(&(t = time(NULL)))));
		return -1;
	}
}
//�����ļ�����
int send_data(SOCKET sock, sockaddr_in addr, int addrlen, FILE* fp, char* buffer, char* data, int data_size, unsigned short block_num) {
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
	result = sendto(sock, buffer, send_size, 0, (struct sockaddr*)&addr, addrlen);
	if (result == SOCKET_ERROR) {
		printf("��������ʧ��\n");
		return -1;
	}
	else {
		printf("�������ݳɹ�send:%dbytes\n", result);
		return result;
	}
}