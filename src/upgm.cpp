#include <upgm/payment.hpp>
#include <upgm/data_tree.hpp>
#include <upgm/request_template.hpp>
#include <upgm/upgm.hpp>

#include <fstream>
#include <sstream>
#include <limits>

namespace PG
{


UPGM::UPGM()
{
	registerHook( "pay"       , &UPGM::paymentHookRead   );
	registerHook( "code"      , &UPGM::codeHookRead      );
	registerHook( "answer"    , &UPGM::answerHookRead    );
	registerHook( "transport" , &UPGM::transportHookRead );
	registerHook( "db"        , &UPGM::dbHookRead        );

	registerHook( "db"       , &UPGM::dbHookWrite   );
}

UPGM::~UPGM() throw()
{
}

static UPGM::RequestResult asRequestResult(const std::string & str)
{
	if (str == "next_stage" ) { return UPGM::NEXT_STAGE; }
	if (str == "complete"   ) { return UPGM::COMPLETED;  }
	if (str == "fail"       ) { return UPGM::FAIL;       }
	if (str == "sleep"      ) { return UPGM::SLEEP;      }
	throw std::runtime_error("No such payment stage - " + str);
}

std::string evaluateStringFromTemplate(const RequestTemplate & request)
{
	//TODO implement
	return std::string("TEST");
}

void UPGM::registerHook(const std::string & name, HookRead hook)
{
	_hooksRead[name] = hook;
}

void UPGM::registerHook(const std::string & name, HookWrite hook)
{
	//_hooksRead[name] = hook;
	_hooksWrite[name] = hook;
}

static std::string asString(UPGM::RequestResult result)
{

	switch (result)
	{
		case UPGM::UNDEF      : return "UNDEF";
		case UPGM::NEXT_STAGE : return "NEXT_STAGE";
		case UPGM::COMPLETED  : return "COMPLETED";
		case UPGM::FAIL       : return "FAIL";
		case UPGM::SLEEP      : return "SLEEP";
	}
	throw std::runtime_error("RequestResult value undef");
}

static std::string asString(int i)
{
	enum { size = std::numeric_limits<int>::digits };
	char buff[size+1];
	snprintf(buff, size, "%i", i);
	return buff;
}

const std::string & UPGM::evaluateConfigValue(const std::string & value)
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
				HooksRead::const_iterator it = _hooksRead.find(hookName);
				if (it != _hooksRead.end())
				{
					HookRead hook = it->second;
					return (this->*hook)(Path(path.begin()+1, path.end()));
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
	if (value.length() > 0)
	{
		if (value[0] == variableMark)
		{
			const std::string pathStr(value.begin() + 1, value.end());
			const Path path ( pathFromString( pathStr ));
			if (path.size() > 0)
			{
				const std::string & hookName = path[0];
				HooksWrite::const_iterator it = _hooksWrite.find(hookName);
				if (it != _hooksWrite.end())
				{
					HookWrite hook = it->second;
					(this->*hook)(Path(path.begin()+1, path.end()), value);
					return;
				}
				throw HookUndefException(hookName); 
			}
		}
	}	
	throw NotAVariableException(value);
}

void UPGM::populate(DataTree & tree, const Config::Section & sec)
{
	
	for (Config::Section::const_iterator it = sec.begin();
	     it != sec.end();
	     ++it) {
		tree.set(it->first, evaluateConfigValue(it->second));
	}
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

void UPGM::request(const std::string & actionName,
                   Transport & transport,
                   Parser & parser)
{
	try {
		const std::string request = evaluateConfigValue(_scheme.getValue(actionName, "request"));
		// Clear variables
		_answer    = DataTree();
		_code      = DataTree();
		_transport = DataTree();

		RequestTemplate requestTemplate(request);
		RequestTemplate::Variables vars = requestTemplate.variables();
		RequestTemplate::VariablesMap varsMap;
		for (RequestTemplate::Variables::const_iterator it = vars.begin();
		     it != vars.end();
		     ++it) {
			varsMap[*it] = evaluateConfigValue(_scheme.getValue(actionName, *it));
		}

		// Sending request
		transport << requestTemplate.evaluate(varsMap);

		// Reading answer
		std::string answer;

		_transport = transport >> answer;

		// Parsing answer
		_answer = parser.parse(answer);
	}
	catch (const Config::NoSuchValue & e) { fprintf(stderr, "request undef (%s)\n", e.what()); }

}

void      UPGM::performStage(int stage,
                            Transport & transport,
                            Parser & parser,
                            const Payment & payment)
{
	RequestResult result(UNDEF);

	int currentStage (stage);
	std::string actionName = _scheme.getValue("stages", asString(currentStage));

	bool noError(true);

	do
	{
		
		noError = false;
		result = UNDEF;
		fprintf(stderr, "Performing action %s\n", actionName.c_str());


		 _payment = payment.generateDataTree();
		
		// Try to perform request
		request(actionName, transport, parser);

		// Try to access answer code
		try {
			const std::string code = evaluateConfigValue(_scheme.getValue(actionName, "code"));
			
			DataTree codeData;
			try {
				populate(codeData,_codes.section(code));
			}
			catch ( ... ) {
				populate(codeData,_codes.section("default"));
			}
			
			_code = codeData;
		}
		catch (const Config::NoSuchValue & e) { fprintf(stderr, "code undef (%s)\n", e.what()); }
		fprintf(stderr, "evaluating params for %s\n", actionName.c_str());

		// Evaluate parameters
		evalParams(actionName);

		// Get next action or result
		try {
			result = asRequestResult(evaluateConfigValue(_scheme.getValue(actionName, "result")));
			evalParams(actionName);
		}
		catch (const Config::NoSuchValue & e)
		{
			try {
				actionName = evaluateConfigValue(_scheme.getValue(actionName, "next"));
				fprintf(stderr, "next action = %s\n", actionName.c_str());
				noError = true;
			}
			catch (const Config::NoSuchValue & e)
			{
				throw std::runtime_error("no 'next' or 'result' specified. I don't know what to do now. ");
			}
		}

		if (result == NEXT_STAGE)
		{	
			++currentStage;
			actionName = evaluateConfigValue(_scheme.getValue("stages", asString(currentStage)));
			noError = true;
		}
	} while ( noError );

	fprintf(stderr, "---FINAL---\nRESULT  : [%s]\n", asString(result).c_str());
	fprintf(stderr, "MESSAGE : [%s]\n", _code("message").c_str());
}


std::string UPGM::customVariableWithName(const std::string & name)
{
	throw VariableUndefException(name); 
}

void UPGM::setScheme(const Config & requestScheme)
{
	_scheme = requestScheme;
	_codes.parseFile(_scheme.getValue("stages", "codes"));
}

const std::string & UPGM::paymentHookRead(const Path & path)
{
	return _payment(path);
}
const std::string & UPGM::codeHookRead(const Path & path)
{
	return _code(path);
}
const std::string & UPGM::answerHookRead(const Path & path)
{
	return _answer(path);
}

const std::string & UPGM::transportHookRead(const Path & path)
{
	return _transport(path);
}

const std::string & UPGM::dbHookRead(const Path & path)
{
	return _database(path);
}

void UPGM::dbHookWrite(const Path & path, const std::string & value)
{
	_database(path) = value;
}

}
