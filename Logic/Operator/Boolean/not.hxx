#ifndef NOT_HXX
#define NOT_HXX

template<Proposition SubPropertyType>
Not<SubPropertyType>::Not(const ptr<SubPropertyType> &son):
    Operator<NotFun<SubPropertyType>>(NOT,son)
{
    static_assert (std::is_same<SubPropertyType,ASubTheorem>::value||std::is_same<SubPropertyType,ASubRule>::value,
                   "SubPropertyType in Not operator must be a subTheorem or a subRule" );
}

template<Proposition SubPropertyType>
const ptr<SubPropertyType>& Not<SubPropertyType>::operator[](const size_t &k) const
{
    return this->m_fun[k];
}

template<Proposition SubPropertyType> template<Proposition SubPropertyType2>
bool Not<SubPropertyType>::operator==(const Not<SubPropertyType2> &ope) const
{
    return this->m_fun==ope.m_fun;
}

template<Proposition SubPropertyType>
bool NotFun<SubPropertyType>::operator()() const
{
    return !m_sonProp->testEvaluate();
}

template<Proposition SubPropertyType>
std::string NotFun<SubPropertyType>::symbol()
{
    return NotToStr::s_symbol;
}

template<Proposition SubPropertyType>
std::string NotFun<SubPropertyType>::toString(unsigned short priorityParent) const
{
    if(priorityParent<priority())
    {
        return "("+symbol()+m_sonProp->toString(priority())+")";
    }
    else
    {
        return symbol()+m_sonProp->toString(priority());
    }
}

template<Proposition SubPropertyType>
constexpr unsigned short NotFun<SubPropertyType>::priority()
{
    return 3;
}

template<Proposition SubPropertyType> template<Proposition SubPropertyType2>
bool NotFun<SubPropertyType>::operator==(const NotFun<SubPropertyType2> &ope) const
{
    return *m_sonProp==*(ope.m_sonProp);
}

template<Proposition SubPropertyType>
const ptr<SubPropertyType>& NotFun<SubPropertyType>::operator[](const size_t &k) const
{
    switch(k)
    {
        case 0:
        {
            return m_sonProp;
        }
        default:
        {
            throw std::runtime_error("Not operator has only one arguments");
        }
    }
}


template<Proposition SubPropertyType>
NotFun<SubPropertyType>::NotFun(const ptr<SubPropertyType> &prop1): m_sonProp(prop1)
{

}

template<Proposition SubPropertyType>
const ptr<SubPropertyType>&
get(const NotFun<SubPropertyType>& ope)
{
    return ope.m_sonProp;
}

template<Proposition SubPropertyType>
const ptr<SubPropertyType>&
get(const Not<SubPropertyType>& ope)
{
    return get(ope.m_fun);
}

#endif
