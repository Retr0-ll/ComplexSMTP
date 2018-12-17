#ifndef _TEST_
#include"test.h"
#endif

int main()
{
	Init(2, 2);
	int len;

	SOCKET s = INVALID_SOCKET;
	SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
	SSL *ssl = NULL;

	char inputbuf[100];

	while (true)
	{
		while (true)
		{
			std::cout << "$YuLiBao@DEBUG: ~$ ";
			std::cin.getline(inputbuf,100);

			if (strcmp(inputbuf, "#CMD connect") == 0)
			{
				ssl = Cnt(&s,ctx);
			}
			else if (strcmp(inputbuf, "#CMD disconnect") == 0)
			{
				SSL_shutdown(ssl);
				closesocket(s);
				std::cout << "Disconnected" << std::endl;
			}
			else if (strcmp(inputbuf, "#CMD end") == 0)
			{
				SSL_write(ssl, "\r\n.\r\n", 5);
				std::cout << "send >>" << "\\r\\n.\\r\\n" << std::endl;
			}
			else if (strcmp(inputbuf, "#CMD show") == 0)
			{
				len = SSL_read(ssl, inputbuf, 100);
				inputbuf[len] = '\0';

				std::cout << "receive >>" << inputbuf;
			}
			else
			{
				strcat_s(inputbuf, "\r\n");
				SSL_write(ssl, inputbuf, strlen(inputbuf));

				std::cout << "send >>" << inputbuf;
			}
		}
	}
}