#include "RequestParser.hpp"
#include "../../exception/IllegalStateException.hpp"
#include <iostream>

// 구현한 것보다 더 긴 메서드를 수신한 서버는 501(Not Implemented)상태 코드 로 응답해야 한다.(SHOULD)
s_method	setMethod(std::string __method)
{
	if (__method == "GET") return GET;
	else if (__method == "POST") return POST;
	else if (__method == "PUT") return PUT;
	else if (__method == "DELETE") return DELETE;
	else throw IllegalStateException("HTTP Request start line invalid method.");
}

// 현재 나와있는 version에만 유효해야 하는가?
float	setVersion(std::string __version)
{
	size_t	index;
	size_t	major;
	size_t	minor;
	size_t	major_index;
	char	*major_end;
	char	*minor_end;

	index = __version.find("HTTP/");
	if (index == std::string::npos)
		throw IllegalStateException("HTTP Request start line invalid version.");
	index += 5;
	major_index = index;
	major = ::strtol(&__version.at(index), &major_end, 10);
	if (*major_end != '.' || &__version.at(index) == major_end)
		throw IllegalStateException("HTTP Request start line invalid version.");
	index += major_end - &__version.at(index) + 1;
	minor = ::strtol(&__version.at(index), &minor_end, 10);
	if (*minor_end != '\r' || &__version.at(index) == minor_end)
		throw IllegalStateException("HTTP Request start line invalid version.");
	return (::strtod(&__version.at(major_index), &major_end));
}

// 최소 8000 octet 길이의 request-line을, 지원하는 것을 권장한 다.(RECOMMENDED)
RequestLine	RequestParser::parseRequestLine(std::string __requestLineString)
{
	RequestLine					requestLine;
	std::vector<std::string>	sp_splited;

	sp_splited = split(__requestLineString, SP);
	requestLine._method = setMethod(sp_splited[0]);
	// 구문 분석할 URI보다 긴 요청 대상을 수신한 서버는 414(URI Too Long) 상태 코드로 응답해야 합니다.
	requestLine._uri = sp_splited[1];
	requestLine._version = setVersion(sp_splited[2]);
	return (requestLine);
}

Headers		RequestParser::parseHeaders(std::string __headersString)
{
	Headers								headers;
	std::vector<std::string>			crlf_splited;

	// 같은 헤더가 여러 개일 수도 있음
	crlf_splited = split(__headersString, CRLF);
	for (size_t i = 0; i < crlf_splited.size(); i++)
	{
		std::string		fieldName;
		std::string		fieldValue;
		std::vector<std::string>	values;
		size_t			index;
		// 1. 문법 검증
		// 2. name, value split => 첫번째 :를 찾아서 2개로 나누면 될 듯
		index = crlf_splited[i].find(':');
		fieldName = crlf_splited[i].substr(0, index);
		// OWS => SP만 인지 확인하기
		fieldValue = crlf_splited[i].substr(crlf_splited[i][index + 1] == SP ? index + 2 : index + 1, crlf_splited[i].size());
		index = fieldValue.size() - 1;
		if (fieldValue[index] == SP)
			fieldValue.erase(index);
		// , 단위로 나누고 white space 제거
		values = split(fieldValue, ',');
		for (size_t i = 0; i < values.size(); i++)
			trim(values[i]);
		// 3. key가 있으면 value에 push_back(), 없으면 key, value를 insert()
		if (headers.keyExist(fieldName))
			headers.addFieldValue(fieldName, values);
		else
			headers.insertHeader(fieldName, values);
	}
	return (headers);
}