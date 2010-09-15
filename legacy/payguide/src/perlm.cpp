#include "perlm.h"
#include <cstring>


#ifdef PAYGUIDE_PERL
static char **argv=NULL;
static int argn;
#endif
static int ConvertData(char *buff, int buff_size, const char *old_data, int old_data_size, const char *sub_name);
CPerlModule::CPerlModule()
{

#ifdef PAYGUIDE_PERL
	my_perl=NULL;
	broken=1;

	p1=new char[2];
	strncpy(p1,"",1);

	p2=new char[4];
	strncpy(p1,"-e",3);

	p3=new char[3];
	strncpy(p1,"0",2);

	embedding[0]=p1;
	embedding[1]=p2;
	embedding[2]=p3;

	sem_init(&lock, 0,1);
#endif


}

CPerlModule::~CPerlModule()
{
#ifdef PAYGUIDE_PERL

	if (my_perl!=NULL)
	{
		perl_destruct(my_perl);
		perl_free(my_perl);
		my_perl=NULL;
	}
	delete [] p1;
	delete [] p2;
	delete [] p3;
#endif
}


int CPerlModule::Exec(char *buff, int buff_size, const char *arg, const char *sub_name)
{
#ifdef PAYGUIDE_PERL
	if (broken!=0)
		return -2;
	//char *args[] = { NULL };
	int val=0;
	sem_wait(&lock);


	//call_argv(sub_name, G_DISCARD | G_NOARGS, args);
	val=ConvertData(buff, buff_size, arg, strlen(arg), sub_name);
	sem_post(&lock);
	return val;
#else
	return 0;
#endif
}

int CPerlModule::Init()
{
#ifdef PAYGUIDE_PERL
	my_perl = perl_alloc();
	perl_construct(my_perl);
	PL_perl_destruct_level=1;
	broken=perl_parse(my_perl, NULL, argn, argv, NULL);
	return broken;
#else
	return 0;
#endif
}

int CPerlModule::Clean()
{
#ifdef PAYGUIDE_PERL
	if (my_perl!=NULL)
	{
		perl_destruct(my_perl);
		perl_free(my_perl);
		my_perl=NULL;
	}
#endif
	return 0;
}


int PerlInit(const char *prg_name, const char *perl_file)
{
#ifdef PAYGUIDE_PERL
	argv=new char *[2];
	argv[0]=new char[strlen(prg_name)+1]; strncpy(argv[0], prg_name, strlen(prg_name)+1);
	argv[1]=new char[strlen(perl_file)+1]; strncpy(argv[1], perl_file, strlen(perl_file)+1);
	//int argn=1;
	PERL_SYS_INIT3(&argn,&argv,NULL);
	PL_perl_destruct_level=1;
#endif
	return 0;
}

int PerlShutdown()
{
#ifdef PAYGUIDE_PERL

	if (argv!=NULL)
	{
		if (argv[0]!=NULL)
			delete [] argv[0];
		if (argv[1]!=NULL)
			delete [] argv[1];
		delete [] argv;
	}
	argv=NULL;
	PERL_SYS_TERM();
#endif
	return 0;
}

int ConvertData(char *buff, int buff_size, const char *old_data, int old_data_size, const char *sub_name)
{
	int result=0;
#ifdef PAYGUIDE_PERL
	dSP;                            /* initialize stack pointer      */
	ENTER;                          /* everything created after here */
	SAVETMPS;                       /* ...is a temporary variable.   */
	PUSHMARK(SP);                   /* remember the stack pointer    */
	XPUSHs(sv_2mortal(newSVpv(old_data, old_data_size))); /* push the exponent onto stack  */
	PUTBACK;                      /* make local stack pointer global */
	int count=call_pv(sub_name, G_SCALAR | G_EVAL);      /* call the function             */

	if (count!=1)
	{
		result=-2;
	}
	else
	{
		SPAGAIN;                       /* refresh stack pointer         */
		char *tmp=POPpx;/* pop the return value from stack */
		if (tmp!=NULL && strcmp(tmp, "")!=0)
			strncpy(buff, tmp, buff_size);
		else
			result=-1;

		PUTBACK;
	}
		FREETMPS;                       /* free that return value        */
		LEAVE;                       /* ...and the XPUSHed "mortal" args.*/
#endif
		return result;
}
