#include <vector>
#include <iostream>
#include <cstring>
#include "parser.h"
//using namespace std;

CConfigElement::CConfigElement()
{
	section=NULL;
	param=NULL;
	value=NULL;
}

CConfigElement::~CConfigElement()
{

	if (section!=NULL)
		delete [] section;
	if (param!=NULL)
		delete [] param;
	if (value!=NULL)
		delete [] value;

	section=NULL;
	param=NULL;
	value=NULL;
}


CConfig::CConfig ()
{

}

int CConfig::ReadFile(const char *filename)
{
	Clear ();
	FILE *f = fopen (filename, "rb");
	if (f == NULL)
	{
		return -1;
	}
	fseek (f, 0L, SEEK_END);
	long fsize = ftell (f);
	fseek (f, 0L, SEEK_SET);
	if (fsize < 0 || fsize > 100 * 1024) // max size == 100kb
	{
		fclose (f);
		return -1;
	}
	char *buffer = new char[fsize];
	if (buffer == NULL)
	{
		fclose (f);
		return -1;
	}

	int nread = fread (buffer, 1, fsize, f);
	if (nread <= 0)
	{
		delete [] buffer;
		fclose (f);
		return -1;
	}

	fclose (f);
	if (ParseBuffer (buffer, nread) != 0)
	{
		delete [] buffer;
		return -1;
	}
		delete [] buffer;
	return 0;
}

const char *CConfig::GetValue (const char *aParam)
{
	return GetValue (aParam, NULL);
}

int CConfig::SetValue (const char *aParam, const char *new_val)
{
	return SetValue (aParam, NULL, new_val);
}

const char *CConfig::GetValue (const char *aParam, const char *Section)
{
	unsigned int idx = 0;
	unsigned int sz = node.size();
	for (; idx < sz; idx++)
	{
		if (Section!=NULL && node[idx]->section!=NULL)
		{
			if (!strcmp(aParam, node[idx]->param) && !strcmp(Section, node[idx]->section))
			{
				return node[idx]->value;
			}
		}
		else
		{
			if (Section!=NULL && node[idx]->section==NULL)
				continue;

			if (Section==NULL && node[idx]->section!=NULL)
				continue;

			if (!strcmp(aParam, node[idx]->param))
			{
				return node[idx]->value;
			}

		}
	}
	return NULL;
}

int CConfig::SetValue (const char *aParam, const char *Section, const char *new_value)
{
	if (new_value==NULL)
		return -1;

	unsigned int idx = 0;
	unsigned int sz = node.size();
	for (; idx < sz; idx++)
	{
		if (Section!=NULL && node[idx]->section!=NULL)
		{
			if (!strcmp(aParam, node[idx]->param) && !strcmp(Section, node[idx]->section))
			{
				if (node[idx]->value!=NULL)
					delete [] node[idx]->value;
				node[idx]->value=new char[strlen(new_value)+2];

				if (node[idx]->value==NULL)
					return -1;

				strncpy(node[idx]->value, new_value, strlen(new_value)+1);

				return 0;
			}
		}
		else
		{
			if (Section!=NULL && node[idx]->section==NULL)
				continue;

			if (Section==NULL && node[idx]->section!=NULL)
				continue;

			if (!strcmp(aParam, node[idx]->param))
			{
				if (node[idx]->value!=NULL)
					delete [] node[idx]->value;
				node[idx]->value=new char[strlen(new_value)+2];

				if (node[idx]->value==NULL)
					return -1;

				strncpy(node[idx]->value, new_value, strlen(new_value)+1);

				return 0;

			}

		}
	}
	return -1;
}



void CConfig::Clear ()
{
	vector <CConfigElement *>::iterator it;
	for (it = node.begin(); it != node.end(); it++)
	if (*it != NULL)
		delete *it;

	node.clear();
}

CConfig::~CConfig ()
{
		Clear ();
}

int CConfig::ParseBuffer (char *buffer, int size)
{
bool new_line=true;
bool section_mode=false;
bool comment_mode=false;

if (buffer==NULL) return 1;
	if (size < 0)
		return -1;

	char *ptr = buffer;
	char *paramstart = buffer;
	unsigned int paramsize = 0;
	char *valuestart = NULL;
	char *sectionstart=buffer;
	unsigned int valuesize = 0;
	unsigned int sectionsize = 0;

	while (ptr - buffer <= size)
	{
		if (ptr - buffer == size || *ptr == '\n' || (section_mode && *ptr==']'))
		{
			if (ptr - buffer < size)
			{

				if (ptr - buffer < size && *ptr == ']')
				{
					if (section_mode)
						sectionsize=ptr-sectionstart;
					paramstart=ptr+2;
				}

				if (*ptr == '\n' )
				{
					new_line=true;
					section_mode=false;
					comment_mode=false;
				}
				else
					new_line=false;

				if (paramsize == 0 || comment_mode)
				{
					ptr++;
					paramstart=ptr;
					paramsize=0;
					continue;
				}

			}

			valuesize = ptr - valuestart;
			if (valuestart > paramstart)
			{

				char *param, *value, *section=NULL;

				param = new char [paramsize + 1];
				if (param == NULL)
					return -1;

				value = new char [valuesize + 1];
				if (value == NULL)
				{
					delete [] param;
					return -1;
				}

				if (sectionsize!=0)
				{
					while(*sectionstart=='\n')
					{
						sectionstart++;
						sectionsize--;
					}

					section= new char [sectionsize+1];
					if (section==NULL)
					{
						delete [] param;
						delete [] value;
						return -1;
					}
					strncpy (section, sectionstart, sectionsize);
					section[sectionsize] = 0;
				}
				else
				{
					section=NULL;
				}

				while(*paramstart=='\n')
				{
					paramstart++;
					paramsize--;
				}


				strncpy (param, paramstart, paramsize);
				param[paramsize] = 0;
				if (section!=NULL)
				{
					if (strcmp(section, "end")==0)
					{
						delete [] section;
						section=NULL;
					}
				}


				strncpy (value, valuestart, valuesize);
				value[valuesize] = 0;




				CConfigElement *new_elmnt=new CConfigElement();

				new_elmnt->value=value;
				new_elmnt->param=param;
				new_elmnt->section=section;


				/*
				if (section!=NULL)
					printf("param=[%s], value=[%s], section=[%s]\n",param, value, section);
				else
					printf("param=[%s], value=[%s], section=NULL\n",param, value);
				*/

				node.push_back (new_elmnt);
			}

			paramsize = 0;
			valuesize = 0;
			paramstart = ptr + 1;
			valuestart = ptr + 1;
		}
		else if (*ptr == '=' && paramsize==0 && valuesize==0)
		{
			paramsize = ptr - paramstart;
			valuestart = ptr + 1;
		}
		else if (*ptr == '#' && new_line)
		{
			comment_mode=true;
		}
		else if (*ptr == '[')
		{
			section_mode=true;
			sectionstart=ptr+1;
		}



		ptr++;
	}

	return 0;
	}
