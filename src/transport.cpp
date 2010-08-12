#include <upgm/transport.hpp>

namespace PG
{

Transport::Transport()
{
}

Transport::~Transport() throw()
{
}

void Transport::configure(const DataTree & config)
{
	_config = config;
}

}
