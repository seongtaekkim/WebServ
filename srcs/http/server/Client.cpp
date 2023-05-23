#include "Client.hpp"
#include "../response/method/Method.hpp"
#include "../../iom/KqueueManage.hpp"

int Client::_s_connCnt = 0;


Client::Client(InetAddress inetAddress, Server& server, Socket& socket)
	: _inetAddress(inetAddress), _server(server), _socket(socket), _in(this->_socket), _out(this->_socket)
	, _req(), _res(), _maker(this->_req, this->_res, *this), _parser(*this), _pathParser(), _putTask(), _cgiTask() {
	Client::_s_connCnt++;
	this->_currProgress = Client::HEADER;
	KqueueManage::instance().create(this->_socket, *this);
	updateTime();
}

Client::Client(const Client& other) 
	: _inetAddress(other._inetAddress), _server(other._server)
	, _socket(other._socket), _in(this->_socket), _out(this->_socket)
	, _req(other._req), _res(other._res), _maker(other._maker), _parser(other._parser) {}

Client::~Client(void) {
	Client::_s_connCnt--;
	// delete &this->_in;
	// delete &this->_out;
	// this.
	std::cout << "ininin" << std::endl;
	// CGI&  cgi = this->_cgiTask->cgi();
	// std::cout << cgi.pid()<<  std::endl;
	// cgi.file().remove();
	std::cout << "client disconenct !! " << this->_socket.getFd() << std::endl;
	if (this->_putTask)
		ReleaseResource::pointer(this->_putTask);
	if (this->_cgiTask)
		ReleaseResource::pointer(this->_cgiTask);
	this->_server.disconnect(*this);
	delete &this->_socket;
}

Socket& Client::socket() const {
	return (this->_socket);
}

// recv 0 == client에서 접속종료
bool Client::recv(FileDescriptor &fd) {
	(void)fd;
	if (this->_in.recv() <= 0) {
		delete this;
		return (false);
	}

	std::cout << "request start ===============================" << std::endl;
	// std::cout << this->_in.storage() << std::endl;

	// Request test(this->_in.storage());

	// std::cout << "< Request Line >	\n";
	// std::cout << test.method() << " " << test.uri() << " " << test.version() << std::endl;
	// std::cout << "< Headers > \n";
	// const auto& headers = test.headers();
	// for (auto it = headers.cbegin(); it != headers.cend(); ++it) {
	// 	std::cout << it->first << ": ";
	// 	const auto& values = it->second;
	// 	for (auto valueIt = values.cbegin(); valueIt != values.cend(); ++valueIt) {
	// 		std::cout << *valueIt;
	// 		if (valueIt != values.cend() - 1) {
	// 			std::cout << ", ";
	// 		}
	// 	}
	// 	std::cout << std::endl;
	// }
	// std::cout << "< Body > \n";
	// std::cout << test.body() << std::endl;
	std::cout << "request end ===============================" << std::endl;
	std::cout << "receive=================================================================" << std::endl;
	// std::cout << this->_in.storage() << std::endl;
	std::cout << "receive end =================================================================" << std::endl;
	this->progress();
	updateTime();
	std::cout << "progress end !!!!!!!" << std::endl;
	return (true);
}

bool Client::send(FileDescriptor& fd) {
	
	// response 종료 체크
	// m_out size 체크
	(void)fd;
	std::cout<< "send in in ini " << std::endl;

	if (this->_currProgress == Client::END)
		return (false);
	if (this->_res.isEnd() != true)
		return (false);
    ssize_t ret = 0;
	// request, response 로직에서 생서한 응답버퍼 _out 를 send해야 함.
    // if ((ret = this->_out.send()) > 0)
    // if ((ret = this->_in.send()) > 0)
		//std::cout << "out ret : " << ret << std::endl;
 		// 시간 체크
		// std::cout << this->_res.body() << std::endl;
	// _out.store(this->_res.body());
	std::cout << "send================================================================= " << this->_currProgress << std::endl;
	this->_res.store(_out);
	// std::cout << this->_out.storage() << std::endl;
	std::cout << "send end ===============================================" << std::endl;
	ret = this->_out.send();
	if (ret > 0)
		updateTime();
		std::cout << "send end ===============================================" << std::endl;
	this->_currProgress = Client::END;
	// if (ret == -1)
	// 	delete this;
	_out.clear();
	_in.clear();
	return (true);
}

bool Client::progress(void) {

	switch (this->_currProgress) {
		case Client::HEADER:
			return (progressHead());

		case Client::BODY:
			return (progressBody());

		case Client::END:
			return (true);
	}
	return (false);
}


bool Client::progressHead(void) {

	char c;
	while (this->_in.getC(c)) {
		std::cout << c ;
		this->_in.next();
		//bool catched = true;
		try {
			_parser.parse(c);
			// if (m_parser.state() == HTTPRequestParser::S_END)
			// {
			// 	// std::cout << "******" << std::endl;
			// 	m_request = HTTPRequest(m_parser.version(), m_parser.url(), m_parser.headerFields());
			// 	m_filterChain.doChainingOf(FilterChain::S_BEFORE);

			// 	if (m_request.method().absent() && m_response.ended())
			// 	{
			// 		m_filterChain.doChainingOf(FilterChain::S_AFTER);
			// 		m_state = S_END;
			// 		return (true);
			// 	}
				

			// }
			if (this->_parser.state() == Parser::END) {
				std::cout << "in " << std::endl;
				URL url = URL().builder().appendPath(_parser.pathParser().path()).build();
				_req = Request(_parser.header() ,StatusLine(), url);
				std::cout << _parser.pathParser().path() << Method::METHOD[this->parser().method()] << std::endl;
				if (!Method::METHOD[this->parser().method()]) {
					this->_res.header().contentLength(0);
					this->_res.status(HTTPStatus::STATE[HTTPStatus::METHOD_NOT_ALLOWED]);
					this->_maker.setLastMaker();
					this->_maker.executeMaker();
				} else if (Method::METHOD[this->parser().method()]->getHasBody() == true) {
					_parser.state(Parser::BODY);
					this->_currProgress = Client::BODY;
					_parser.parse(0);
					if (_parser.state() != Parser::END)
						return (progressBody());
					else {
						this->_maker.setMaker();
						this->_maker.executeMaker();
						// this->_res.header().contentLength(0);
						// this->_res.status(HTTPStatus::STATE[HTTPStatus::METHOD_NOT_ALLOWED]);
					}
				} else {
					this->_maker.setMaker();
					this->_maker.executeMaker();
					this->_res.status(HTTPStatus::STATE[HTTPStatus::OK]);
					// this->_currProgress = Client::END;
				}
				break;
			}
			// catched = false;
		} catch (Exception &exception) {
			std::cerr << "Failed to process header: " << exception.message() << std::endl;
			this->_res.status(HTTPStatus::STATE[HTTPStatus::BAD_REQUEST]);
		}

		// if (catched)
		// {
		// 	NIOSelector::instance().update(m_socket, NIOSelector::NONE);
		// 	m_filterChain.doChainingOf(FilterChain::S_AFTER);
		// 	m_state = S_END;

		// 	break;
		// }
	}

	return (true);
}

bool Client::progressBody(void) {
	if (!this->_in.storage().empty()) {
		try {
			_parser.parse(0);

			// if (m_parser.state() == HTTPRequestParser::S_END)
			// {
				// if (m_response.ended())
				// {
					this->_maker.setMaker();
					this->_maker.executeMaker();
					// this->_maker .doChainingOf(FilterChain::S_AFTER);
				// }
				// m_filterChain.doChainingOf(FilterChain::S_BETWEEN);
				this->_res.status(HTTPStatus::STATE[HTTPStatus::OK]);
				// KqueueManage::instance().setEvent(this->_socket.getFd(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
				return (true);
				// }
		} catch (Exception &exception) {
			this->_res.status(HTTPStatus::STATE[HTTPStatus::BAD_REQUEST]);
			this->_maker.setMaker();
			this->_maker.executeMaker();
			// this->_currProgress = Client::END;
		}
	}
	return (false);
}


int Client::state(void) {

	if (this->_res.body() && this->_res.body()->isEnd() == true)
		this->_currProgress = Client::END;


	return (this->_currProgress);
}

Parser& Client::parser(void) {
	return (this->_parser);
}

Response& Client::response(void) {
	return (this->_res);
}

ResponseMaker& Client::maker(void) {
	return (this->_maker);
}

void Client::fileWrite(PutTask &task) {
	if (_putTask)
		delete _putTask;
	_putTask = &task;
}

void Client::cgiWrite(CGITask& task) {
	if (_cgiTask)
		delete _cgiTask;
	_cgiTask = &task;
}

PutTask* Client::fileWrite(void) {
	return (this->_putTask);
}

CGITask* Client::cgiWrite(void) {
	return (this->_cgiTask);
}

std::string& Client::body(void) {
	return (this->_body);
}

SocketStorage& Client::in(void) {
	return (this->_in);
}

SocketStorage& Client::out(void) {
	return (this->_out);
}

void Client::end() {
	this->_currProgress = Client::END;
}

Request& Client::request(void) {
	return (this->_req);
}

Server& Client::server(void) {
	return (this->_server);
}

InetAddress Client::inetAddress(void) const {
	return (this->_inetAddress);
}

void Client::updateTime(void) {
	unsigned long time = Time::currentSecond();
	if (time)
		this->_lastTime = time;
}

unsigned long Client::lastTime(void) const {
	return (this->_lastTime);
}

void Client::deny(Client& client) {
	std::cout << "deny" << std::endl;
	client.response().status(HTTPStatus::STATE[HTTPStatus::SERVICE_UNAVAILABLE]);
	client.maker().setLastMaker();
	client.maker().executeMaker();
	KqueueManage::instance().setEvent(client.socket().getFd(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
}
