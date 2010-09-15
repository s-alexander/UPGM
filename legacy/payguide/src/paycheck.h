#ifndef __Paycheck__
#define __Paycheck__

/*
* ���� �������:
*  0 - ������ ��������
*  1 - ������ ��������
*  2 - �������� ������/������ ��� ����� � ������������� ������
*  3 - ����� ���� "��".
*  4 - ������� ���� "cancel"
*  5 - ����!
* 10 -������� ����������
* 11 - ������ ����������� � ����
* 12 - ���������� �� ����
* 13 - ����� �� ������ ������ �������
* 14 - ����� �� ������ ������ �������
* 15 - ����� �� ������ ������ ������ ����������/engines
* 20 - ���������� ���������� (���)
* 21 - ���������� ���������� (���)
* 22 - ������ ������ �������� � ����� �� ������
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

