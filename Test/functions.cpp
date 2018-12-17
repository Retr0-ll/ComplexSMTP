#include"test.h"

#pragma comment (lib,"Ws2_32.lib")


#pragma comment (lib,"libcrypto.lib")
#pragma comment (lib,"libssl.lib")

void Init(int major_version, int minor_version)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);

	WSADATA wsadata;
	WORD socket_version = MAKEWORD(major_version, minor_version);
	WSAStartup(socket_version, &wsadata);

	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();


}

SSL* Cnt(SOCKET *s, SSL_CTX *ctx)
{
	*s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in addri;
	memset(&addri, 0, sizeof(addri));

	addri.sin_family = AF_INET;
	addri.sin_port = htons(465);
	addri.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	connect(*s, (LPSOCKADDR)&addri, sizeof(addri));

	SSL *ssl = SSL_new(ctx);
	SSL_set_fd(ssl, *s);
	SSL_connect(ssl);

	std::cout << "Connected" << std::endl;
	return ssl;
}