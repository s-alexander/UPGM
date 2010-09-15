#ifndef __Ulist__
#define __Ulist__
#include "uelement.h"
#include <stdlib.h>
template<class E> class CUlist
{

public:
    CUlist() {//Конструктор
    len=0;
    zero=new CElement<E>(); zero->value=NULL;
    last=new CElement<E>(); last->value=NULL;
    zero->next=last; zero->prev=NULL;
    last->prev=zero; last->next=NULL;
    } 
    
    ~CUlist() //Деструктор
    {
	    cursor=zero;
	    while (cursor!=NULL)
	    {
		cursor=cursor->next;
		if (cursor && cursor->prev)
		{
			delete cursor->prev;
			cursor->prev=NULL;
		}
		else
		{
			delete last;
			last=NULL;
		}
	    }
	    
    }

    void AddItem(E* e);
    void RemoveItem(int id);
    void RemoveThis();

    void ClearToNull();
    E* GetNext();
    E* GetById(int id);
    unsigned int GetLen();

    void ResetCursor();
private:
    int len;
    CElement<E> *zero;
    CElement<E> *cursor;
    CElement<E> *last;
};

template<class E> void CUlist<E>::AddItem(E* e)
{
    if (e==NULL) return;
    CElement<E> *t=new CElement<E>(e);
    t->next=last;
    t->prev=last->prev;
    last->prev->next=t;
    last->prev=t;
    len++;
}

template<class E> void CUlist<E>::RemoveItem(int id)
{
    using namespace std;
    if (id>=len || id <0) return;
    CElement<E> *t=zero->next;
    
    for (int i=0; i<len;i++)
    {
    if (i==id)
    {
if (cursor==t)
{
cursor=t->prev;
}

    if (t->next!=NULL)
    {
	if (t->prev!=NULL)t->next->prev=t->prev; else t->next->prev=NULL;
    }
    
    if (t->prev!=NULL)
    {
	if (t->next!=NULL)t->prev->next=t->next; else t->prev->next=NULL;
    }
    t->next=NULL;
    t->prev=NULL;
    if (t!=NULL){delete t;t=NULL;}
    len--;
    return;
    }
    t=t->next;
}
}
    
template<class E> E* CUlist<E>::GetNext()
{
    E* rtrn;
    if (!cursor)
    {
	    printf("CURSOR=NULL\n");
	    return NULL;
    }
    if (cursor->next) cursor=cursor->next;
    rtrn=cursor->value;
    return rtrn;
}

template<class E> void CUlist<E>::ResetCursor()
{  
    cursor=zero;
}

template<class E> unsigned int CUlist<E>::GetLen()
{
    return abs(len);
}

template<class E> E* CUlist<E>::GetById(int id)
{
  CElement<E> *t=zero->next;
  for (int i=0; i<len;i++)
    {
    if (i==id)
    {
    return t->value; 
    }
    t=t->next;
    if (t==NULL) return NULL;
}
return NULL;
}

template<class E> void CUlist<E>::ClearToNull()
{
  CElement<E> *t=zero->next;
  for (int i=0; i<len;i++)
    {
    t->value=NULL; 
    t=t->next;
}
}

template<class E> void CUlist<E>::RemoveThis()
{
//cursor=cursor->prev;
if (cursor==NULL) return;
CElement<E> *t=cursor->prev;
if (t==NULL)
{
	printf("t is null\n");
	return;
}
if (cursor->next==NULL) return;
if (cursor->prev==NULL) return;

cursor->next->prev=cursor->prev;
cursor->prev->next=cursor->next;

    cursor->next=NULL;
    cursor->prev=NULL;
    delete cursor;cursor=NULL;
    len--;
    cursor=t;
    return;
}


#endif

