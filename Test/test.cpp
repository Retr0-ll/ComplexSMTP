#ifndef _TEST_
#include"test.h"
#endif

int main()
{
	Init(2, 2);
	int len;

	SOCKET s = INVALID_SOCKET;

	char inputbuf[100];

	while (true)
	{
		while (true)
		{
			std::cout << "$YuLiBao@DEBUG: ~$ ";
			std::cin.getline(inputbuf,100);

			if (strcmp(inputbuf, "#CMD connect") == 0)
			{
				s = Cnt();
			}
			else if (strcmp(inputbuf, "#CMD disconnect") == 0)
			{
				closesocket(s);
				std::cout << "Disconnected" << std::endl;
			}
			else if (strcmp(inputbuf, "#CMD end") == 0)
			{
				send(s, "\r\n.\r\n", 5, 0);
				std::cout << "send >>" << "\\r\\n.\\r\\n" << std::endl;
			}
			else if (strcmp(inputbuf, "#CMD show") == 0)
			{
				len = recv(s, inputbuf, 100, 0);
				inputbuf[len] = '\0';

				std::cout << "receive >>" << inputbuf;
			}
			else
			{
				strcat_s(inputbuf, "\r\n");
				send(s, inputbuf, strlen(inputbuf), 0);

				std::cout << "send >>" << inputbuf;
			}
		}
	}
}