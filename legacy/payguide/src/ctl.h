#ifndef __Ctl__
#define __Ctl__
#include <string>
#include "uremote_socket.h"

#include "commands.h"

/* Reload main config from file filename */
void ReloadConfigIfImportant();
int ReloadConfigValues(char *filename);

/* Reload paysys modules (*.so files) */
int ReloadPaySys();

/* Reload operators (*.op files) */
int ReloadOperators();

/* ReConfigure operators (*.op files) */
int ReConfOperators();

/* ReConfigure modules (or "paysys") (*.so files) */
int ReConfPaysys();

/* Get statistic string and threads list */
std::string *GetThreadsList();
std::string *GetModulesList();

int GetThreadsBin(char *buff, int buff_size);
int GetModulesBin(char *buff, int buff_size);
int ExecCmd(int cmd, const char *arg, unsigned int arg_size, int *result_size, CSocketRW *sock);
#endif

