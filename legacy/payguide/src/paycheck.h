#ifndef __Paycheck__
#define __Paycheck__

/*
* Типы пакетов:
*  0 - ваучер проверка
*  1 - платеж проверка
*  2 - отправка логина/пароля или ответ о недостаточных правах
*  3 - ответ типа "ок".
*  4 - ответип типа "cancel"
*  5 - пинг!
* 10 -команда управления
* 11 - запрос подключения к логу
* 12 - отключение от лога
* 13 - ответ на запрос списка потоков
* 14 - ответ на запрос списка модулей
* 15 - ответ на запрос списка черных операторов/engines
* 20 - отладочная информация (лог)
* 21 - отладочная информация (лог)
* 22 - запрос списка клиентов и ответ на запрос
*
*/

/* Authorization levels  */
#define AUTH_NONE 0
#define AUTH_STAT 1
#define AUTH_PAYCHECK 2
#define AUTH_CONTROL 3
#define AUTH_ADMIN 4

#include "pay.h"
#include "settings.h"
#include <string>
using std::string;

int PCInit(int port, int m_queue, const char *interface, const char *users_filename, int packagetimeout);
int PCShutdown();
SPay *PCGetNextPay();
int PutResulToPaycheck(const char *formatted_xml);
//int PCSetState(SPay *pay, int result,int sleep, const char *msg, const char *sender_name, SPayResult *payres);
int PCSetState(SPay *pay, SPayResult *payres);
SPay *CompileCheckPay(char *data, int size);
void SendLogMessages(int priority, const char *message);
#endif

