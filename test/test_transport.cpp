#include <test/test_transport.hpp>

#include <iostream>
#include <sstream>

namespace TEST
{


void TestTransport::operator<<(const std::string & data)
{
	std::cout << data << std::endl;
}

PG::DataTree TestTransport::operator>>(std::string & buffer)
{
	if (_answer.size() > 0)
	{
		buffer = _answer.front();
		_answer.pop();
	}
	return PG::DataTree();
}

TestTransport::TestTransport(const std::queue<std::string> & answer)
{
	_answer = answer;
}

}
