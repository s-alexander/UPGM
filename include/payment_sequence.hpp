#ifndef PGPAYMENT_SEQUENCE
#define PGPAYMENT_SEQUENCE

#include <string>
#include <vector>
#include <stdexcept>
#include <cstdio>

namespace PG
{

class InvalidArgumentException: public std::runtime_error
{
public:
	InvalidArgumentException(): std::runtime_error("Invalid argument") { ;; }
};


class PaymentSequence
{
public:
	PaymentSequence(): _stage(0), _stageName("main") { ;; }
	size_t currentStage() const { return _stage; }
	void nextStage() { ++_stage; }
	const std::string & stageName(size_t stage) const {
		const StageNames::const_iterator it = _stageNames.find(stage);
		if (it != _stageNames.end()) { return it->second; }
		fprintf(stderr, "stage %i not found\n", stage);
		throw InvalidArgumentException();
	}
	const std::string & stageName() const { return _stageName; }
	void addStage(size_t stage, const std::string & stageName) {
		fprintf(stderr, "stage %li added as %s\n", stage, stageName.c_str());
		_stageNames[stage] = stageName;
		_stageId[stageName] = stage;
	}
	void setStage(const std::string & stageName) {
		fprintf(stderr, "stage set to %s\n", stageName.c_str());
		const StageId::const_iterator it = _stageId.find(stageName);
		if (it != _stageId.end()) { _stage = it->second; }
		_stageName = stageName;
	}
private:
	size_t _stage;
	std::string _stageName;
	typedef std::map< size_t, std::string > StageNames;
	typedef std::map< std::string, size_t > StageId;
	StageNames _stageNames;
	StageId _stageId;
};


}

#endif
