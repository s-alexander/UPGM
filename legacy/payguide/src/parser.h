#ifndef __Parser__
#define __Parser__

#include <vector>
#include <iostream>
#include "settings.h"
using namespace std;

class CConfigElement
{
public:
	CConfigElement();
	~CConfigElement();
	char *section;
	char *param;
	char *value;
	
};

class CConfig
{
	public:
	    CConfig ();
	    int ReadFile (const char *filename);
	    const char *GetValue (const char *aParam);
	    const char *GetValue (const char *aParam, const char *Section);
	    
	    int SetValue (const char *aParam, const char *NewVal);
	    int SetValue (const char *aParam, const char *Section, const char *NewVal);
	    
	    void Clear ();
	    ~CConfig ();
	    
	private:
	    int ParseBuffer (char *buffer, int size);
	    vector <CConfigElement *> node;
};
/*
Usage:
cfg.ReadFile ("my.conf"); -- reading from file `my.conf` (returns 0 on success)
cfg.GetValue("mysqlhost"); -- getting value of `mysqlhost` (NULL if not defined)
*/
#endif

