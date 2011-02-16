#include <upgm/config.hpp>
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <fstream>

namespace PG
{

Config::Config(): _inQuote(false)
{
}

Config::~Config() throw()
{
}

bool Config::isSkippable(char chr)
{
	const char skippable[] = { ' ', '\t', '\n', '\r' };
	const char * end = skippable+sizeof(skippable)/sizeof(skippable[0]);
	const bool result = std::find(skippable,
	                              end,
	                              chr) != end;
	return result;
}

void Config::flush()
{
	bool root = _section.empty();
	if (root) {
		_root.push_back( ConfigEntry ( _param, ConfigValue(_value, _inQuote)) );
	}
	else {
		_data[_section].push_back( ConfigEntry ( _param, ConfigValue(_value, _inQuote)) );
	}
	_param.clear();
	_value.clear();
	_inQuote = false;
}

void Config::parseFile(const std::string & filename)
{
	std::string buf;
	{
		std::ifstream file(filename.c_str());
		while (file.good()) {
			enum { bufferSize = 1024 };
			char buffer[bufferSize+1];

			file.read(buffer, bufferSize);
			buf.append(buffer, file.gcount());
		}
		file.close();
	}
	parse(buf);
}

void Config::resetTmp(std::string & buffer)
{
	buffer.clear();
}
void Config::saveTmp(std::string & buffer, const std::string & data)
{
	if (_blockStart > 0 && _blockEnd >= _blockStart) {
		buffer.append(data, _blockStart, _blockEnd-_blockStart);
	}
	_blockStart = -1;
	_blockEnd = -1;
}

void Config::appendTmp(std::string & buffer, const std::string & data)
{
	buffer.append(data, _blockStart, _blockEnd-_blockStart);
	_blockStart = _blockEnd;
}

void Config::parse(const std::string & data)
{
	bool escaped = false;
	const char equal = '=';
	const char linebreak = '\n';
	const char sectionStart = '[';
	const char sectionEnd   = ']';
	const char quote  = '\"';
	const char escape = '\\';
	const char commentMark = '#';
	_blockStart = _blockEnd = -1;
	size_t i (0);
	const char * endMark = NULL;
	enum ReadMode { UNDEF, SECTION, PARAM, VALUE, COMMENT };
	ReadMode readMode = UNDEF;
	bool skipOneChar = false;

	for (std::string::const_iterator it = data.begin();
	     it != data.end();
	     ++it, ++i)
	{
		const char chr = *it;

		switch (readMode)
		{
		case UNDEF:
		if (sectionStart == chr) { readMode = SECTION; skipOneChar = true; }
		else if  (commentMark == chr) { readMode= COMMENT; }
		else if (!isSkippable(chr) ) { readMode = PARAM; _blockStart = i; }
		break;

		case COMMENT:
		if (linebreak == chr) { readMode = UNDEF; _blockStart = -1; _blockEnd = -1; }
		break;

		case SECTION:
		if (sectionEnd == chr) { readMode = UNDEF; resetTmp(_section); saveTmp(_section, data); }
		break;

		case PARAM:
		if (equal == chr) { readMode = VALUE; endMark = NULL; saveTmp(_param, data); skipOneChar = true; }
		break;

		case VALUE:
		if (NULL == endMark && quote == chr && !escaped) { endMark = &quote; _inQuote = true; skipOneChar = true; }
		else if ( !isSkippable(chr) && NULL == endMark) { endMark = &linebreak; }
		else
		{
			const bool lastChar = (it + 1 == data.end());
			const bool isEndMark = (!escaped && NULL != endMark && *endMark == chr) || (NULL == endMark && linebreak == chr);
			if ( isEndMark || lastChar) {
				if (lastChar && !isEndMark) { ++ _blockEnd; }
				saveTmp(_value, data); flush(); readMode = UNDEF; _inQuote = false; endMark = NULL;
			}
		}
		break;
		}

		if (!skipOneChar && readMode != UNDEF && (!isSkippable(chr) || _inQuote )) {
			_blockEnd = i+1;
			if (-1 == _blockStart ) {
				_blockStart = i;
			}
		}
		skipOneChar = false;
		escaped = (!escaped && chr == escape);
		if (escaped)
		{
			--_blockEnd;
			switch (readMode)
			{
			case SECTION:
				appendTmp(_section, data);
			break;
			case PARAM:
				appendTmp(_param, data);
			break;
			case VALUE:
				appendTmp(_value, data);
			break;
			default:
			break;
			}
			_blockStart = ++_blockEnd;
		}
	}
}

/*const std::string & Config::getValue(const std::string & sectionName,
                                     const std::string & paramName) const
{
	const Section & sec = section(sectionName);
	Section::const_iterator param = sec.find(paramName);
	if (param != sec.end())
	{
		return param->second;
	}
	throw NoSuchValue(paramName);
}

const std::string & Config::getValue(const std::string & name) const
{
	Section::const_iterator param = _root.find(name);
	if (param != _root.end())
	{
		return param->second;
	}
	throw NoSuchValue(name);
}*/

const Config::Section & Config::section(const std::string & name) const
{
	CfgData::const_iterator it = _data.find(name);
	if (it != _data.end())
	{
		return it->second;
	}
	throw NoSuchValue(name);
}

}
