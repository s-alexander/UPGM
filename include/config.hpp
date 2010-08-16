#ifndef PG_CONFIG
#define PG_CONFIG

#include <map>
#include <string>
#include <vector>
#include <stdexcept>

namespace PG
{

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
	typedef std::pair<std::string, std::string> ConfigEntry;
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
	size_t _blockStart;
	size_t _blockEnd;
	std::string _section;
	std::string _param;
	std::string _value;
};

}

#endif
