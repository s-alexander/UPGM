#ifndef __Operator__
#define __Operator__
#include "paysys.h"
#include "settings.h"
#include "sem.h"
#include "format.h"

class CPaySys;
struct SPay;

class COperator
{
	public:
	    /* Constructor */
	    COperator();
	    
	    /* Destructor */
	    ~COperator();
	    
	    /* Returns paysys for this operator,
	    currently ALWAYS returns default_paysys */
	    CPaySys *GetValidPaySys(SPay *p);
	    
	    pcre *GetWhiteFormat();
	    pcre *GetBlackFormat();
	    
	    /* Load operator from *.op file */
	    int Load(const char *file_name);
	    
	    /* Reread config file */
	    int ReloadConfig();
	    
	    /* Returns operator id, loaded from *.op file.
	    Must correspond with value from MySQL database. */
	    int GetOperatorId();
	    
	    CConfig *GetConfig();
	    CSemaphore lock;

	    int Use();
	    int UnUse(const char *engine_name);
	    
	private:
//	    CUlist<CPaySys> *valid_pay_sys;
	    pcre *wre;
	    pcre *bre;
	    int operator_id;
	    sem_t sem;
	    CConfig *op_config;
	    std::string filename;
	    int use_count;
	    const char *white_data_format;
	    const char *black_data_format;

};

/* Load operators (*.op files) from dir directory and populate CUlist */
CUlist<COperator> *LoadOperators(const char *dir);
int OperatorsInit();
int OperatorsShutdown();
#endif
