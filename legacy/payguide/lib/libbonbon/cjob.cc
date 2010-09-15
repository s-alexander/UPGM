#include "cjob.h"
#include <stdlib.h>

bonbon::CJob::CJob()
{
//    user_data=NULL;
    job_type=0;
}

bonbon::CJob::~CJob()
{
    
}

const bonbon::CJob &bonbon::CJob::operator=(const CJob &job)
{
    
    return *this;
}

int bonbon::CJob::MakeKiller()
{
    if (job_type==0)
    {
        job_type=CJOB_TYPE_KILLER;
        return 0;
    }
    return -1;
}

int bonbon::CJob::GetType()
{
    return job_type;
}
