#ifndef PG_PAYMENT
#define PG_PAYMENT

#include <upgm/data_tree.hpp>

namespace PG
{

class Payment
{
public:
	DataTree generateDataTree() const;
};

}

#endif
