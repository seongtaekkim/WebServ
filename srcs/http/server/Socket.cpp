#include "Socket.hpp"
#include "../../exception/IOException.hpp"

const bool Socket::_s_isReuse = true;

Socket::Socket(int fd) : FileDescriptor(fd) {}

Socket* Socket::create(void) {
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		throw IOException("socket create error : ", errno);
	return (new Socket(fd));
}

Socket::~Socket() throw() {}

void Socket::bind(void) {
	this->validateNotClosed();
    struct sockaddr_in server_addr;
    ::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(8080);

    if (::bind(this->getFd(), (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        throw IOException("bind error : ", errno);
}

void Socket::listen(void) {
	this->validateNotClosed();
    if (::listen(this->getFd(), DEFAULT_BACKLOG) == -1)
        throw IOException("listen() error", errno);
}

// 클라이언트 소켓 생성
Socket* Socket::accept() {
	this->validateNotClosed();
	struct sockaddr_storage addr;
	socklen_t len = sizeof(addr);

	int fd = ::accept(this->getFd(), (struct sockaddr*)&addr, &len);
	if (fd == -1)
		throw IOException("accept", errno);

    ///fcntl(client_socket, F_SETFL, O_NONBLOCK);
	return (new Socket(fd));
}

ssize_t Socket::recv(void *buffer, std::size_t length, int flags) {
	std::cout << "socket:recv : " << this->getFd() << std::endl;
	this->validateNotClosed();
	ssize_t ret;
	if ((ret = ::recv(this->getFd(), buffer, length, flags)) == -1)
		throw IOException("recv error! : ", errno);
	return (ret);
}

std::string sample() {
std::string str;
// str.append("GET / HTTP/1.1");
// str.append("\nUser-Agent: test");
// str.append("\nAccept: */*");
// str.append("\nHost: localhost:8080");
// str.append("\nAccept-Encoding: gzip, deflate, br");
// str.append("\nConnection: keep-alive");
 
str.append("\nHTTP/1.1 200 OK");
str.append("\nContent-Length: 70");
str.append("\nContent-Type: text/html");
str.append("\nDate: Thu, 30 Mar 2023 19:00:10 GMT");
str.append("\nServer: webserv\r\n\r\n");
 
str.append("<html>");
str.append("<head>");
str.append("<title>Listing of /</title>");
str.append("</head>");
str.append("<body>");
str.append("test");
str.append("</body>");
str.append("</html>");
//std::cout << "str size : " << str.length() << std::endl;
return str;
}

ssize_t Socket::send(const void *buffer, size_t length, int flags) {
	this->validateNotClosed();
	ssize_t ret;
	std::cout << "send data : " << static_cast<const char *>(buffer) << std::endl;
	
	std::string s = std::string(static_cast<const char*>(buffer));

	std::string tmps = sample();
	const char* tmp = tmps.c_str();
	if ((ret = ::send(this->getFd(), tmp, tmps.size(), flags)) == -1)
	// if ((ret = ::send(this->getFd(), buffer, s.size(), flags)) == -1)
		throw IOException("send error : ", errno);
	std::cout << "send ret : " << ret << std::endl;
	return (ret);
}

//https://www.joinc.co.kr/w/Site/Network_Programing/AdvancedComm/SocketOption
void Socket::reuse() {
	this->validateNotClosed();
	if (::setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, &Socket::_s_isReuse, sizeof(int)) == -1)
		throw IOException("setsockopt error : ", errno);
}