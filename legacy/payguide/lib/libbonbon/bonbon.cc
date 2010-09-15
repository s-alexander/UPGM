#include "bonbon.h"
int bonbon::BonbonInit()
{
    if (CLockInit()!=0)
        return -1;
    return 0;
}
