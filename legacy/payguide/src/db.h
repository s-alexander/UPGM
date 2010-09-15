#ifndef __Db__
#define __Db__
#include "pay.h"
#include "settings.h"
#include <string>
using std::string;

/* This function calls only once at payguide startup */
int DBVeryFirstInit();

/* Start connection to MySQL server with specified host name, database name, user login and password */
int DBInit(const string *host, const string *db_name, const string *user, const string *password);

/* Returns next _new_ pay from DB  or NULL, if were is no new pays. You had to delete this pay by yourself.*/
SPay *DBGetNextPay();

/* Turn off all MySQL connections. */
int DBShutdown();

/* Set new state for pay with id. See result codes in pay.h  */
int DBSetState(SPay *pay, int result,int sleep, const char *msg, const char *sender_name);
int DBSetState(SPay *pay, SPayResult *payres);

/* Set new host name, database name, user login and password  */
void SetDBConnectionParam(const string *new_host, const string *new_db_name, const string *new_user, const string *new_password);

/* Set new reconnection timeout and attempts */
void DBSetReconnectTime(int new_time);
void DBSetReconnectAttempts(int new_value);

/* Turn MySQL connection off */
int DBConnectionDown();

/* Turn MySQL connection on */
int DBConnectionUp();

int AddOperatorToIgnoreList(int operator_id);
int AddEngineToIgnoreList(int engine_id);

int RemoveOperatorFromIgnoreList(int operator_id);
int RemoveEngineFromIgnoreList(int operator_id);

int ClearOperatorIgnoreList();
int ClearEngineIgnoreList();

int GetOperatorIgnoreList(char *buff, int buff_size, int *l);
int GetEngineIgnoreList(char *buff, int buff_size, int *l);

#endif

