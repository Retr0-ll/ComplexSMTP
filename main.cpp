#ifndef _SMTP_
#include"smtp.h"
#endif

#ifndef _IOSTREAM_
#include <iostream>
#endif

#define BUFFER_SIZE 1024*10 //�ֽ�

int main()
{
	LoadSocket(2, 2);

	SmtpServer svr(BUFFER_SIZE);
	svr.Listen(25);
	svr.Start();

	return 0;
}