#include <upgm/payment.hpp>

namespace PG
{

DataTree Payment::generateDataTree() const
{
	DataTree result;
	result.set("id", "13");
	return result;
}


}
