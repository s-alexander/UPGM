#include "/home/alex/proj/payguide/payguide_production/src/pay.h"
#include "/home/alex/proj/payguide/payguide_production/src/core.h"
#include "/home/alex/proj/UPGM/include/upgm.hpp"
#include "/home/alex/proj/UPGM/modules/centrtelecom/include/centrtelecom.hpp"
#include "/home/alex/proj/UPGM/include/shared_mysql_connection.hpp"
#include "/home/alex/proj/UPGM/include/http_transport.hpp"
#include "/home/alex/proj/UPGM/include/xml_parser.hpp"
#include "/home/alex/proj/UPGM/include/payment.hpp"
#include "/home/alex/proj/UPGM/include/db_mysql.hpp"

typedef PG::UPGM ModuleClass;
int main(int argc, char ** argv)
{
	return 0;
}

struct GlobalContext
{
	PG::SharedMysqlConnection mysqlConnection;
};

struct Context
{
	ModuleClass module;
	std::string scheme;
	std::string config;
	std::string codes;
	std::string logpath;
};

extern "C" void *ThreadStart()
{
	return NULL;
}

extern "C" void ThreadEnd(void *thread_init)
{
	return;
}

extern "C" void *Load(CPaySys *paysys)
{
	GlobalContext * globalContext = new GlobalContext();
	if (paysys!=NULL)
	{
		if (paysys->GetConfig()!=NULL)
		{
		}
	}
	return globalContext;
}

extern "C" void UnLoad(CPaySys *paysys, void *first_init_param)
{
	/* Runs when *.so UNloads */
	delete (GlobalContext*)first_init_param;
}

extern "C" SPayResult SendData(SPay *data, void *init_param, void *first_init_param)
{
	GlobalContext * globalContext = (GlobalContext *)first_init_param;
	Context * context = (Context *) init_param;

	PG::Payment payment(*data);

	PG::Config scheme;
	scheme.parseFile(context->scheme.c_str());

	PG::Config config;
	config.parseFile(context->config.c_str());

	PG::Config codes;
	codes.parseFile(context->codes.c_str());

	ModuleClass & module = context->module;
	module.setScheme(scheme);
	module.setCodes(codes);
	module.setConfig(config);
	module.setLog(context->logpath);

	module.performStage(&globalContext->mysqlConnection, payment);

	return payment.asSPayResult();
}


extern "C" void Call(void *first_init_param)
{
	return;
}

extern "C" void *PayInitExt(CPaySys *paysys, COperator *operat, SPay *pay, int worker_id, void *first_init_param)
{

	const char *paysys_codename=NULL;
	if (pay!=NULL)
		paysys_codename=pay->paysys_codename;

	Context *context= new Context();

	if (paysys!=NULL)
	{
		/* Load values from PAY SYSTEM config */
		if (paysys->GetConfig()!=NULL)
		{
			if (paysys->GetConfig()->GetValue("scheme", paysys_codename)!=NULL)
				context->scheme=paysys->GetConfig()->GetValue("scheme", paysys_codename);

			if (paysys->GetConfig()->GetValue("config", paysys_codename)!=NULL)
				context->config=paysys->GetConfig()->GetValue("config", paysys_codename);

			if (paysys->GetConfig()->GetValue("codes", paysys_codename)!=NULL)
				context->codes=paysys->GetConfig()->GetValue("codes", paysys_codename);

			if (paysys->GetConfig()->GetValue("log", paysys_codename)!=NULL)
				context->logpath=paysys->GetConfig()->GetValue("log", paysys_codename);
		}
	}
	return context;
}


extern "C" void CleanUp(void *param)
{
	delete (Context *)param;
}

