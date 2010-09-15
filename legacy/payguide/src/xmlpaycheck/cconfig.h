// 
// File:   cconfig.h
// Author: alex
//
// Created on 17 Март 2008 г., 23:32
//

#ifndef _CCONFIG_H
#define	_CCONFIG_H

namespace paycheck
{
    class CConfigElement
    {
        public:
        CConfigElement();
        ~CConfigElement();
        char *section;
        char *param;
        char *value;
    };

    class CConfig
    {
        public:
            CConfig ();
            int ReadFile (const char *filename);
            int ParseText(const char *text);
            const char *GetValue (const char *aParam);
            const char *GetValue (const char *aParam, const char *Section);

            int SetValue (const char *aParam, const char *NewVal);
            int SetValue (const char *aParam, const char *Section, const char *NewVal);

            void Clear();
            ~CConfig();

        private:
            int ParseBuffer (const char *buffer, int size);
            std::vector <paycheck::CConfigElement *> node;
    };
}

#endif	/* _CCONFIG_H */

