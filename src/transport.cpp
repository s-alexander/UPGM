#include <upgm/transport.hpp>

namespace PG
{

Transport::Transport(Log & log): _log(log)
{
}

Transport::~Transport() throw()
{
}

void Transport::operator<<(const std::string & data) {
	_log << " >>> SEND: >>>\n";
	_log << data;
	_log << "^^^\n";
	this->writeImpl(data);
}

DataTree Transport::operator>>(std::string & buffer) {
	const DataTree result (this->readImpl(buffer));
	_log << " <<< RECV: <<<\n";
	_log << buffer << "\n";
	_log << "^^^\n";
	return result;
}

void Transport::configure(const DataTree & config)
{
	_config = config;
}

}
