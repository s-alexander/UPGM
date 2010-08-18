#include <upgm/path.hpp>
#include <cstdio>

namespace PG
{

Path pathFromString(const std::string & string)
{
	Path result;
	const char escape = '\\';
	const char separator = '.';
	size_t begin = 0;
	size_t end = 0;
	bool escaped = false;
	std::string buff;
	size_t i (0);
	fprintf(stderr, "converting %s to path\n", string.c_str());
	for (std::string::const_iterator it = string.begin();
	     it != string.end();
	     ++it, ++i)
	{
		end = i;
		const char chr (*it);
		const bool lastChar = (it+1 == string.end());
		if (chr == escape && !escaped) {
			escaped = true;

			buff.append(string, begin, end-begin);
			begin = ++end;
		}
		else
		{

			bool isSeparator = (!escaped && chr == separator);
			if (isSeparator || lastChar)
			{
				if (lastChar && !isSeparator) { ++end; }
				buff.append(string, begin, end-begin);
				result.push_back(buff);
				fprintf(stderr, "add [%s]\n",buff.c_str());
				buff.clear();
				begin = ++end;
				if (lastChar && isSeparator) { result.push_back(std::string()); }
			}
			escaped = false;
		}
	}
	return result;
}

}

