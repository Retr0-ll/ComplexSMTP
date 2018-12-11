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
		std::cout << "ERROR wsastartup failed with error: " << error << std::endl;
		exit(1);
	}
}


SmtpServer& operator<<(SmtpServer& server, const char *data_send)
{
	//��������
	send(server.session_socket_, data_send, strlen(data_send), 0);

	//��¼��־���������׼���
	GetTimeStamp(server.log_time_buffer_, LOG_T_F);
	server.log_file_ << server.log_time_buffer_ << "INFO send:  " << data_send;
	std::cout << "INFO send:  " << data_send;

	return server;
}


int operator>>(SmtpServer& server, char *data_receive)
{
	//�������ݣ����û����������������
	int data_len = 0;
	data_len = recv(server.session_socket_, data_receive, server.buffer_size_, NULL);
	GetTimeStamp(server.log_time_buffer_, LOG_T_F);

	//��¼��־���������׼���
	data_receive[data_len] = '\0';
	server.log_file_ << server.log_time_buffer_ << "INFO receive:  " << data_receive;
	std::cout << "INFO receive:  " << data_receive;
	
	return data_len;
}


SmtpServer::SmtpServer(int buffer_size) :listen_socket_(INVALID_SOCKET), buffer_size_(buffer_size), buffer_(NULL)
{
	//ͨ����ʽ LOG_FN_F ��ȡLOG�ļ���
	char log_fn[30];
	GetTimeStamp(log_fn, LOG_FN_F);

	//��Log�ļ�
	log_file_.open(log_fn);

	//��ȡsocket ��ַ��ipv4 ��ʽSOCKET Э��TCP
	listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket_ == INVALID_SOCKET)
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "ERROR socket failed with error : "
			<< WSAGetLastError() << std::endl;

		WSACleanup();
		exit(2);
	}

	//���뻺���ڴ�
	buffer_ = new char[buffer_size_];
	if (buffer_ == NULL)
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "ERROR failed to new a " << buffer_
			<< "bytes buffer" << std::endl;

		WSACleanup();
		exit(4);
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
		exit(3);
	}

	//��ʼ�����˿�
	if (listen(listen_socket_, SOMAXCONN) == SOCKET_ERROR)
	{
		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "ERROR listen failed with error: "
			<< WSAGetLastError() << std::endl;

		closesocket(listen_socket_);
		WSACleanup();
		exit(3);
	}

	GetTimeStamp(log_time_buffer_, LOG_T_F);
	log_file_<< log_time_buffer_<< "INFO server listenning on " << listen_addr_
		<< ":" << listen_port_ << "......" << std::endl;

	std::cout << "INFO server listenning on " << listen_addr_
		<< ":" << listen_port_ << "......" << std::endl;

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

		if (session_socket_ == INVALID_SOCKET)
		{
			log_file_ << GetTimeStamp << "WARNING accept failed with error: "
				<< WSAGetLastError() << std::endl;

			continue;
		}

		log_file_ << log_time_buffer_ << "INFO accepted a connection from " << inet_ntoa(host_addr.sin_addr)
			<< ":" << ntohs(host_addr.sin_port) << std::endl;
		std::cout << "INFO accepted a connection from " << inet_ntoa(host_addr.sin_addr)
			<< ":" << ntohs(host_addr.sin_port) << std::endl;


		//Ȼ����ûص�������ʼSMTP SERVER�߼�
		if (server_logic(svr) == 0)
		{
			closesocket(session_socket_);
			log_file_ << log_time_buffer_ << "INFO mail receive succeed" << std::endl;
			std::cout << "INFO mail receive success" << std::endl;
			//����Զ�̷�����  ���ûص����� ��ʼSMTP Client�߼�
			if (ConnectRemote() == 0)
			{
				if (client_logic(svr) == 0)
				{
					closesocket(session_socket_);
					log_file_ << log_time_buffer_ << "INFO mail send succeed" << std::endl;
					std::cout << "INFO mail send succeed" << std::endl;
				}
				else
				{
					closesocket(session_socket_);
					log_file_ << log_time_buffer_ << "WARNING mail send succeed" << std::endl;
					std::cout << "INFO mail send succeed" << std::endl;
				}
			}
			closesocket(session_socket_);
		}
		else
		{
			closesocket(session_socket_);

			GetTimeStamp(log_time_buffer_, LOG_T_F);
			log_file_ << log_time_buffer_ << "WARNING mail receive failed" << std::endl;
			std::cout << "WARNING mail receive failed" << std::endl;
		}
	}
}


int SmtpServer::SaveMailData(char *mail_list)
{
	int data_len = 0;
	int data_count = 0;
	data_file_.open(mail_list,std::ios::app);
	data_file_ << END_OF_DATA;

	//����ʼ����
	data_file_.width(8);
	data_file_ << 0 << std::endl;

	while (true)
	{
		data_len = recv(session_socket_, buffer_, buffer_size_, 0);

		//�������Ͽ�����
		if (data_len == -1)
		{
			GetTimeStamp(log_time_buffer_, LOG_T_F);
			log_file_ << log_time_buffer_ << "WARRING disconnected from the client" << std::endl;
			std::cout<< "WARRING disconnected from the client" << std::endl;

			return 1;
		}
		data_count += data_len;
		buffer_[data_len] = '\0';

		GetTimeStamp(log_time_buffer_, LOG_T_F);
		log_file_ << log_time_buffer_ << "INFO receiving data......... " << data_len <<" bytes"<<std::endl;
		std::cout << "INFO receiving data......... " << data_len << " bytes" << std::endl;
		
		//д���ļ�
		data_file_ << buffer_;
		std::cout << "INFO receive:  " << buffer_;

		//������ݽ�����־
		if (strcmp(CHECK_DATA_END(buffer_, data_len), END_OF_DATA) == 0)
		{
			//����ʼ�������
			data_file_.width(8);
			data_file_ << data_len << std::endl;
			data_file_.close();
			

			GetTimeStamp(log_time_buffer_, LOG_T_F);
			log_file_ << log_time_buffer_ << "INFO finished  ..... total: " << data_count << " bytes" << std::endl;

			std::cout << "INFO finished  ..... total: " << data_count << " bytes" << std::endl;
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

	//��ȡ�ʼ���С
	data_file_.seekg(-10, std::ios::end);
	data_file_ >> mail_size;

	//��λ���ʼ����
	char cmp[6];
	cmp[5] = '\0';
	for (offset = -mail_size;; offset--)
	{
		data_file_.seekg(offset, std::ios::end);
		data_file_.read(cmp, 5);
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
		exit(2);
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
		exit(3);
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

	std::cout << "INFO server connected to " << remote_addr_
		<< ":" << remote_port_ << "......" << std::endl;

	return 0;

}


SmtpServer::~SmtpServer()
{
	delete[]buffer_;
	log_file_.close();
	closesocket(listen_socket_);
	WSACleanup();
}