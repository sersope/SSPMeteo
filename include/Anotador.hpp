#ifndef __ANOTADOR__HPP__
#define __ANOTADOR__HPP__

#include <string>

class Anotador
{
    public:
        Anotador();
        Anotador(const std::string & fname);
        void setName(const std::string & fname);
        void anota(const std::string & texto,char t='l');
        std::string str_ahora(char t='l');
    private:
        std::string fname;
};

#endif // __ANOTADOR__HPP__
