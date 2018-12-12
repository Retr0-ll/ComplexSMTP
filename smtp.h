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
#define LOG_T_MAXLEN 50


/*
* RB[��λ�ظ���] �����˷���������Ӧ�б�
*/
#define RB220 "220 localhost\r\n"
#define RB250_EXT "250-AUTH LOGIN PLAINOK\r\n250-AUTH=LOGIN PLAIN\r\n250-STARTTLS\r\n250 8BITMIME\r\n"
#define RB334_USER "334 dXNlcm5hbWU6\r\n"
#define RB334_PASS "334 UGFzc3dvcmQ6\r\n"
#define RB235 "235 Authentication successful\r\n"
#define RB250 "250 OK\r\n"
#define RB354 "354 End data with <CR><LF>.<CR><LF>\r\n"
#define RB221 "221 Bye\r\n"

#define RB500 "500 Command Error\r\n"
#define RB550 "550 MI:IMF\r\n"


/*
* ����ͻ��˵������б�����ֶΣ���鳤��
*/
#define EHLO "EHLO SimpleSmtp\r\n"
#define EHLO_C "EHLO"
#define EHLO_L 4

#define AL "AUTH LOGIN\r\n"
#define AL_L 12
#define MF_C "MAIL FROM: "
#define MF_L 11
#define RT_C "RCPT TO: "
#define RT_L 9
#define DATA "DATA\r\n"
#define DATA_L 6
#define QT "QUIT\r\n"
#define QT_L 6
#define RS "RSET\r\n"
#define RS_L 6

#define END_OF_DATA "\r\n.\r\n"
#define END_OF_DATA_L 5


/*
* �꺯�� CHECK_DATA_END ���� �ʼ����ݽ�������ĵ�ַ
* buffer��ָ�򻺳��ָ��
* data_len�����ݿ�ĳ���
* cmd ��Ҫ��������
*/
#define CHECK_DATA_END(buffer, data_len) (buffer+data_len-END_OF_DATA_L)


/*
* �꺯�� GET_PARA ���ض�Ӧ����Ĳ�����ַ
* buffer��ָ�򻺳��ָ��
* cmd����Ҫ��ò���������
*/
#define GET_PARA(buffer, cmd) (buffer+strlen(cmd))


/*
* �������˳��Ĵ�����
*/
enum SVR_E{
	FILE_OPEN_ERROR,
	BUFFER_GET_ERROR,
	SOCKET_LOAD_ERROR,
	SOCKET_CREAT_ERROR,
	SOCKET_BIND_ERROR,
	SOCKET_LISTEN_ERROR,
	REMOTE_CONNECT_ERROR
};

/********
* ��ȡָ����ʽ��ʱ���
* char *output_time �������������ָ����ʽ���ַ�����ʽʱ���
* const char *format ��ʽ
********/
void GetTimeStamp(char *output_time, const char *format);


/*����Socket��Դ*/
void LoadSocket(int major_version, int minor_version);

/*һЩUI ����*/


class SmtpServer
{
private:
	/*��������ַ���˿ڡ������������׽��֡���ǰ�Ự�׽���*/
	const char *listen_addr_;
	unsigned short listen_port_;
	SOCKET listen_socket_;
	SOCKET session_socket_;

	/*Զ�������ĵ�ַ�Ͷ˿�*/
	const char *remote_addr_;
	unsigned short remote_port_;


public:
	/*********
	*�ص��������Ͷ���
	**********/
	typedef int(*CallBack)(SmtpServer &);

	/*���������ջ���*/
	char* buffer_;

	/*������SMTPͨ��״̬ ���ڻص������д����߼������*/
	int state_;
	int exstate_;


private:
	/*�����С*/
	int buffer_size_;

	/*��������־ �ʼ������ļ� ʱ�������*/
	std::ofstream log_file_;
	std::fstream data_file_;
	char log_time_buffer_[LOG_T_MAXLEN];

public:
	/***********
	*���캯����Log�ļ�����ʼ�������������׽��֡��������������
	*���������ͷ�SOCKET��Դ���ͷŷ��������塢�ر�Log�ļ�
	***********/
	SmtpServer(int buffer_size);
	~SmtpServer();


	/***********
	 *Listen ��������˿� �󶨵�ַ(Ĭ��127.0.0.1)�Ͷ˿ڲ���ʼ����
	 *Start  ��������������ʼ�������ӣ������ûص�������������
	***********/
	void Listen(unsigned short listen_port);
	void Start(CallBack server_logic, CallBack client_logic, SmtpServer& svr);


	/*�ú�����Server�ص������е��ã����յ�DATA����󣬴����ʼ����ļ���*/
	int SaveMailData(char *mail_list);
	/*�ú�����Client�ص������е��ã������ʼ��б�����һ���ʼ�*/
	int ReadMailData(char *mail_list);

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

private:
	/*����Ĭ�ϵ�Զ��SMTP������ ��Client�ص�֮ǰִ�� */
	int ConnectRemote();
};
