#include <upgm/transport.hpp>
#include <queue>

namespace TEST
{

class TestTransport: public PG::Transport
{
public:
	TestTransport(const std::queue<std::string> & answer);
	~TestTransport() throw() { ;; }
	virtual void operator<<(const std::string & data);
	virtual PG::DataTree operator>>(std::string & buffer);
private:
	std::queue<std::string> _answer;
};

}
