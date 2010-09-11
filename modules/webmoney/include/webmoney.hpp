#ifndef PG_WEBMONEY
#define PG_WEBMONEY
#include <upgm/upgm.hpp>

namespace PG
{
class WebMoney: public UPGM
{
	void registerUserHooks(Transport &transport, Parser &parser, Payment &payment);
};
}

#endif
