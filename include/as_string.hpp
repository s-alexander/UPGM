#ifndef PG_AS_STRING
#define PG_AS_STRING

#include <limits>
#include <string>

namespace PG
{

template <typename T>
static const char * formatter() { return 0; }

template <>
const char * formatter<float>() { return "%.2f"; }

template <>
const char * formatter<int>() { return "%i"; }

template <>
const char * formatter<long int>() { return "%li"; }

template <>
const char * formatter<long long int>() { return "%lli"; }

template <typename T>
static std::string asString(T num)
{
	const size_t size = std::numeric_limits<T>::digits + 2;
	char buff[size];
	snprintf(buff, size, formatter<T>(), num);
	return buff;
}

template <>
std::string asString<const char *>(const char * str)
{
	return (0 != str) ? std::string(str) : std::string();
}

}

#endif
