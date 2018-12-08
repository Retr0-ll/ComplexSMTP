#pragma once

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#ifndef _WINSOCK2API_
#include <WinSock2.h>
#endif

#ifndef _SMTP_
#define SMTP
#endif

#ifndef _FSTREAM_
#include <fstream>
#endif

/*
* LOG_FN_F �����˷�����Log�ļ���Ŀ¼�Լ�������ʽ
* LOG_T_F  �����˷�����Log�ļ���ʱ�������ʽ
* LOG_T_MAXLEN ������LOG_FN_F �� LOG_T_F ��ʽ��Ӧ����󳤶�
*/
#define LOG_FN_F ".\\Log\\Log-%Y%m%d%H%M%S.txt"
#define LOG_T_F "%m-%d-%H:%M:%S ----- "
#define LOG_T_MAXLEN 30

/*
* RB[��λ�ظ���] �����˷���������Ӧ�б�
*/
#define RB220 "220 localhost\r\n"
#define RB250 "250 OK\r\n"
#define RB354 "354 End data with <CR><LF>.<CR><LF>\r\n"
#define RB221 "221 Bye\r\n"

/*
* ����ͻ��˵ĵ������б�
*/
#define EHLO "EHLO "
#define MF "MAIL FROM: "
#define RT "RCPT TO: "
#define DATA "DATA"
#define QT "QUIT"
#define END_OF_DATA "\r\n.\r\n"

/*
* �꺯�� CHECK_DATA_END ���� �ʼ����ݽ�������ĵ�ַ
* buffer��ָ�򻺳��ָ��
* data_len�����ݿ�ĳ���
* cmd ��Ҫ��������
*/
#define CHECK_DATA_END(buffer, data_len) (buffer+data_len-strlen(END_OF_DATA))


/*
* �꺯�� GET_PARA ���ض�Ӧ����Ĳ�����ַ
* buffer��ָ�򻺳��ָ��
* cmd����Ҫ��ò���������
*/
#define GET_PARA(buffer, cmd) (buffer+strlen(cmd))










/********
* ��ȡָ����ʽ��ʱ���
* char *output_time �������������ָ����ʽ���ַ�����ʽʱ���
* const char *format
********/
void GetTimeStamp(char *output_time, const char *format);

void LoadSocket(int major_version, int minor_version);











class SmtpServer
{
private:
	/*��������ַ���˿ڡ��Ự�׽��֡������������׽���*/
	const char *listen_addr_;
	unsigned short listen_port_;
	SOCKET listen_socket_;
	SOCKET session_socket_;
public:
	/*********
	*�ص��������Ͷ���
	**********/
	typedef void(*CallBack)(SmtpServer &);

	/*���������ջ���*/
	char* buffer_;


private:
	/*�����С*/
	int buffer_size_;

	/*��������־ �ʼ������ļ� ʱ�������*/
	std::ofstream log_file_;
	std::ofstream data_file_;
	char log_time_buffer_[LOG_T_MAXLEN];

public:
	/***********
	*���캯����Log�ļ�����ʼ�������������׽��֡��������������
	*���������ͷ�SOCKET��Դ���ͷŷ��������塢�ر��ʼ������ļ����ر�Log�ļ�
	***********/
	SmtpServer(int buffer_size);
	~SmtpServer();

	/***********
	 *Listen ��������˿� �󶨵�ַ(Ĭ��127.0.0.1)�Ͷ˿ڲ���ʼ����
	 *Start  ��������������ʼ�������� 
	***********/
	void Listen(unsigned short listen_port);
	void Start(CallBack callback, SmtpServer& svr );

	/*�ú����ڻص������е��ã����յ�DATA����󣬴����ʼ����ļ���*/
	void SaveMailData();

	/***********
     * SmtpServer�������� << �� >>��������������¶������������������Ϊ
	*
	* SmtpServer& operator<<(SmtpServer&, char *send);
	* ��Զ���Ѿ����ӵĿͻ��˷�������
	*
	* void operator>>(SmtpServer&, char *receive);
	* ��Զ���Ѿ����ӵĿͻ��˽�������, �洢�ڻ�����receive�У����ؽ��յ�������
	* receive�Ĵ�С ͳһ��λServer�໺���Ա buffer_size_��һ������£�receiveֱ�������Աbuffer_
	*
	* << �� >> ���ڱ�׼�����Log�ļ���ͬʱ��¼����/��������
	***********/
	friend SmtpServer& operator<<(SmtpServer& server, const char *data_send);
	friend int operator>>(SmtpServer& server, char *data_receive);
public:
};
