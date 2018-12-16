#ifndef _SMTP_
#include "smtp.h"
#endif

#ifndef _IOSTREAM_
#include <iostream>
#endif

#ifndef _CTIME_
#include <ctime>
#endif

#ifndef _FSTREAM_
#include <fstream>
#endif

#pragma comment (lib,"Ws2_32.lib")

void GetTimeStamp(char *output, const char * format)
{
	time_t now_time;
	struct tm info;

	//��ȡ��ǰʱ��� ����Ϊ���ص�ַ ת����tm �ṹ
	time(&now_time);
	localtime_s(&info, &now_time);

	//����ָ����ʽ����� ���뻺����
	strftime(output, LOG_T_MAXLEN - 1, format, &info);
}


void LoadSocket(int major_version, int minor_version)
{
	WSADATA wsadata;
	WORD socket_version = MAKEWORD(major_version, minor_version);
	int error;

	error = WSAStartup(socket_version, &wsadata);
	if (error)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		std::cout << "ERROR ";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
		std::cout << "wsastartup failed with error : " << error << std::endl;
		exit(1);
	}
}


SmtpServer& operator<<(SmtpServer& server, const char *data_send)
{
	//��������
	send(server.session_socket_, data_send, strlen(data_send), 0);

	//��¼��־
	GetTimeStamp(server.log_time_buffer_, LOG_T_F);
	server.log_file_ << server.log_time_buffer_ << "INFO send:  " << data_send;

	//DEBUG
#if SMTP_DEBUG
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	std::cout << "INFO";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	std::cout << "	send:  " << data_send;
#endif


	return server;
}


int operator>>(SmtpServer& server, char *data_receive)
{
	//�������ݣ����û����������������
	int data_len = 0;
	data_len = recv(server.session_socket_, data_receive, server.buffer_size_, NULL);
	GetTimeStamp(server.log_time_buffer_, LOG_T_F);

	//�ͻ�������Ͽ�����
	if (data_len == -1)
	{
		server.state_ = -2;

		server.log_file_ << server.log_time_buffer_ << "WARNING client disconnected from server" << std::endl;

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		std::cout << "WARNING ";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
		std::cout << "client disconnected from server" << std::endl;

		return -1;
	}

	//��¼��־���������׼���
	data_receive[data_len] = '\0';
	server.log_file_ << server.log_time_buffer_ << "INFO receive:  " << data_receive;

	//DEBUG
#if SMTP_DEBUG
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	std::cout << "INFO";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	std::cout << " receive:  " << data_receive;
#endif
	
	return data_len;
}


SmtpServer::SmtpServer(int buffer_size) :listen_socket_(INVALID_SOCKET), buffer_size_(buffer_size), buffer_(NULL)
{
	//ͨ����ʽ LOG_FN_F ��ȡLOG�ļ���
	char log_fn[30];
	GetTimeStamp(log_fn, LOG_FN_F);

	//��Log�ļ�
	log_file_.open(log_fn);
	if (!log_file_.is_open())
	{
		exit(FILE_OPEN_ERROR);
	}

	//��ȡsocket ��ַ��ipv4 ��ʽSOCKET Э��TCP
	listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket_ == INVALID_SOCKET)
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "ERROR socket failed with error : "
			<< WSAGetLastError() << std::endl;

		WSACleanup();
		exit(SOCKET_CREAT_ERROR);
	}

	//���뻺���ڴ�
	buffer_ = new char[buffer_size_];
	if (buffer_ == NULL)
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "ERROR failed to new a " << buffer_size_
			<< "bytes buffer" << std::endl;

		WSACleanup();
		exit(BUFFER_GET_ERROR);
	}

}


void SmtpServer::Listen(unsigned short listen_port)
{
	//���õ�ַ�Ͷ˿�
	listen_addr_ = "127.0.0.1";
	listen_port_ = listen_port;

	//�������˿ڵ�ַ��ʼ��
	sockaddr_in svr_adr;
	memset(&svr_adr, 0, sizeof(svr_adr));
	
	//��ַ�� ipv4 
	svr_adr.sin_family = AF_INET;
	svr_adr.sin_port = htons(listen_port_);
	svr_adr.sin_addr.S_un.S_addr = inet_addr(listen_addr_);


	//�󶨶˿ں͵�ַ
	if (bind(listen_socket_, (LPSOCKADDR)&svr_adr, sizeof(svr_adr)) == SOCKET_ERROR)
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "ERROR bind failed with error: "
			<< WSAGetLastError() << std::endl;

		closesocket(listen_socket_);
		WSACleanup();
		exit(SOCKET_BIND_ERROR);
	}

	//��ʼ�����˿�
	if (listen(listen_socket_, SOMAXCONN) == SOCKET_ERROR)
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "ERROR listen failed with error: "
			<< WSAGetLastError() << std::endl;

		closesocket(listen_socket_);
		WSACleanup();
		exit(SOCKET_LISTEN_ERROR);
	}

	GetTimeStamp(log_time_buffer_, LOG_T_F);
	log_file_<< log_time_buffer_<< "INFO server listenning on " << listen_addr_
		<< ":" << listen_port_ << "......" << std::endl;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	std::cout << "INFO" << " listenning on " << listen_addr_
		<< ":" << listen_port_ << "......" << std::endl;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

}


void SmtpServer::Start(CallBack server_logic, CallBack client_logic, SmtpServer& svr)
{
	//�ͻ��˵�ַ��ʼ��
	sockaddr_in host_addr;
	int host_addr_len = sizeof(host_addr);
	memset(&host_addr, 0, host_addr_len);

	while (1)
	{
		session_socket_ = INVALID_SOCKET;

		//�������ӣ����û����������������
		session_socket_ = accept(listen_socket_, (SOCKADDR*)&host_addr, &host_addr_len);
		GetTimeStamp(log_time_buffer_, LOG_T_F);

		//����ʧ��������������������һ������
		if (session_socket_ == INVALID_SOCKET)
		{
			log_file_ << GetTimeStamp << "WARNING accept failed with error: "
				<< WSAGetLastError() << std::endl;

			continue;
		}

		log_file_ << log_time_buffer_ << "INFO accepted a connection from " << inet_ntoa(host_addr.sin_addr)
			<< ":" << ntohs(host_addr.sin_port) << std::endl;

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		std::cout << "INFO" << " a connection from " << inet_ntoa(host_addr.sin_addr)
			<< ":" << ntohs(host_addr.sin_port) << std::endl;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);

		//Ȼ����ûص�������ʼSMTP SERVER�߼�
		if (server_logic(svr) == 0)
		{
			//�رտͻ���SOCKET
			closesocket(session_socket_);

			GetTimeStamp(log_time_buffer_, LOG_T_F);
			log_file_ << log_time_buffer_ << "INFO mail receive succeed" << std::endl;

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
			std::cout << "INFO mail receive success" << std::endl;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);

			//ֻ�дӿͻ��˽����ʼ��ɹ����Ž�һ��������Զ�̷�������ͨ��
			//����Զ�̷�����  ���ûص����� ��ʼSMTP Client�߼�
			if (ConnectRemote() == 0)
			{
				//���ӳɹ�����ʼ�����ʼ�
				if (client_logic(svr) == 0)
				{
					//�ر�Զ�̷�����SOCKET
					closesocket(session_socket_);

					GetTimeStamp(log_time_buffer_, LOG_T_F);
					log_file_ << log_time_buffer_ << "INFO mail send succeed" << std::endl;

					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
					std::cout << "INFO mail send succeed" << std::endl;
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);
				}
				else
				{
					//�����ʼ�ʧ��Ҳ�ر�Զ�̷�����SOCKET
					closesocket(session_socket_);

					GetTimeStamp(log_time_buffer_, LOG_T_F);
					log_file_ << log_time_buffer_ << "WARNING mail send failed" << std::endl;

					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
					std::cout << "WARNING ";
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);
					std::cout << "mail send failed" << std::endl;
				}
			}
		}
		else
		{
			//�����ʼ�ʧ��Ҳ�رտͻ���SOKCET
			closesocket(session_socket_);

			GetTimeStamp(log_time_buffer_, LOG_T_F);
			log_file_ << log_time_buffer_ << "WARNING mail receive failed" << std::endl;

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
			std::cout << "WARNING ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);
			std::cout << "mail receive failed" << std::endl;
		}
	}
}


int SmtpServer::SaveMailData(char *mail_list)
{
	int data_len = 0;
	int data_count = 0;
	data_file_.open(mail_list,std::ios::app);
	if (!data_file_.is_open())
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "ERROR open data file failed" << std::endl;
	}

	//����ʼ����
	data_file_ << END_OF_DATA;
	data_file_.width(8);
	data_file_ << 0 << std::endl;

	//����һ�������ʼ����ݰ�
	while (true)
	{
		data_len = recv(session_socket_, buffer_, buffer_size_, 0);
		//�������Ͽ�����
		if (data_len == -1)
		{
			GetTimeStamp(log_time_buffer_, LOG_T_F);
			log_file_ << log_time_buffer_ << "WARRING disconnected from the client" << std::endl;

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
			std::cout << "WARRING ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);
			std::cout << "disconnected from the client" << std::endl;

			return 1;
		}

		data_count += data_len;
		buffer_[data_len] = '\0';

		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "INFO receiving data......... " << data_len << " bytes" << std::endl;

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		std::cout << "INFO";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 5);
		std::cout << " receiving data......... " << data_len << " bytes" << std::endl;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);
		
		//д���ļ�
		data_file_ << buffer_;

		//DEBUG
#if SMTP_DEBUG
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		std::cout << "INFO";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);
		std::cout << " receive:  ";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
		std::cout << buffer_;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);
#endif

		//������ݽ�����־
		if (strcmp(CHECK_DATA_END(buffer_, data_len), END_OF_DATA) == 0)
		{
			//����ʼ�������
			data_file_.width(8);
			data_file_ << data_len << std::endl;
			data_file_.close();
			

			GetTimeStamp(log_time_buffer_, LOG_T_F);
			log_file_ << log_time_buffer_ << "INFO finished  ..... total: " << data_count << " bytes" << std::endl;

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
			std::cout << "INFO";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 5);
			std::cout << " finished  ..... total: " << data_count << " bytes" << std::endl;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);

			break;
		}
	}

	return 0;
}


int SmtpServer::ReadMailData(char *mail_list)
{
	int mail_size;
	int offset;

	data_file_.open(mail_list, std::ios::in);
	if (!data_file_.is_open())
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "ERROR open data file failed" << std::endl;

		return 1;
	}

	//��ȡ�ʼ���С
	data_file_.seekg(-10, std::ios::end);
	data_file_ >> mail_size;

	//��λ���ʼ����
	/*���ʼ���С�Ϳ���ֱ�Ӷ�λ���ʼ����
	 *��������seekg�������϶�λ���ȷС�ڴ������
	 *�²���Windows�ļ���ʽ��ԭ��
	 *����������ʹ���ʼ���С������λ������һ��forѭ����ϸ�Ƚ��ʼ����
	 *(Ϊ���ɻ���ؾ�����������취 �����QAQ ������~
	 */
	char cmp[END_OF_DATA_L+1];
	cmp[END_OF_DATA_L] = '\0';
	for (offset = -mail_size;; offset--)
	{
		data_file_.seekg(offset, std::ios::end);
		data_file_.read(cmp, END_OF_DATA_L);
		if (strcmp(cmp, END_OF_DATA) == 0)
		{
			offset += 17;
			break;
		}
	}

	//��ȡ�ʼ�����
	data_file_.seekg(offset, std::ios::end);
	data_file_.read(buffer_, mail_size);
	buffer_[mail_size] = '\0';

	data_file_.close();
	return 0;
}


int SmtpServer::ConnectRemote()
{
	session_socket_ = INVALID_SOCKET;
	remote_addr_ = "220.181.12.17";
	remote_port_ = 25;

	session_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (session_socket_ == INVALID_SOCKET)
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "ERROR session socket failed with error : "
			<< WSAGetLastError() << std::endl;

		WSACleanup();
		exit(SOCKET_CREAT_ERROR);
	}

	//��ʼ��Զ�̵�ַ
	sockaddr_in remote_adr;
	memset(&remote_adr, 0, sizeof(remote_adr));

	remote_adr.sin_family = AF_INET;
	remote_adr.sin_port = htons(remote_port_);
	remote_adr.sin_addr.S_un.S_addr = inet_addr(remote_addr_);

	//����Զ��SMTP������
	if (connect(session_socket_, (LPSOCKADDR)&remote_adr, sizeof(remote_adr)) == SOCKET_ERROR)
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "ERROR connect failed with error: "
			<< WSAGetLastError() << std::endl;

		closesocket(listen_socket_);
		closesocket(session_socket_);
		WSACleanup();
		exit(REMOTE_CONNECT_ERROR);
	}
	if (session_socket_ == INVALID_SOCKET)
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "WARNING unable to connect to the remote: " << std::endl;

		return 1;
	}

	//���ӳɹ�
	GetTimeStamp(log_time_buffer_, LOG_T_F);
	log_file_ << log_time_buffer_ << "INFO server connected to " << remote_addr_
		<< ":" << remote_port_ << "......" << std::endl;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	std::cout << "INFO server connected to " << remote_addr_
		<< ":" << remote_port_ << "......" << std::endl;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 7);

	return 0;

}


SmtpServer::~SmtpServer()
{
	delete[]buffer_;
	log_file_.close();
	closesocket(listen_socket_);
	WSACleanup();
}