#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "RequestMessage.hpp"
#include <list>

// 1. 요청을 받는다 : HTTP 요청 메시지를 네트워크로부터 읽어 들인다. => RequestMessage class
// 2. 요청을 처리한다 : 요청 메시지를 해석하고 행동을 취한다. => RequestAction class
// 3. 리소스에 접근한다 : 메시지에서 지정한 리소스에 접근한다. => RequestResource class

class Request {
public:

private:
	std::list<RequestMessage>	_list;

};

#endif