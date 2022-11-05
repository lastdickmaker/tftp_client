#include"head.h"
extern FILE* log_file;
extern time_t t;
extern clock_t start, end;
//����
void download(int mode, const char* filename, char* buffer, SOCKET sock, sockaddr_in addr, int addrlen) {
	int recv_bytes = 0;//��¼���մ�С
	sockaddr_in serveraddr = { 0 };
	int max_send = 0;//��ʱ�ش�����
	int result;//��¼����ֵ
	int data_size;//ÿ�γɹ����ļ���ȡ���ֽ���
	int block_num = 0;//�������
	char data[DATA_SIZE];//��ȡ�ļ�����
	char recv_buffer[BUFFER_SIZE];//�����������
	BOOL end_flag = FALSE;//�����Ƿ����
	BOOL start_flag = TRUE;//�����Ƿ�ʼ
	FILE* fp;
	if (mode == 1)
		fp = fopen(filename, "w");
	if (mode == 2)
		fp = fopen(filename, "wb");
	if (fp == NULL) {
		printf("���ļ�ʧ��\n");
		printf("\n�����������...");
		result = getch();
		return;
	}
	//���Ͷ�����
	read_request(mode, filename, buffer, sock, addr, addrlen);
	while (1) {
		if (start_flag) {
			//��¼��ʼʱ��
			start = clock();
			start_flag = FALSE;
		}
		if (end_flag) {
			printf("������� �����С:%dbytes speed:%.1fkb/s", recv_bytes, recv_bytes / (1024 * (double)(end - start) / CLK_TCK));
			printf("\n�����������...");
			result = getch();
			fclose(fp);
			return;
		}
		//�������ݰ�
		result = receive_data(recv_buffer, sock, serveraddr, addrlen);
		if (result > 0) {
			max_send = 0;//�����ش�����
			if (block_num != ((recv_buffer[2] << 8) + recv_buffer[3] - 1))
				result = -1;
		}
		//�յ���ȷ���ݰ�
		if (result > 0) {
			recv_bytes += result - 4;//��¼�������ݴ�С
			max_send = 0;//�����ش�����
			block_num++;
			data_size = fwrite(recv_buffer + 4, 1, result - 4, fp);
			if (data_size < 512) {
				//�������
				end_flag = TRUE;
				end = clock();
			}
			result = send_ACK(sock, serveraddr, addrlen, fp, buffer, data, data_size, block_num);
		}
		//��ʱ����ʧ���ش�
		else if (result == -1) {
			max_send++;//�ش�������һ
			printf("...�ش���...%d\n", max_send);
			if (max_send > MAX_RETRANSMISSION) {
				printf("�ش���������");
				printf("\n�����������...");
				result = getch();
				fprintf(log_file, "ERROR:�ش��������� %s", asctime(localtime(&(t = time(NULL)))));
				return;
			}
			if (block_num > 0) {
				fprintf(log_file, "�ش�ACK�� ACK���:%d %s", block_num, asctime(localtime(&(t = time(NULL)))));
				send_ACK(sock, serveraddr, addrlen, fp, buffer, data, data_size, block_num);
			}
			else {
				fprintf(log_file, "�ش�������					%s", asctime(localtime(&(t = time(NULL)))));
				read_request(mode, filename, buffer, sock, addr, addrlen);
			}
		}
		//�յ������
		else {
			printf("ERROR!������:%d %s", recv_buffer[3], recv_buffer + 4);
			printf("\n�����������...");
			result = getch();
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
		printf("���Ͷ�����ʧ��");
		printf("\n�����������...");
		result = getch();
		fprintf(log_file, "ERROR:���Ͷ�����ʧ��	������:%d	%s", WSAGetLastError(), asctime(localtime(&(t = time(NULL)))));
	}
	else {
		fprintf(log_file, "���Ͷ�����ɹ�	send%dbytes	�ļ���:%s	%s", result, filename, asctime(localtime(&(t = time(NULL)))));
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
	//����ʱ�ޣ�����ʱ������Ϊ����ʧ��
	for (wait_time = 0; wait_time < TIME_OUT; wait_time++) {
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		select(sock + 1, &readfds, NULL, NULL, &tv);
		result = recvfrom(sock, recv_buffer, BUFFER_SIZE, 0, (struct sockaddr*)&addr, (int*)&addrlen);
		if (result > 0 && result < 4) {
			printf("bad packet");
			printf("\n�����������...");
			result = getch();
			fprintf(log_file, "ERROR:���հ�����ȷ	%s", asctime(localtime(&(t = time(NULL)))));
			return 0;
		}
		else if (result >= 4) {
			if (recv_buffer[1] == ERROR_CODE) {
				fprintf(log_file, "ERROR:���յ������	������:%d	������Ϣ%s	%s", recv_buffer[3], recv_buffer + 4, asctime(localtime(&(t = time(NULL)))));
				return -2;
			}
			fprintf(log_file, "�������ݳɹ�	receive%dbytes	���ݰ����:%d	%s", result, recv_buffer[3] + (recv_buffer[2] >> 8), asctime(localtime(&(t = time(NULL)))));
			return result;
		}
	}
	if (wait_time >= TIME_OUT) {
		fprintf(log_file, "ERROR:�ȴ����ճ�ʱ					%s", asctime(localtime(&(t = time(NULL)))));
		return -1;
	}
}
//����ACK��
int send_ACK(SOCKET sock, sockaddr_in addr, int addrlen, FILE* fp, char* buffer, char* data, int data_size, unsigned short block_num) {
	int result;
	int send_size = 0;
	memset(buffer, 0, sizeof(buffer));
	//��������
	buffer[++send_size] = ACK;
	//�������
	buffer[++send_size] = (char)(block_num >> 8);
	buffer[++send_size] = (char)block_num;
	result = sendto(sock, buffer, 4, 0, (struct sockaddr*)&addr, addrlen);
	if (result == SOCKET_ERROR) {
		printf("����ACKʧ��");
		printf("\n�����������...");
		result = getch();
		fprintf(log_file, "ERROR:����ACK��ʧ��	ACK���:%d	������:%d	%s", block_num, WSAGetLastError(), asctime(localtime(&(t = time(NULL)))));
		return -1;
	}
	else {
		fprintf(log_file, "����ACK���ɹ�	send%dbytes	ACK�����:%d	%s", result, block_num, asctime(localtime(&(t = time(NULL)))));
		return result;
	}
}