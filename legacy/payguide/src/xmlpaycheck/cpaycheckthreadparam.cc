#include "cpaycheckthreadparam.h"

paycheck::CPaycheckThreadParam::CPaycheckThreadParam(bonbon::CJobManager *in_m, std::vector <paycheck::CSocket *> *connections_s, CPaycheckCore *paycheck_c):bonbon::CThreadParam()
{
    in_manager=in_m;
    connections_store=connections_s;
    paycheck_core=paycheck_c;
}

paycheck::CPaycheckThreadParam::~CPaycheckThreadParam()
{

}
