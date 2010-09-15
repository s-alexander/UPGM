#ifndef __Uelement__
#define __Uelement__
template<class E> class CElement
{
	public:
	CElement(){}
	CElement(E* v){value=v;}
	~CElement()
	{
		if (value!=NULL)
		{
			delete value;
			value=NULL;
		}
		next=NULL;
		prev=NULL;
	}
	CElement<E> *next;
	CElement<E> *prev;
	E* value;
};
#endif

