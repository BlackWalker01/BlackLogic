#include "term.h"
#include "Boolean/boolean.h"
#include "Boolean/constboolean.h"
#include "allvariable.h"
#include <sstream>

using namespace N_Logic;

std::shared_ptr<AbstractTerm> AbstractTerm::create(const std::string &name)
{
    auto it=s_constantHash.find(name);
    if(it!=s_constantHash.end())
    {
        switch (it->second)
        {
            case FALSE_TYPE:
            {
                return std::make_shared<ConstBoolean>(false);
            }
            case TRUE_TYPE:
            {
                return std::make_shared<ConstBoolean>(true);
            }
            case BOOLEAN_SET_TYPE:
            {
                return std::make_shared<ConstSet<BooleanSet>>(BooleanSet());
            }
            case NATURALINTEGERSET_TYPE:
            {
                return std::make_shared<ConstSet<NaturalIntegerSet>>(NaturalIntegerSet());
            }
        }
    }
    else
    {
        bool isConst=true;
        //bool isReal=false;
        for(size_t k=0;k<name.size();k++)
        {
            char c=name[k];
            if(c<'0' || c>'9')
            {
                isConst=false;
                break;
            }
            if(c=='.')
            {
                //isReal=true;
            }
        }

        if(isConst)
        {
            std::stringstream ss;
            ss<<name;
            size_t val=0;
            ss>>val;
            return std::make_shared<ConstNaturalInteger>(val);
        }
        else
        {
            return std::make_shared<Boolean>(name);
        }
    }
    return nullptr;
}

AbstractTerm::AbstractTerm(const std::string &name): m_name(name)
{

}


AbstractTerm::AbstractTerm(const AbstractTerm &term): m_name(term.m_name)
{

}


std::string AbstractTerm::name() const
{
    return m_name;
}


std::string AbstractTerm::toString(unsigned short) const
{
    return m_name;
}

