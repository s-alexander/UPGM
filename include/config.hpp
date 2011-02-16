#ifndef PG_CONFIG
#define PG_CONFIG

#include <map>
#include <string>
#include <vector>
#include <stdexcept>

namespace PG
{

class ConfigValue {
public:
	ConfigValue(const std::string value, bool inQuote):
		_value(value), _inQuote(inQuote) { ;; }
	const std::string & value() const { return _value; }
	bool inQuote() const { return _inQuote; }
	const std::string & operator*() const { return _value; }
private:
	std::string _value;
	bool _inQuote;
};

class Config
{
public:
	class NoSuchValue: public std::runtime_error
	{
	public:
		NoSuchValue(const std::string & value): std::runtime_error("No such value - " + value),
			_value(value) { ;; }
		~NoSuchValue() throw() { ;; }
		const std::string & valueName() const { return _value; }
	private:
		std::string _value;
	};
	typedef std::pair<std::string, ConfigValue> ConfigEntry;
	typedef std::vector< ConfigEntry > Section;
	Config();
	~Config() throw();
	void parse(const std::string & data);
	void parseFile(const std::string & filename);
//	const std::string & getValue(const std::string & section,
//	                     const std::string & name) const;
//	const std::string & getValue(const std::string & name) const;
	const Section & section(const std::string & name) const;
	static bool isSkippable(char chr);
private:
	void flush();
	void resetTmp(std::string & buffer);
	void saveTmp(std::string & buffer, const std::string & data);
	void appendTmp(std::string & buffer, const std::string & data);

	typedef std::map<std::string, Section> CfgData;

	Section _root;
	CfgData _data;
	long long _blockStart;
	long long _blockEnd;
	bool _inQuote;
	std::string _section;
	std::string _param;
	std::string _value;
};

}

#endif
