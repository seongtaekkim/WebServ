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

ssize_t Socket::send(const void *buffer, size_t length, int flags) {
	this->validateNotClosed();
	ssize_t ret;
	std::cout << "send data : " << static_cast<const char *>(buffer) << std::endl;
	
		std::string s = std::string(static_cast<const char*>(buffer));
	if ((ret = ::send(this->getFd(), buffer, s.size(), flags)) == -1)
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