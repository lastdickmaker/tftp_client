#include"head.h"
extern FILE* log_file;
extern time_t t;
extern clock_t start, end;

//�ϴ�
void upload(int mode, const char* filename, char* buffer, SOCKET sock, sockaddr_in addr, int addrlen) {
	int send_bytes = 0;//��¼�ܴ������ݴ�С
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
		fp = fopen(filename, "r");
	if (mode == 2)
		fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("���ļ�ʧ��");
		printf("\n�����������...");
		result = getch();
		return;
	}
	//����д����
	write_request(mode, filename, buffer, sock, addr, addrlen);
	while (1) {
		//����ACK
		result = receive_ACK(recv_buffer, sock, serveraddr, addrlen);
		if (result != block_num)
			result = -1;
		//�յ���ЧACK��
		if (result >= 0) {
			max_send = 0;//�����ش�����
			//��������
			if (end_flag) {
				printf("������� �����С:%dbytes speed:%.1fkb/s", send_bytes, send_bytes / (1024 * ((double)(end - start) / CLK_TCK)));
				printf("\n�����������...");
				result = getch();
				fclose(fp);
				return;
			}
			//����ACK��ŵ���һ�����
			block_num = result;
			memset(data, 0, DATA_SIZE);
			data_size = fread(data, 1, DATA_SIZE, fp);//��ȡ�ļ�����
			fprintf(log_file, "��%s�ļ���ȡ	size:%dbytes			%s", filename, data_size, asctime(localtime(&(t = time(NULL)))));
			//��¼��ʼ����ʱ��
			if (start_flag) {
				start = clock();
				start_flag = FALSE;
			}
			//�������ݰ�
			result = send_data(sock, serveraddr, addrlen, fp, buffer, data, data_size, ++block_num);
			send_bytes += data_size;
			if (data_size < 512 && result != -1) {
				//��¼��������ʱ��
				end = clock();
				end_flag = TRUE;//�������
			}
		}
		//��ʱ����ʧ���ش�
		else if (result == -1 || result == -2) {
			max_send++;//�ش�������һ
			printf("...�ش���...%d\n", max_send);
			//�ش��ﵽ����
			if (max_send > MAX_RETRANSMISSION) {
				printf("�ش���������");
				printf("\n�����������...");
				result = getch();
				fprintf(log_file, "ERROR:�ش���������	%s", asctime(localtime(&(t = time(NULL)))));
				return;
			}
			if (block_num > 0) {
				fprintf(log_file, "�ش����ݰ�	���ݰ����:%d	%s", block_num, asctime(localtime(&(t = time(NULL)))));
				send_data(sock, serveraddr, addrlen, fp, buffer, data, data_size, block_num);
			}
			else {
				fprintf(log_file, "�ش�д����					%s", asctime(localtime(&(t = time(NULL)))));
				write_request(mode, filename, buffer, sock, addr, addrlen);
			}
		}
		//�յ������
		else if (result == -3) {
			printf("ERROR!������:%d %s", recv_buffer[3], recv_buffer + 4);
			printf("\n�����������...");
			result = getch();
			return;
		}
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
		printf("����д����ʧ��");
		printf("\n�����������...");
		result = getch();
		fprintf(log_file, "ERROR:����д����ʧ��	������:%d	%s", WSAGetLastError(), asctime(localtime(&(t = time(NULL)))));
	}
	else {
		fprintf(log_file, "����д����ɹ�	send%dbytes	�ļ���:%s	%s", result, filename, asctime(localtime(&(t = time(NULL)))));
	}
	return result;
}

//����ACK��
int receive_ACK(char* recv_buffer, SOCKET sock, sockaddr_in& addr, int addrlen) {
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
		result = recvfrom(sock, recv_buffer, 4, 0, (struct sockaddr*)&addr, (int*)&addrlen);
		if (result > 0 && result < 4) {
			printf("bad packet");
			printf("\n�����������...");
			result = getch();
			fprintf(log_file, "ERROR:���հ�����ȷ	%s", asctime(localtime(&(t = time(NULL)))));
			return -2;
		}
		else if (result >= 4) {
			if (recv_buffer[1] == ERROR_CODE) {
				fprintf(log_file, "ERROR:���յ������	������:%d	������Ϣ%s	%s", recv_buffer[3], recv_buffer + 4, asctime(localtime(&(t = time(NULL)))));
				return -3;
			}
			fprintf(log_file, "����ACK���ɹ�	receive%dbytes	ACK���:%d	%s", result, recv_buffer[3] + (recv_buffer[2] << 8), asctime(localtime(&(t = time(NULL)))));
			return recv_buffer[3] + (recv_buffer[2] << 8);//�������
		}
	}
	if (wait_time >= TIME_OUT) {
		fprintf(log_file, "ERROR:�ȴ����ճ�ʱ					%s", asctime(localtime(&(t = time(NULL)))));
		return -1;
	}
}

//�����ļ�����
int send_data(SOCKET sock, sockaddr_in addr, int addrlen, FILE* fp, char* buffer, char* data, int data_size, unsigned short block_num) {
	int result;
	int send_size = 0;
	memset(buffer, 0, sizeof(buffer));
	//��������
	buffer[++send_size] = DATA;
	//������
	buffer[++send_size] = (char)(block_num >> 8);
	buffer[++send_size] = (char)block_num;
	send_size++;
	//�������
	memcpy(buffer + send_size, data, data_size);
	send_size += data_size;
	buffer[send_size] = 0;
	result = sendto(sock, buffer, send_size, 0, (struct sockaddr*)&addr, addrlen);
	if (result == SOCKET_ERROR) {
		printf("��������ʧ��");
		printf("\n�����������...");
		result = getch();
		fprintf(log_file, "ERROR:��������ʧ��	���ݰ����:%d ������:%d	%s", block_num, WSAGetLastError(), asctime(localtime(&(t = time(NULL)))));
		return -1;
	}
	else {
		fprintf(log_file, "�������ݰ��ɹ�	send%dbytes	���ݰ����:%d	%s", result, block_num, asctime(localtime(&(t = time(NULL)))));
		return result;
	}
}