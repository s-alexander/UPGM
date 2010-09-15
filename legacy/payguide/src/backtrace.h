#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <execinfo.h>

// descriptor of a file for writing backtrace_symbols
int bt_fd = -1;

// open file for backtrace data
void open_backtrace_fd (const char *fname)
{
    //char fname[] = "backtrace.log";
    if (bt_fd!=-1)
	    close(bt_fd);
    bt_fd=-1;
    unlink (fname);
    bt_fd = open (fname, O_CREAT | O_WRONLY | O_APPEND, 0666);
    if (bt_fd != -1)
    	write (bt_fd, "\n", 1);
}

void signal_handler (int signo) 
{
    // signal_handler variables (made static to keep stack as is)
    static void *stack[100];
    static char **functions;
    static int	count, i;

    if (bt_fd == -1)
    {
	printf("SIGNAL: %d\n", signo);
	count = backtrace(stack, 100);
	functions = backtrace_symbols (stack, count);

	for (i=0; i<count; i++) 
	{
	    printf("Point %3d: %s\n", i, functions[i]);
	}
	free(functions);
    }
    else
    {
	count = backtrace(stack, 100);
	backtrace_symbols_fd (stack, count, bt_fd);
    	close (bt_fd);
	bt_fd = -1;
    }
    
    exit(0);
}

// installing signal handlers to save backtrace info
void install_handlers() 
{
    signal(SIGBUS, signal_handler);
    signal(SIGILL, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
//    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);	// -9
}

void onexit ()
{
    if (bt_fd != -1)
        close (bt_fd); // Close backtrace file if opened
}

