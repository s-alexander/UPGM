#include <upgm/template.hpp>
#include <cstdio>

namespace PG
{

void Template::saveVar(const std::string & varName, size_t position)
{
	_variables.push_back(varName);
	_location[position] = varName;
}

void Template::parse(const std::string & request)
{
	_location.clear();
	bool escaped = false;
	const char varMark = '$';
	const char varStart = '(';
	const char varEnd   = ')';
	const char escape = '\\';
	size_t i (0);
	enum ReadMode { DATA, VAR };
	ReadMode readMode = DATA;
	size_t varBegin(0);

	std::string buff;

	size_t blockStart = 0, blockEnd = 0;
	for (std::string::const_iterator it = request.begin();
	     it != request.end();
	     ++it, ++i)
	{
		const char chr = *it;
		const bool lastChr = (it+1 == request.end());

		switch (readMode)
		{
			case DATA:
			if ((chr == varMark && !escaped) || lastChr)
			{
				readMode = VAR;
				if (request.begin() != it) {
					buff.append(request, blockStart, blockEnd-blockStart+(lastChr?1:0)+1);
				}
				blockStart = blockEnd;
				varBegin = buff.length();
			}
			break;
			case VAR:
			if (chr == varStart)
			{
				blockStart = i+1;
			}
			else if (chr == varEnd || lastChr)
			{
				saveVar(std::string(request, blockStart, blockEnd-blockStart+1), varBegin);
				readMode = DATA;
				blockStart = i+1;
			}
			break;
		}

		escaped = (!escaped && chr == escape);

		if (escaped && readMode == DATA) {
			buff.append(request, blockStart, blockEnd-blockStart+1);
			blockStart = i+1;
		}
		blockEnd = i;
	}
	_request = buff;
}

void Template::setTemplate(const std::string & tmpl)
{
	parse(tmpl);
}

Template::Template()
{
}
Template::Template(const std::string & request)
{
	parse(request);
}

std::string Template::evaluate(const VariablesMap & data)
{
	std::string result (_request);
	size_t offset = 0;
	for (ParamLocation::const_iterator it = _location.begin();
	     it != _location.end();
	     ++it) {
		const std::string & var = it->second;
		const size_t position = it->first;
		const VariablesMap::const_iterator valueIt = data.find(var);
		if (data.end() != valueIt)
		{
			const std::string & value = valueIt->second;
			result.insert(offset+position, value);
			offset += value.length();
		}
	}
	return result;
}

void Template::set(const std::string & name, const std::string & value) {
	_data[name] = value;
}

std::string Template::evaluate() {
	const std::string result (this->evaluate(_data));
	_data.clear();
	return result;
}


}
