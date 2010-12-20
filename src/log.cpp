#include <upgm/log.hpp>

namespace PG
{

Log::Log():_log(0)
{
}

Log::~Log() throw()
{
	if ( _log ) {
		_log->close();
		delete _log;
	}
}

void Log::setup(const std::string & path, const std::string & paymentId)
{
	if (_log) {
		_log->close();
		delete _log;
		_log = 0;
	}
	if (path.length() > 0) {
		const std::string filename (path + "/" + paymentId + ".log");
		_log = new std::ofstream(filename.c_str(), std::ios_base::app | std::ios_base::out);
	}
}

}
