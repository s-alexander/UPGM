#include <upgm/payment.hpp>
#include <upgm/data_tree.hpp>
#include <upgm/request_template.hpp>
#include <upgm/common_hooks.hpp>
#include <upgm/upgm.hpp>

#include <fstream>
#include <sstream>
#include <limits>
#include <cstdlib>

namespace PG
{


UPGM::UPGM()
{
}

UPGM::~UPGM() throw()
{
	Hooks::const_iterator it = _hooks.begin();
	const Hooks::const_iterator end = _hooks.end();
	for (; end != it; ++it) { delete it->second; }

}

void UPGM::registerHook(HookPtr  hook)
{
	const char * hookName = hook->name();
	const Hooks::iterator it = _hooks.find(hookName);
	const Hooks::const_iterator end = _hooks.end();
	if (end != it)
	{
		delete it->second;
		it->second = hook.release();
	}
	else
	{
		_hooks.insert(std::make_pair(hookName, hook.release()));
	}
}

std::string UPGM::evaluateConfigValue(const std::string & value)
{
	const char variableMark = '$';
	if (value.length() > 0)
	{
		if (value[0] == variableMark)
		{
			const std::string pathStr(value.begin() + 1, value.end());
			const Path path ( pathFromString( pathStr ));
			if (path.size() > 0)
			{
				const std::string & hookName = path[0];
				Hooks::const_iterator it = _hooks.find(hookName);
				if (it != _hooks.end())
				{
					return it->second->read(Path(path.begin()+1, path.end()));
				}
				throw HookUndefException(hookName);
			}
		}
	}
	return value;
}

void UPGM::evaluateConfigParam(const std::string & param, const std::string & value)
{
	const char variableMark = '$';
	if (param.length() > 0)
	{
		std::string::const_iterator begin = param.begin();
		const std::string::const_iterator end = param.end();
		if (param[0] == variableMark) {
			++begin;
		}
		const std::string pathStr(begin, end);
		const Path path ( pathFromString( pathStr ));
		if (path.size() > 0)
		{
			const std::string & hookName = path[0];
			Hooks::const_iterator it = _hooks.find(hookName);
			if (it != _hooks.end())
			{
				it->second->write(Path(path.begin()+1, path.end()), value);
				return;
			}
			throw HookUndefException(hookName);
		}
	}
	throw NotAVariableException(value);
}

void UPGM::evalParams(const std::string & sectionName)
{
	const Config::Section & section = _scheme.section(sectionName);
	for (Config::Section::const_iterator it = section.begin();
	     it != section.end();
	     ++it) {
		try {
			evaluateConfigParam(it->first, evaluateConfigValue(it->second));
		} catch (const NotAVariableException & e) { ;; }
	}
}

void      UPGM::performStage(int stage,
                            Transport & transport,
                            Parser & parser,
                            Payment & payment)
{
	registerHook(HookPtr(new StageHook(&_sequence)));
	registerHook(HookPtr(new PrintHook()));
	registerHook(HookPtr(new CodeHook()));

	registerHook(HookPtr(new TransportHook(&transport)));
	registerHook(HookPtr(new ParserHook(&parser)));
	registerHook(HookPtr(new RequestHook()));
	registerHook(HookPtr(new PaymentHook(&payment)));

	registerUserHooks(transport, parser, payment);

	do
	{
		evalParams(_sequence.stageName());
	}
	while ( payment.stateUndef() );

}

void UPGM::setScheme(const Config & requestScheme)
{
	_scheme = requestScheme;
}

}
