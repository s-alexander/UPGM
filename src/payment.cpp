#include <upgm/payment.hpp>

namespace PG
{

DataTree Payment::generateDataTree() const
{
	DataTree result;
	result.set("id", "13");
	result.set("data", "Alex Sergeev$Test$123123$$573");
	return result;
}


}
