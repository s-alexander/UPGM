#ifndef __PerlM__
#define __PerlM__
#include "settings.h"


#ifdef PAYGUIDE_PERL
#include <EXTERN.h>
#include <perl.h>
#include <semaphore.h>

#endif

class CPerlModule
{
public:
	CPerlModule();
	~CPerlModule();
	int Init();
	int Clean();
	int Exec(char *buff, int buff_size, const char *agr, const char *sub_name);
protected:
#ifdef PAYGUIDE_PERL
	PerlInterpreter *my_perl;
	char *embedding[3];
	sem_t lock;
	int broken;
#endif
private:
	char *p1;
	char *p2;
	char *p3;

};

int PerlInit(const char *prg_name, const char *perl_file);
int PerlShutdown();

#endif
