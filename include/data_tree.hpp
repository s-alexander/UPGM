#ifndef PG_DATA_TREE
#define PG_DATA_TREE

#include <map>
#include <string>
#include <upgm/path.hpp>

namespace PG
{

class DataTree
{
public:
	DataTree();
	DataTree(const std::string & name, const DataTree & data);
	void set(const std::string & name, const DataTree & data);
	void set(const std::string & name, const std::string & value);
	const DataTree & operator[](const std::string & name) const;
	const std::string & operator()(const std::string & name) const;
	const std::string & operator()(const Path & path) const;

	DataTree & operator[](const std::string & name);
	std::string & operator()(const std::string & name);
	std::string & operator()(const Path & path);
private:
	const std::string & operator()(const Path::const_iterator & begin,
	                               const Path::const_iterator & end) const;
	std::string & operator()(const Path::const_iterator & begin,
	                         const Path::const_iterator & end);
	typedef std::map< std::string, DataTree > ResultMap;
	typedef std::map<std::string, std::string> Variables;

	ResultMap _resultMap;
	Variables _data;
};

}

#endif
