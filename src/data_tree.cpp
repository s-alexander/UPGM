#include <upgm/data_tree.hpp>
#include <stdexcept>

namespace PG
{

DataTree::DataTree()
{
}

DataTree::DataTree(const std::string & name, const DataTree & data)
{
	_resultMap[name] = data;
}

void DataTree::set(const std::string & name, const DataTree & data)
{
	_resultMap[name] = data;
}
void DataTree::set(const std::string & name, const std::string & value)
{
	_data[name] = value;
}

void DataTree::set(const Path & path, const std::string & value)
{
	if (path.size() == 1) {
		set(path[0], value);
	}
	else if (path.size() > 1) {
		set(Path(path.begin()+1, path.end()), value);
	}
}

const DataTree & DataTree::operator[](const std::string & name) const
{
	ResultMap::const_iterator it = _resultMap.find(name);
	if (it != _resultMap.end()) {
		return it->second;
	}
	throw std::runtime_error("No such node - " + name);
}
const std::string & DataTree::operator()(const std::string & name)
const {
	Variables::const_iterator it = _data.find(name);
	if (it != _data.end()) {
		return it->second;
	}
	throw std::runtime_error("No such var - " + name);
}

const std::string & DataTree::operator()(const Path::const_iterator & begin,
                               const Path::const_iterator & end) const
{
	if (end == begin) {
		throw std::runtime_error("Empty path");
	}
	else if (begin + 1 == end) {
		return operator()(*begin);
	}
	else {
		const DataTree & child = operator[](*begin);
		return child(begin+1, end);
	}
}

const std::string & DataTree::operator()(const Path & path) const
{
	return operator()(path.begin(), path.end());
}

//DataTree & DataTree::operator[](const std::string & name)
//{
//	return _resultMap[name];
//}
//
//std::string & DataTree::operator()(const std::string & name)
//{
//	return _data[name];
//}
//
//std::string & DataTree::operator()(const Path::const_iterator & begin,
//                               const Path::const_iterator & end)
//{
//	if (end == begin) {
//		throw std::runtime_error("Empty path");
//	}
//	else if (begin + 1 == end) {
//		return operator()(*begin);
//	}
//	else {
//		DataTree & child = operator[](*begin);
//		return child(begin+1, end);
//	}
//}
//
//std::string & DataTree::operator()(const Path & path)
//{
//	return operator()(path.begin(), path.end());
//}
}

