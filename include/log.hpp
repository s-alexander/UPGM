#ifndef PG_LOG
#define PG_LOG

#include <string>
#include <fstream>
#include <upgm/data_tree.hpp>

namespace PG
{

class Log
{
public:
	Log();
	virtual ~Log() throw();
	void setup(const std::string & path, const std::string & payment);

	template <typename T>
	Log & operator<<(T data) throw() {
		try {
			if ( _log && _log->good() ) {
				(*_log) << data;
				_log->flush();
			}
		} catch (...) { ;; }
		return *this;
	}

private:
	std::ofstream * _log;
};

}

#endif
