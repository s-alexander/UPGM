#include "format.h"
#include <stdio.h>
#include <string.h>


int ValidData(pcre *format, const char *data)
{
	
	if (format==NULL || data==NULL)
	{
		return 2;
	}
	
	int rc = pcre_exec(format, NULL, data, strlen(data), 0,0,NULL,0);
	if (rc<0)
		rc=0;
	else
		rc=1;
	
	
	return rc;
	
}
