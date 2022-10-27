#include"head.h"
extern FILE* log_file;
extern time_t t;
void download(int mode, const char* filename, char* buffer, SOCKET sock, sockaddr_in addr, int addrlen) {
	sockaddr_in serveraddr = { 0 };
	int max_send = 0;//��ʱ�ش�����
	int result;//��¼����ֵ
	int data_size;//ÿ�γɹ����ļ���ȡ���ֽ���
	int block_num = 0;//�������
	char data[DATA_SIZE];//��ȡ�ļ�����
	char recv_buffer[BUFFER_SIZE];//�����������
	BOOL recv_flag = FALSE;//�����Ƿ����
	FILE* fp;
	fp = fopen(filename, "w+");
	if (fp == NULL) {
		printf("���ļ�ʧ��\n");
		return;
	}
	read_request(mode, filename, buffer, sock, addr, addrlen);
	while (1) {
		result = receive_data(recv_buffer, sock, serveraddr, addrlen);
		//�յ���ȷ���ݰ�
		if (result >0) {
			max_send = 0;//�����ش�����
			if (recv_flag) {
				printf("�������\n");
				return;
			}
			block_num = 0;
			block_num += recv_buffer[2];
			block_num = (block_num << 8) + recv_buffer[3];
			data_size = fwrite(recv_buffer + 4, 1, result, fp);
			printf("�����ļ�����size:%d\n", data_size);
			result = send_ACK(sock, serveraddr, addrlen, fp, buffer, data, data_size, block_num);
			if (data_size < 512) {
				recv_flag = TRUE;//�������
			}
		}
		//��ʱ����ʧ���ش�
		else if (result == -1) {
			max_send++;
			if (max_send > MAX_RETRANSMISSION) {
				printf("�ش�ʧ��\n");
				fprintf(log_file, "ERROR:�ش��������� %s", asctime(localtime(&(t = time(NULL)))));
				return;
			}
			if (block_num > 0) {
				fprintf(log_file, "�ش�ACK�� ACK���:%d %s", block_num, asctime(localtime(&(t = time(NULL)))));
				send_data(sock, serveraddr, addrlen, fp, buffer, data, data_size, block_num);
			}
			else {
				fprintf(log_file, "�ش������� %s", asctime(localtime(&(t = time(NULL)))));
				read_request(mode, filename, buffer, sock, addr, addrlen);
			}
		}
		//�յ������
		else {
			return;
		}
	}
}
//���Ͷ�����
int read_request(int mode, const char* filename, char* buffer, SOCKET sock, sockaddr_in addr, int addrlen) {
	int send_size = 0;//��������ݴ�С
	int result;//��¼����ֵ
	memset(buffer, 0, sizeof(buffer));//��ʼ������
	buffer[++send_size] = RRQ;//operation codeͷ��
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
		printf("���Ͷ�����ʧ��\n");
		fprintf(log_file, "ERROR:���Ͷ�����ʧ�� ������:%d %s", WSAGetLastError(), asctime(localtime(&(t = time(NULL)))));
	}
	else {
		printf("���Ͷ�����ɹ�send:%dbytes\n", result);
		fprintf(log_file, "���Ͷ�����ɹ� %s", asctime(localtime(&(t = time(NULL)))));
	}
	return result;
}
//��������
int receive_data(char* recv_buffer, SOCKET sock, sockaddr_in& addr, int addrlen) {
	memset(recv_buffer, 0, sizeof(recv_buffer));
	struct timeval tv;
	fd_set readfds;
	int result;
	int wait_time;
	for (wait_time = 0; wait_time < TIME_OUT; wait_time++) {
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		select(sock + 1, &readfds, NULL, NULL, &tv);
		result = recvfrom(sock, recv_buffer, BUFFER_SIZE, 0, (struct sockaddr*)&addr, (int*)&addrlen);
		if (result > 0 && result < 4) {
			printf("bad packet\n");
			fprintf(log_file, "ERROR:���հ�����ȷ %s", asctime(localtime(&(t = time(NULL)))));
			return 0;
		}
		else if (result >= 4) {
			if (recv_buffer[1] == ERROR_CODE) {
				printf("ERROR!\n");
				fprintf(log_file, "ERROR:���յ������ ������:%d ������Ϣ%s %s", recv_buffer[3], recv_buffer + 4, asctime(localtime(&(t = time(NULL)))));
				return -2;
			}
			printf("���ձ��ĳɹ�");
			printf("recv:%dbytes ", result);
			printf("blocknum:%d\n", recv_buffer[3]);
			fprintf(log_file, "�������ݳɹ� ���ݰ����:%d %s", recv_buffer[3]+(recv_buffer[2]>>8), asctime(localtime(&(t = time(NULL)))));
			return result;
		}
	}
	if (wait_time >= TIME_OUT) {
		printf("���յȴ���ʱ\n");
		fprintf(log_file, "ERROR:�ȴ����ճ�ʱ %s", asctime(localtime(&(t = time(NULL)))));
		return -1;
	}
}
//����ACK��
int send_ACK(SOCKET sock, sockaddr_in addr, int addrlen, FILE* fp, char* buffer, char* data, int data_size, unsigned short block_num) {
	int result;
	int send_size = 0;
	memset(buffer, 0, sizeof(buffer));
	buffer[++send_size] = ACK;
	buffer[++send_size] = (char)(block_num >> 8);
	buffer[++send_size] = (char)block_num;
	result = sendto(sock, buffer, 4, 0, (struct sockaddr*)&addr, addrlen);
	if (result == SOCKET_ERROR) {
		printf("����ACKʧ��\n");
		fprintf(log_file, "ERROR:����ACK��ʧ�� ACK���:%d ������:%d %s", block_num, WSAGetLastError(), asctime(localtime(&(t = time(NULL)))));
		return -1;
	}
	else {
		printf("����ACK���ɹ�send:%dbytes\n", result);
		fprintf(log_file, "����ACK���ɹ� ACK�����:%d %s", block_num, asctime(localtime(&(t = time(NULL)))));
		return result;
	}
}