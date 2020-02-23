#include "subrule.h"
#include "theorem.h"
#include "Formula/Arithmetic/allarithformula.h"
#include "allproposition.h"

using namespace N_Logic;

ptr<ValueTypeObject> N_Logic::createSubRule(const std::string &name, std::vector<OperatorOrdering> &opeList,
                                            std::vector<OperatorOrdering> &orderedOpeList,
                                                std::vector<std::shared_ptr<VariableContainer>> &varList)
{
    if(orderedOpeList.size())
    {        
        auto ope=orderedOpeList[0].ope;
        if(ope->name()==HYP)
        {
            size_t nbPar=orderedOpeList[0].nbPar;
            orderedOpeList.erase(orderedOpeList.begin());
            opeList.erase(opeList.begin());
            std::vector<OperatorOrdering> topOpeList, topOrderedOpeList, queueOpeList, queueOrderedOpeList;
            std::vector<ptr<IISubRuleFormula>> subProps;
            for(size_t k=0;k<ope->arity();k++)
            {
                if(orderedOpeList.size()>0)
                {
                    if((orderedOpeList[0].argIndex==k && orderedOpeList[0].nbPar==nbPar+1) ||
                            (orderedOpeList[0].nbPar==nbPar && k==ope->arity()-1)) //implication of hypothesis operator
                    {
                        auto crtOpe=orderedOpeList[0].ope;
                        for(unsigned int i=0; i<opeList.size();i++)
                        {
                            if(crtOpe==opeList[i].ope)
                            {
                                //if crtOpe is the implication of hypothesis operator
                                if(orderedOpeList[0].nbPar==nbPar && k==ope->arity()-1)
                                {
                                    topOpeList=opeList;
                                }
                                else
                                {
                                    topOpeList.insert(topOpeList.begin(),opeList.begin(),opeList.begin()+i+1);
                                    queueOpeList.insert(queueOpeList.begin(),opeList.begin()+i+1,opeList.end());
                                }
                                break;
                            }
                        }

                        //store remaining operators to the left of current operator
                        std::unordered_map<ptr<IOperator>,OperatorOrdering> hashLeftOpe;
                        for(unsigned int i=0;i<topOpeList.size();i++)
                        {
                            auto ope=&(topOpeList[i]);
                            hashLeftOpe[ope->ope]=*ope;
                        }

                        //store remaining operators to the right of current operator
                        std::unordered_map<ptr<IOperator>,OperatorOrdering> hashRightOpe;
                        for(unsigned int i=0;i<queueOpeList.size();i++)
                        {
                            auto ope=&(queueOpeList[i]);
                            hashRightOpe[ope->ope]=*ope;
                        }

                        //separate operators in queueOrderedOpeList to the left and to the right and order them in each list
                        for(unsigned int i=0;i<orderedOpeList.size();i++)
                        {
                            auto ope=&(orderedOpeList[i]);
                            auto itLeft=hashLeftOpe.find(ope->ope);
                            if(itLeft!=hashLeftOpe.end())
                            {
                                topOrderedOpeList.push_back(*ope);
                                continue;
                            }
                            auto itRight=hashRightOpe.find(ope->ope);
                            if(itRight!=hashRightOpe.end())
                            {
                                queueOrderedOpeList.push_back(*ope);
                                continue;
                            }
                        }

                        orderedOpeList=queueOrderedOpeList;
                        opeList=queueOpeList;
                    }
                }

                //top sub-SubTheorem
                subProps.push_back(std::dynamic_pointer_cast<const IISubRuleFormula>
                                   (createSubRule(name+ope->symbol()+sizeToString(k),topOpeList,topOrderedOpeList,varList)));
                topOrderedOpeList.clear();
                queueOrderedOpeList.clear();
            }

            switch (ope->name())
            {
                case HYP:
                {
                    ptr<ASubRule> subPropCast=nullptr;
                    std::vector<ptr<ASubRule>> subPropCasts;
                    for(auto subProp: subProps)
                    {
                        if((subPropCast=std::dynamic_pointer_cast<const ASubRule>(subProp)))
                        {
                            subPropCasts.push_back(subPropCast);
                        }
                        else
                        {
                            throw std::runtime_error("Expected SubRule in hypothesis operator");
                        }
                    }
                    return std::make_shared<const SubRule<Hyp<ASubRule>>>(name+"HYP",subPropCasts);
                }
                default:
                {
                    throw std::runtime_error("Empty SubRule");
                }
            }
        }
        else
        {
            switch(ope->arity())
            {
                case 1:
                {
                    std::vector<OperatorOrdering> queueOpeList(opeList.begin()+1,opeList.end());
                    std::vector<OperatorOrdering> queueOrderedOpeList(orderedOpeList.begin()+1,orderedOpeList.end());
                    switch(ope->name())
                    {
                        case NOT:
                        {
                            auto subOpe=createSubRule(name+"!",queueOpeList,queueOrderedOpeList,varList);
                            switch (subOpe->valueType())
                            {
                                case BOOL_TYPE:
                                {
                                    auto subOpeCast=std::dynamic_pointer_cast<const ASubRule>(subOpe);
                                    return std::make_shared<const SubRule<Not<ASubRule>>>(name,subOpeCast);
                                }
                                default:
                                {
                                    throw std::runtime_error("Expected boolean variable just after not operator");
                                }
                            }
                        }
                        default:
                        {
                            throw std::runtime_error("Only operators of arity one in this part of formula");
                        }
                    }
                    break;
                }
                case 2:
                {
                    std::vector<OperatorOrdering> queueOrderedOpeList(orderedOpeList.begin()+1,orderedOpeList.end());
                    std::vector<OperatorOrdering> leftOpeList;
                    std::vector<OperatorOrdering> rightOpeList;
                    std::vector<OperatorOrdering> leftOrderedOpeList;
                    std::vector<OperatorOrdering> rightOrderedOpeList;
                    if(ope->arity()==2)
                    {
                        for(unsigned int k=0; k<opeList.size();k++)
                        {
                            if(ope==(opeList[k].ope))
                            {
                                leftOpeList.insert(leftOpeList.begin(),opeList.begin(),opeList.begin()+k);
                                rightOpeList.insert(rightOpeList.begin(),opeList.begin()+k+1,opeList.end());
                                break;
                            }
                        }

                        //store remaining operators to the left of current operator
                        std::unordered_map<ptr<IOperator>,OperatorOrdering> hashLeftOpe;
                        for(unsigned int i=0;i<leftOpeList.size();i++)
                        {
                            auto crtOpe=&(leftOpeList[i]);
                            hashLeftOpe[crtOpe->ope]=*crtOpe;
                        }

                        //store remaining operators to the right of current operator
                        std::unordered_map<ptr<IOperator>,OperatorOrdering> hashRightOpe;
                        for(unsigned int i=0;i<rightOpeList.size();i++)
                        {
                            auto crtOpe=&(rightOpeList[i]);
                            hashRightOpe[crtOpe->ope]=*crtOpe;
                        }

                        //separate operators in queueOrderedOpeList to the left and to the right and order them in each list
                        for(unsigned int i=0;i<queueOrderedOpeList.size();i++)
                        {
                            auto crtOpe=&(queueOrderedOpeList[i]);
                            auto itLeft=hashLeftOpe.find(crtOpe->ope);
                            if(itLeft!=hashLeftOpe.end())
                            {
                                leftOrderedOpeList.push_back(*crtOpe);
                                continue;
                            }
                            auto itRight=hashRightOpe.find(crtOpe->ope);
                            if(itRight!=hashRightOpe.end())
                            {
                                rightOrderedOpeList.push_back(*crtOpe);
                                continue;
                            }
                        }
                    }

                    switch(ope->name())
                    {
                        case AND:
                        {
                            auto leftOpe=createSubRule(name+"&&L",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubRule(name+"&&R",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case BOOL_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case BOOL_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubRule>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubRule>(rightOpe);
                                            return std::make_shared<const SubRule<And<ASubRule>>>
                                            (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("And operator expects boolean variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("And operator expects boolean variable");
                                }
                            }
                        }
                        case EQUIVALENT:
                        {
                            auto leftOpe=createSubRule(name+"<=>L",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubRule(name+"<=>R",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case BOOL_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case BOOL_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubRule>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubRule>(rightOpe);
                                            return std::make_shared<const SubRule<Equivalent<ASubRule>>>
                                            (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("Equivalent operator does not handle bool variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("Equivalent operator does not handle bool variable");
                                }
                            }
                        }
                        case IMPLICATION:
                        {
                            auto leftOpe=createSubRule(name+"=>L",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubRule(name+"=>R",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case BOOL_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case BOOL_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubRule>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubRule>(rightOpe);
                                            return std::make_shared<const SubRule<Implication<ASubRule >>>
                                            (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("Implication operator expects boolean variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("Implication operator expects boolean variable");
                                }
                            }
                        }
                        case OR:
                        {
                            auto leftOpe=createSubRule(name+"||L",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubRule(name+"||R",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case BOOL_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case BOOL_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubRule>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubRule>(rightOpe);
                                            return std::make_shared<const SubRule<Or<ASubRule >>>
                                            (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("Or operator expects boolean variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("Or operator expects boolean variable");
                                }
                            }
                        }
                        case SETEQUAL:
                        {
                            auto firstVar=varList[0];
                            varList.erase(varList.begin());
                            auto rightOpe=createSubRule(name+"=R",rightOpeList,rightOrderedOpeList,varList);
                            varList.insert(varList.begin(),firstVar);
                            if(firstVar->type==VALUE_TYPE::VOID_TYPE)
                            {
                                firstVar->type=rightOpe->valueType();
                            }
                            else if(firstVar->type!=rightOpe->valueType())
                            {
                                throw std::runtime_error(firstVar->var->name()+" is not of the same type of right operand");
                            }
                            auto leftOpe=createSubRule(name+"=L",leftOpeList,leftOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case NATURAL_INT_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case NATURAL_INT_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const SubArithmeticRule<NaturalInteger>>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubArithmeticRule<size_t>>(rightOpe);
                                            return std::make_shared<const SubArithmeticRule<SetEqual<SubArithmeticRule<NaturalInteger> >>>(name,
                                                                                                                leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("SetEqual operator does not handle bool variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("SetEqual operator does not handle bool variable");
                                }
                            }
                        }
                        case EQUAL:
                        {
                            auto leftOpe=createSubRule(name+"==L",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubRule(name+"==R",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case NATURAL_INT_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case NATURAL_INT_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubArithmeticRule<size_t>>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubArithmeticRule<size_t>>(rightOpe);
                                            return std::make_shared<const SubRule<Equal<ASubArithmeticRule<size_t>, ASubArithmeticRule<size_t> >>>
                                                                    (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("Equal operator does not handle bool variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("Equal operator does not handle bool variable");
                                }
                            }
                        }
                        case BELONGSTO:
                        {
                            if(varList.size()<2)
                            {
                                throw std::runtime_error("€ operator expects 2 arguments but got only "+sizeToString(varList.size()));
                            }
                            std::shared_ptr<VariableContainer> firstVar=nullptr;
                            ptr<ValueTypeObject> leftOpe=nullptr;
                            ptr<ValueTypeObject> rightOpe=nullptr;
                            if(leftOpeList.size()==0)
                            {
                                firstVar=varList[0];
                                varList.erase(varList.begin());
                                rightOpe=createSubRule(name+"€R",rightOpeList,rightOrderedOpeList,varList);
                                varList.insert(varList.begin(),firstVar);
                                if(firstVar->type==VALUE_TYPE::VOID_TYPE)
                                {
                                    switch (rightOpe->valueType())
                                    {
                                        case NATURAL_INTEGERSET_TYPE:
                                        {
                                            firstVar->type=NATURAL_INT_TYPE;
                                            break;
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("BelongsTo operator expects set as right operand");
                                        }
                                    }
                                    firstVar->init();
                                }
                                else if(firstVar->type!=rightOpe->valueType())
                                {
                                    throw std::runtime_error(firstVar->var->name()+" is not of the same type of right operand in € operator");
                                }
                                leftOpe=createSubRule(name+"€L",leftOpeList,leftOrderedOpeList,varList);
                            }
                            else
                            {
                                leftOpe=createSubRule(name+"€L",leftOpeList,leftOrderedOpeList,varList);
                                rightOpe=createSubRule(name+"€R",rightOpeList,rightOrderedOpeList,varList);
                            }
                            switch(leftOpe->valueType())
                            {
                                case NATURAL_INT_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case NATURAL_INTEGERSET_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubArithmeticRule<size_t>>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubArithmeticRule<NaturalIntegerSet>>(rightOpe);
                                            return std::make_shared<const SubRule<BelongsTo<ASubArithmeticRule<size_t>, ASubArithmeticRule<NaturalIntegerSet> >>>
                                                    (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("BelongsTo operator does not handle bool variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("BelongsTo operator does not handle bool variable");
                                }
                            }
                        }
                        case LET:
                        {
                            auto leftOpe=createSubRule(name+"letL",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubRule(name+"letR",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case VOID_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case BOOL_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubArithmeticRule<void>>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubRule>(rightOpe);
                                            return std::make_shared<const SubRule<Let<ASubArithmeticRule<void>,ASubRule>>>
                                                                                            (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("Let operator handles only boolean as right operand");
                                        }
                                    }
                                }
                                case BOOL_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case BOOL_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubRule>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubRule>(rightOpe);
                                            return std::make_shared<const SubRule<Let<ASubRule,ASubRule>>>
                                                                                (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("Let operator handles only boolean as right operand");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("Let operator does not handle numbers as left operand");
                                }
                            }
                        }
                        case PLUS:
                        {
                            auto leftOpe=createSubRule(name+"+L",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubRule(name+"+R",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case NATURAL_INT_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case NATURAL_INT_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubArithmeticRule<size_t>>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubArithmeticRule<size_t>>(rightOpe);
                                            return std::make_shared<const SubArithmeticRule<Plus<ASubArithmeticRule<size_t>,
                                                    ASubArithmeticRule<size_t> >>>(name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("Plus operator does not handle bool variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("Plus operator does not handle bool variable");
                                }
                            }
                        }
                        default:
                        {
                            throw std::runtime_error("Do not handle boolean operators in arithmetic formula");
                        }
                    }
                }

                default:
                {
                    throw std::runtime_error("Empty SubRule");
                }
            }

        }
    }
    else if(varList.size())
    {
        std::shared_ptr<AbstractTerm> var=varList[0]->var;
        varList.erase(varList.begin());
        if(var->isConstant())
        {
            switch(var->valueType())
            {
                case VALUE_TYPE::BOOL_TYPE:
                {
                    return std::make_shared<const SubRule<ConstBoolean>>(name,
                                            std::dynamic_pointer_cast<ConstBoolean>(var)->evaluate());
                }
                case VALUE_TYPE::NATURAL_INT_TYPE:
                {
                    return std::make_shared<const ConstSubArithmeticRule<ConstNaturalInteger>>(name,
                                            std::dynamic_pointer_cast<ConstNaturalInteger>(var)->evaluate());
                }
                case VALUE_TYPE::NATURAL_INTEGERSET_TYPE:
                {
                    return std::make_shared<const ConstSubArithmeticTheorem<ConstSet<NaturalIntegerSet>>>(name,
                                                std::dynamic_pointer_cast<ConstSet<NaturalIntegerSet>>(var)->evaluate());
                }
                case VALUE_TYPE::BOOLEANSET_TYPE:
                {
                    return std::make_shared<const ConstSubArithmeticTheorem<ConstSet<BooleanSet>>>(name,
                                                std::dynamic_pointer_cast<ConstSet<BooleanSet>>(var)->evaluate());
                }
                default:
                {
                    throw std::runtime_error("No bool variable in arithmetic formula");
                }
            }
        }
        else
        {
            switch(var->valueType())
            {
                case VALUE_TYPE::BOOL_TYPE:
                {
                    return std::make_shared<const SubRule<Boolean>>(name,
                                            std::dynamic_pointer_cast<Boolean>(var));
                }
                case VALUE_TYPE::NATURAL_INT_TYPE:
                {
                    return std::make_shared<const SubArithmeticRule<NaturalInteger>>(name,
                                        std::dynamic_pointer_cast<NaturalInteger>(var));
                }

                case VALUE_TYPE::NATURAL_INTEGERSET_TYPE:
                {
                    return std::make_shared<const SubArithmeticRule<VarSet<NaturalIntegerSet>>>(name,
                                                std::dynamic_pointer_cast<VarSet<NaturalIntegerSet>>(var));
                }
                case VALUE_TYPE::BOOLEANSET_TYPE:
                {
                    return std::make_shared<const SubArithmeticRule<VarSet<BooleanSet>>>(name,
                                                std::dynamic_pointer_cast<VarSet<BooleanSet>>(var));
                }
                default:
                {
                    throw std::runtime_error("No bool variable in arithmetic formula");
                }
            }
        }
    }
    else
    {
        throw std::runtime_error("Empty SubRule");
    }
    return nullptr;
}


/**---------------------------------------------------------------
 * Constructor methods from subProperties
 * ---------------------------------------------------------------
 */

template<>
SubRule<And<ASubRule>>::SubRule(const std::string& name_, const ptr<ASubRule>& leftSubProp,
                                         const ptr<ASubRule>& rightSubProp):
    ASubPureRule(name_,leftSubProp->getExtVars(),rightSubProp->getExtVars(),AND_PROP),
    m_son(std::make_unique<And<ASubRule>>(leftSubProp,rightSubProp))
{

}

template<>
SubRule<Equivalent<ASubRule>>::SubRule(const std::string& name_, const ptr<ASubRule>& leftSubProp,
                                                const ptr<ASubRule>& rightSubProp):
    ASubPureRule(name_,leftSubProp->getExtVars(),rightSubProp->getExtVars(),EQUIV_PROP),
    m_son(std::make_unique<Equivalent<ASubRule>>(leftSubProp,rightSubProp))
{

}

SubRule<Hyp<ASubRule>>::SubRule(const std::string& name_, const std::vector<ptr<ASubRule>>& subProps):
    ASubPureRule(name_,getDbVarFromRules(subProps),HYP_PROP), m_son(std::make_unique<Hyp<ASubRule>>(subProps))
{

}

template<>
SubRule<Implication<ASubRule>>::SubRule(const std::string& name_, const ptr<ASubRule>& leftSubProp,
                                                 const ptr<ASubRule>& rightSubProp):
    ASubPureRule(name_,leftSubProp->getExtVars(),rightSubProp->getExtVars(),IMPL_PROP),
    m_son(std::make_unique<Implication<ASubRule>>(leftSubProp,rightSubProp))
{

}

SubRule<Not<ASubRule>>::SubRule(const std::string& name_, const ptr<ASubRule>& subProp):
    ASubPureRule(name_,subProp->getExtVars(),NOT_PROP), m_son(std::make_unique<Not<ASubRule>>(subProp))
{

}

template<>
SubRule<Or<ASubRule>>::SubRule(const std::string& name_, const ptr<ASubRule> &leftSubProp, const ptr<ASubRule> &rightSubProp):
    ASubPureRule(name_,leftSubProp->getExtVars(),rightSubProp->getExtVars(),OR_PROP),
    m_son(std::make_unique<Or<ASubRule>>(leftSubProp,rightSubProp))
{

}

SubRule<Boolean>::SubRule(const std::string& name_, const std::shared_ptr<Boolean>& son):
    ASubRule(name_,DbVar(son),VAR_PROP), m_son(son)
{

}

SubRule<ConstBoolean>::SubRule(const std::string& name_, const bool& val):
    ASubRule(name_,CONST_PROP), m_son(std::make_unique<ConstBoolean>(val))
{

}


/**---------------------------------------------------------------
 * Evaluate methods
 * ---------------------------------------------------------------
 */

template<typename SubPropertyType>
bool SubRule<SubPropertyType>::evaluate() const
{
    return m_son->evaluate();
}

bool SubRule<Hyp<ASubRule>>::evaluate() const
{
    return m_son->evaluate();
}

bool SubRule<Not<ASubRule>>::evaluate() const
{
    return m_son->evaluate();
}

bool SubRule<Boolean>::evaluate() const
{
    return m_son->evaluate();
}

bool SubRule<ConstBoolean>::evaluate() const
{
    return m_son->evaluate();
}


/**---------------------------------------------------------------
 * toString methods
 * ---------------------------------------------------------------
 */
template<typename SubPropertyType>
std::string SubRule<SubPropertyType>::toString(unsigned short priorityParent) const
{
    return m_son->toString(priorityParent);
}

std::string SubRule<Hyp<ASubRule>>::toString(unsigned short priorityParent) const
{
    return m_son->toString(priorityParent);
}

std::string SubRule<Not<ASubRule>>::toString(unsigned short priorityParent) const
{
    return m_son->toString(priorityParent);
}

std::string SubRule<Boolean>::toString(unsigned short priorityParent) const
{
    return m_son->toString(priorityParent);
}

std::string SubRule<ConstBoolean>::toString(unsigned short priorityParent) const
{
    return m_son->toString(priorityParent);
}


/**---------------------------------------------------------------
 * getSon methods
 * ---------------------------------------------------------------
 */
template<typename SubPropertyType>
const SubPropertyType &SubRule<SubPropertyType>::getSon() const
{
    return *(m_son.get());
}

const Hyp<ASubRule>& SubRule<Hyp<ASubRule>>::getSon() const
{
    return *(m_son.get());
}

const Not<ASubRule>& SubRule<Not<ASubRule>>::getSon() const
{
    return *(m_son.get());
}

const Boolean& SubRule<Boolean>::getSon() const
{
    return *(m_son.get());
}

const ConstBoolean& SubRule<ConstBoolean>::getSon() const
{
    return *(m_son.get());
}

/**---------------------------------------------------------------
 * operator[] methods
 * ---------------------------------------------------------------
 */
template<typename SubPropertyType>
const ptr<ASubRule>& SubRule<SubPropertyType>::operator[](const size_t& index) const
{
    return (*m_son)[index];
}

const ptr<ASubRule>& SubRule<Hyp<ASubRule>>::operator[](const size_t& index) const
{
    return (*m_son)[index];
}

const ptr<ASubRule>& SubRule<Not<ASubRule>>::operator[](const size_t&) const
{
    return (*m_son)[0];
}

/**---------------------------------------------------------------
 * arity methods
 * ---------------------------------------------------------------
 */
template<typename SubPropertyType>
size_t SubRule<SubPropertyType>::arity() const
{
    return m_son->arity();
}

size_t SubRule<Not<ASubRule>>::arity() const
{
    return m_son->arity();
}

size_t SubRule<Hyp<ASubRule>>::arity() const
{
    return m_son->arity();
}

size_t SubRule<Boolean>::arity() const
{
    return 0;
}

size_t SubRule<ConstBoolean>::arity() const
{
    return 0;
}


/**---------------------------------------------------------------
 * identifyPriv methods
 * ---------------------------------------------------------------
 */
template<>
bool SubRule<And<ASubRule>>::identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const
{
    if(prop->type()==PropType::AND_PROP)
    {
        auto propCast=std::dynamic_pointer_cast<const SubTheorem<And<ASubTheorem>>>(prop);
        return (*m_son)[0]->identifyPriv(propCast->getSon()[0],dbVarProp) && (*m_son)[1]->identifyPriv(propCast->getSon()[1],dbVarProp);
    }
    return false;
}

template<>
bool SubRule<Equivalent<ASubRule>>::identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const
{
    if(prop->type()==PropType::EQUIV_PROP)
    {
        auto propCast=std::dynamic_pointer_cast<const SubTheorem<Equivalent<ASubTheorem>>>(prop);
        return (*m_son)[0]->identifyPriv(propCast->getSon()[0],dbVarProp) && (*m_son)[1]->identifyPriv(propCast->getSon()[1],dbVarProp);
    }
    return false;
}

template<>
bool SubRule<Implication<ASubRule>>::identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const
{
    if(prop->type()==PropType::IMPL_PROP)
    {
        auto propCast=std::dynamic_pointer_cast<const SubTheorem<Implication<ASubTheorem>>>(prop);
        return (*m_son)[0]->identifyPriv(propCast->getSon()[0],dbVarProp) && (*m_son)[1]->identifyPriv(propCast->getSon()[1],dbVarProp);
    }
    return false;
}

bool SubRule<Hyp<ASubRule>>::identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const
{
    if(prop->type()==PropType::HYP_PROP)
    {
        auto propCast=std::dynamic_pointer_cast<const SubTheorem<Hyp<ASubTheorem>>>(prop);
        if(m_son->arity()>=propCast->arity()-1)
        {
            bool ret=false;
            //Identify all properties before HYP variable and conclusion
            for(size_t k=0;k<m_son->arity()-2;k++)
            {
                ret=(*m_son)[k]->identifyPriv(propCast->getSon()[k],dbVarProp);
                if(!ret)
                {
                    return false;
                }
            }
            if(ret)
            {
                std::string nameHypVar=(*m_son)[m_son->arity()-2]->toString();
                size_t indexHyp=m_son->arity()-2;
                if(dbVarProp.containsHyp(nameHypVar))
                {
                    //check if old mapping is the same as the current one
                    auto props=dbVarProp.getHypAssoc(nameHypVar);
                    for(size_t k=indexHyp;k<propCast->arity()-1;k++)
                    {
                        ret=*(props[k-indexHyp])==*(propCast->getSon()[k]);
                        if(!ret)
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    //map HYP variable with prop subProperties
                    for(size_t k=indexHyp;k<propCast->arity()-1;k++)
                    {
                        dbVarProp.insertHypAssoc(nameHypVar,propCast->getSon()[k]);
                    }
                }

                ret=(*m_son)[m_son->arity()-1]->identifyPriv(propCast->getSon()[propCast->arity()-1],dbVarProp);
            }
            return ret;
        }
    }
    else if(m_son->arity()==2)
    {
        std::string nameHypVar=(*m_son)[0]->toString();
        dbVarProp.insertHypEmpty(nameHypVar);
        return (*m_son)[1]->identifyPriv(prop,dbVarProp);
    }
    return false;
}

bool SubRule<Not<ASubRule>>::identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const
{
    if(prop->type()==PropType::NOT_PROP)
    {
        auto propCast=std::dynamic_pointer_cast<const SubTheorem<Not<ASubTheorem>>>(prop);
        return (*m_son)[0]->identifyPriv(propCast->getSon()[0],dbVarProp);
    }
    return false;
}

template<>
bool SubRule<Or<ASubRule>>::identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const
{
    if(prop->type()==PropType::OR_PROP)
    {
        auto propCast=std::dynamic_pointer_cast<const SubTheorem<Or<ASubTheorem>>>(prop);
        return (*m_son)[0]->identifyPriv(propCast->getSon()[0],dbVarProp) && (*m_son)[1]->identifyPriv(propCast->getSon()[1],dbVarProp);
    }
    return false;
}

bool SubRule<Boolean>::identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const
{
    if(!dbVarProp.contains(m_son->name()))
    {
        dbVarProp[m_son->name()]=prop;
        return true;
    }
    else
    {
        auto exPropCast=std::dynamic_pointer_cast<const ASubRule>(dbVarProp[m_son->name()]);
        if(exPropCast)
        {
            return *prop==*exPropCast;
        }
        else
        {
            return false;
        }

    }
}

bool SubRule<ConstBoolean>::identifyPriv(const ptr<ASubTheorem> &prop, DbVarProp&) const
{
    return (*this)==*prop;
}


/**---------------------------------------------------------------
 * applyPriv methods
 * ---------------------------------------------------------------
 */
template<>
ptr<ASubTheorem> SubRule<And<ASubRule>>::applyPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    return std::make_shared<const SubTheorem<And<ASubTheorem>>>(thName,(*m_son)[0]->applyPriv(thName+"&&L",dbVarProp),
            (*m_son)[1]->applyPriv(thName+"&&R",dbVarProp));
}

template<>
ptr<ASubTheorem> SubRule<Equivalent<ASubRule>>::applyPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    return std::make_shared<const SubTheorem<Equivalent<ASubTheorem>>>(thName,(*m_son)[0]->applyPriv(thName+"<=>L",dbVarProp),
                                                           (*m_son)[1]->applyPriv(thName+"<=>R",dbVarProp));
}

template<>
ptr<ASubTheorem> SubRule<Implication<ASubRule>>::applyPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    return std::make_shared<const SubTheorem<Implication<ASubTheorem>>>(thName,(*m_son)[0]->applyPriv(thName+"=>L",dbVarProp),
            (*m_son)[1]->applyPriv(thName+"=>R",dbVarProp));
}

ptr<ASubTheorem> SubRule<Hyp<ASubRule>>::applyPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    std::vector<ptr<ASubTheorem>> ret;
    //Handle all variable in Hyp operator before HYP variable
    for(size_t k=0;k<m_son->arity()-2;k++)
    {
        ret.push_back((*m_son)[k]->applyPriv(thName+"HYP"+sizeToString(k),dbVarProp));
    }

    //handle Hyp variable
    std::string hypName=(*m_son)[m_son->arity()-2]->toString();
    std::vector<ptr<ASubTheorem>> hypAssoc=dbVarProp.getHypAssoc(hypName);
    for(size_t k=0;k<hypAssoc.size();k++)
    {
        auto crtSubProp=hypAssoc[k];
        ret.push_back(crtSubProp);
    }

    if(ret.size()>0)
    {
        ret.push_back((*m_son)[m_son->arity()-1]->applyPriv(thName+"HypImpl",dbVarProp));
        return std::make_shared<const SubTheorem<Hyp<ASubTheorem>>>(thName+"Hyp",ret);
    }
    else
    {
        return (*m_son)[m_son->arity()-1]->applyPriv(thName+"HypImpl",dbVarProp);
    }
}

ptr<ASubTheorem> SubRule<Not<ASubRule>>::applyPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    return std::make_shared<SubTheorem<Not<ASubTheorem>>>(thName,(*m_son)[0]->applyPriv(thName+"!",dbVarProp));
}

template<>
ptr<ASubTheorem> SubRule<Or<ASubRule>>::applyPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    return std::make_shared<const SubTheorem<Or<ASubTheorem>>>(thName,(*m_son)[0]->applyPriv(thName+"||L",dbVarProp),
            (*m_son)[1]->applyPriv(thName+"||R",dbVarProp));
}

ptr<ASubTheorem> SubRule<Boolean>::applyPriv(const std::string&, DbVarProp& dbVarProp) const
{
    return std::dynamic_pointer_cast<const ASubTheorem>(dbVarProp[m_son->name()]);
}

ptr<ASubTheorem> SubRule<ConstBoolean>::applyPriv(const std::string&, DbVarProp&) const
{
    return std::make_shared<const SubTheorem<ConstBoolean>>(*this);
}


/**---------------------------------------------------------------
 * applyFirstPriv methods
 * ---------------------------------------------------------------
 */
template<>
ptr<ASubTheorem> SubRule<And<ASubRule>>::applyFirstPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    return std::make_shared<const Theorem<And<ASubTheorem>>>(thName,(*m_son)[0]->applyPriv(thName+"&&L",dbVarProp),
            (*m_son)[1]->applyPriv(thName+"&&R",dbVarProp));
}

template<>
ptr<ASubTheorem> SubRule<Equivalent<ASubRule>>::applyFirstPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    return std::make_shared<const Theorem<Equivalent<ASubTheorem>>>(thName,(*m_son)[0]->applyPriv(thName+"<=>L",dbVarProp),
            (*m_son)[1]->applyPriv(thName+"<=>R",dbVarProp));
}

template<>
ptr<ASubTheorem> SubRule<Implication<ASubRule>>::applyFirstPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    return std::make_shared<const Theorem<Implication<ASubTheorem>>>(thName,(*m_son)[0]->applyPriv(thName+"=>L",dbVarProp),
            (*m_son)[1]->applyPriv(thName+"=>R",dbVarProp));
}

ptr<ASubTheorem> SubRule<Hyp<ASubRule>>::applyFirstPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    std::vector<ptr<ASubTheorem>> ret;
    //Handle all variable in Hyp operator before HYP variable
    for(size_t k=0;k<m_son->arity()-2;k++)
    {
        ret.push_back((*m_son)[k]->applyPriv(thName+"HYP"+sizeToString(k),dbVarProp));
    }

    //handle Hyp variable
    std::string hypName=(*m_son)[m_son->arity()-2]->toString();
    std::vector<ptr<ASubTheorem>> hypAssoc=dbVarProp.getHypAssoc(hypName);
    for(size_t k=0;k<hypAssoc.size();k++)
    {
        auto crtSubProp=hypAssoc[k];
        ret.push_back(crtSubProp);
    }

    if(ret.size()>0)
    {
        ret.push_back((*m_son)[m_son->arity()-1]->applyPriv(thName+"HypImpl",dbVarProp));
        return std::make_shared<const Theorem<Hyp<ASubTheorem>>>(thName+"Hyp",ret);
    }
    else
    {
        return (*m_son)[m_son->arity()-1]->applyFirstPriv(thName+"HypImpl",dbVarProp);
    }
}

ptr<ASubTheorem> SubRule<Not<ASubRule>>::applyFirstPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    return std::make_shared<const Theorem<Not<ASubTheorem>>>(thName,(*m_son)[0]->applyPriv(thName+"!",dbVarProp));
}

template<>
ptr<ASubTheorem> SubRule<Or<ASubRule>>::applyFirstPriv(const std::string& thName, DbVarProp& dbVarProp) const
{
    return std::make_shared<const Theorem<Or<ASubTheorem>>>(thName,(*m_son)[0]->applyPriv(thName+"||L",dbVarProp),
            (*m_son)[1]->applyPriv(thName+"||R",dbVarProp));
}

ptr<ASubTheorem> SubRule<Boolean>::applyFirstPriv(const std::string&, DbVarProp& dbVarProp) const
{
    return std::dynamic_pointer_cast<const ASubTheorem>(dbVarProp[m_son->name()])->copyTheorem();
}

ptr<ASubTheorem> SubRule<ConstBoolean>::applyFirstPriv(const std::string&, DbVarProp&) const
{
    return std::make_shared<const Theorem<ConstBoolean>>(*this);
}


/**---------------------------------------------------------------
 * SubRule::operator==(ASubTheorem) methods
 * ---------------------------------------------------------------
 */
template<typename SubPropertyType>
bool SubRule<SubPropertyType>::operator==(const ASubTheorem &prop) const
{
    auto propCast=dynamic_cast<const SubTheorem<SubPropertyType>*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubRule<Hyp<ASubRule>>::operator==(const ASubTheorem &prop) const
{
    auto propCast=dynamic_cast<const SubTheorem<SubPropertyType>*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubRule<Not<ASubRule>>::operator==(const ASubTheorem &prop) const
{
    auto propCast=dynamic_cast<const SubTheorem<SubPropertyType>*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubRule<Boolean>::operator==(const ASubTheorem &prop) const
{
    auto propCast=dynamic_cast<const SubTheorem<SubPropertyType>*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubRule<ConstBoolean>::operator==(const ASubTheorem &prop) const
{
    auto propCast=dynamic_cast<const SubTheorem<SubPropertyType>*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}


/**---------------------------------------------------------------
 * SubRule::operator==(ASubRule) methods
 * ---------------------------------------------------------------
 */
template<typename SubPropertyType>
bool SubRule<SubPropertyType>::operator==(const ASubRule &prop) const
{
    auto propCast=dynamic_cast<const SubRule*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubRule<Hyp<ASubRule>>::operator==(const ASubRule &prop) const
{
    auto propCast=dynamic_cast<const SubRule*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubRule<Not<ASubRule>>::operator==(const ASubRule &prop) const
{
    auto propCast=dynamic_cast<const SubRule*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubRule<Boolean>::operator==(const ASubRule &prop) const
{
    auto propCast=dynamic_cast<const SubRule*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubRule<ConstBoolean>::operator==(const ASubRule &prop) const
{
    auto propCast=dynamic_cast<const SubRule*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}


/**---------------------------------------------------------------
 * SubTheorem::operator==(ASubRule) methods
 * ---------------------------------------------------------------
 */
template<typename SubPropertyType>
bool SubTheorem<SubPropertyType>::operator==(const ASubRule& prop) const
{
    auto propCast=dynamic_cast<const SubRule<SubPropertyType>*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubTheorem<Hyp<ASubTheorem>>::operator==(const ASubRule& prop) const
{
    auto propCast=dynamic_cast<const SubRule<SubPropertyType>*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubTheorem<Not<ASubTheorem>>::operator==(const ASubRule& prop) const
{
    auto propCast=dynamic_cast<const SubRule<SubPropertyType>*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubTheorem<Boolean>::operator==(const ASubRule& prop) const
{
    auto propCast=dynamic_cast<const SubRule<Boolean>*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}

bool SubTheorem<ConstBoolean>::operator==(const ASubRule& prop) const
{
    auto propCast=dynamic_cast<const SubRule<SubPropertyType>*>(&prop);
    if(propCast)
    {
        return *this->m_son==propCast->getSon();
    }
    else
    {
        return false;
    }
}


/**---------------------------------------------------------------
 * SubRule::operator==(SubRule) methods
 * ---------------------------------------------------------------
 */
template<typename SubPropertyType>
bool SubRule<SubPropertyType>::operator==(const SubRule &prop) const
{
    return *m_son==prop.getSon();
}


/**---------------------------------------------------------------
 * SubRule::operator==(SubTheorem) methods
 * ---------------------------------------------------------------
 */
template<typename SubPropertyType>
bool SubRule<SubPropertyType>::operator==(const SubTheorem<SubPropertyType> &prop) const
{
    return *m_son==prop.getSon();
}


/**---------------------------------------------------------------
 * SubTheorem::operator==(SubRule) methods
 * ---------------------------------------------------------------
 */
template<typename SubPropertyType>
bool SubTheorem<SubPropertyType>::operator==(const SubRule<SubPropertyType>& prop) const
{
    return *m_son==(prop.getSon());
}



/**----------------------------------------------------------------
 *  Equal operator case
 * ----------------------------------------------------------------
 */
template<typename ValueType1, typename ValueType2>
bool SubRule<Equal<ASubArithmeticRule<ValueType1>, ASubArithmeticRule<ValueType2> > >::
identifyPriv(const ptr<ASubTheorem> &prop, DbVarProp &dbVarProp) const
{
    if(prop->type()==PropType::EQUAL_PROP)
    {
        auto propCast=std::dynamic_pointer_cast<const SubTheorem<Equal<ASubArithmeticTheorem<ValueType1>,
                ASubArithmeticTheorem<ValueType2> >>>(prop);
        return get<0>(*m_son)->identifyPriv(get<0>(propCast->getSon()),dbVarProp) &&
                get<1>(*m_son)->identifyPriv(get<1>(propCast->getSon()),dbVarProp);
    }
    return false;
}

template<typename ValueType1, typename ValueType2>
ptr<ASubTheorem> SubRule<Equal<ASubArithmeticRule<ValueType1>, ASubArithmeticRule<ValueType2> > >::
applyPriv(const std::string &thName, DbVarProp &dbVarProp) const
{
    return std::make_shared<const SubTheorem<Equal<ASubArithmeticTheorem<ValueType1>, ASubArithmeticTheorem<ValueType2> >>>(thName,
    get<0>(*m_son)->applyPriv(thName+"==L",dbVarProp),get<1>(*m_son)->applyPriv(thName+"==R",dbVarProp));
}

template<typename ValueType1, typename ValueType2>
ptr<ASubTheorem> SubRule<Equal<ASubArithmeticRule<ValueType1>, ASubArithmeticRule<ValueType2> > >::
applyFirstPriv(const std::string &thName, DbVarProp &dbVarProp) const
{
    return std::make_shared<const Theorem<Equal<ASubArithmeticTheorem<ValueType1>, ASubArithmeticTheorem<ValueType2> >>>(thName,
    get<0>(*m_son)->applyPriv(thName+"==L",dbVarProp),get<1>(*m_son)->applyPriv(thName+"==R",dbVarProp));
}


/**----------------------------------------------------------------
 *  BelongsTo operator case
 * ----------------------------------------------------------------
 */
template<typename SetType>
bool SubRule<BelongsTo<ASubArithmeticRule<typename SetType::Type>,ASubArithmeticRule<SetType>> >::
identifyPriv(const ptr<ASubTheorem> &prop, DbVarProp &dbVarProp) const
{
    if(prop->type()==PropType::EQUAL_PROP)
    {
        auto propCast=std::dynamic_pointer_cast<const SubTheorem<BelongsTo<ASubArithmeticTheorem<typename SetType::Type>,
                ASubArithmeticTheorem<SetType>>>>(prop);
        return get<0>(*m_son)->identifyPriv(get<0>(propCast->getSon()),dbVarProp) &&
                get<1>(*m_son)->identifyPriv(get<1>(propCast->getSon()),dbVarProp);
    }
    return false;
}

template<typename SetType>
ptr<ASubTheorem> SubRule<BelongsTo<ASubArithmeticRule<typename SetType::Type>, ASubArithmeticRule<SetType> > >::
applyPriv(const std::string &thName, DbVarProp &dbVarProp) const
{
    return std::make_shared<const SubTheorem<BelongsTo<ASubArithmeticTheorem<typename SetType::Type>,ASubArithmeticTheorem<SetType>>>>
            (thName,get<0>(*m_son)->applyPriv(thName+"==L",dbVarProp),get<1>(*m_son)->applyPriv(thName+"==R",dbVarProp));
}

template<typename SetType>
ptr<ASubTheorem> SubRule<BelongsTo<ASubArithmeticRule<typename SetType::Type>,ASubArithmeticRule<SetType>> >::
applyFirstPriv(const std::string &thName, DbVarProp &dbVarProp) const
{
    return std::make_shared<const Theorem<BelongsTo<ASubArithmeticTheorem<typename SetType::Type>,ASubArithmeticTheorem<SetType>>>>
                                    (thName,get<0>(*m_son)->applyPriv(thName+"==L",dbVarProp),get<1>(*m_son)->applyPriv(thName+"==R",dbVarProp));
}

/**----------------------------------------------------------------
 *  Let operator case
 * ----------------------------------------------------------------
 */
template<>
bool SubRule<Let<ASubArithmeticRule<void>, ASubRule > >::
identifyPriv(const ptr<ASubTheorem> &prop, DbVarProp &dbVarProp) const
{
    auto propCast=std::dynamic_pointer_cast<const SubTheorem<Let<ASubArithmeticTheorem<void>,
            ASubTheorem>>>(prop);
    if(propCast)
    {
        return get<0>(*m_son)->identifyPriv(get<0>(propCast->getSon()),dbVarProp) &&
                get<1>(*m_son)->identifyPriv(get<1>(propCast->getSon()),dbVarProp);
    }
    return false;
}

template<>
ptr<ASubTheorem> SubRule<Let<ASubArithmeticRule<void>, ASubRule > >::
applyPriv(const std::string &thName, DbVarProp &dbVarProp) const
{
    return std::make_shared<const SubTheorem<Let<ASubArithmeticTheorem<void>, ASubTheorem> >>(thName,
    get<0>(*m_son)->applyPriv(thName+"==L",dbVarProp),get<1>(*m_son)->applyPriv(thName+"==R",dbVarProp));
}

template<>
ptr<ASubTheorem> SubRule<Let<ASubArithmeticRule<void>, ASubRule > >::
applyFirstPriv(const std::string &thName, DbVarProp &dbVarProp) const
{
    return std::make_shared<const Theorem<Let<ASubArithmeticTheorem<void>, ASubTheorem>>>(thName,
    get<0>(*m_son)->applyPriv(thName+"==L",dbVarProp),get<1>(*m_son)->applyPriv(thName+"==R",dbVarProp));
}


template<>
bool SubRule<Let<ASubRule, ASubRule > >::
identifyPriv(const ptr<ASubTheorem> &prop, DbVarProp &dbVarProp) const
{
    auto propCast=std::dynamic_pointer_cast<const SubTheorem<Let<ASubTheorem,
            ASubTheorem>>>(prop);
    if(propCast)
    {
        return get<0>(*m_son)->identifyPriv(get<0>(propCast->getSon()),dbVarProp) &&
                get<1>(*m_son)->identifyPriv(get<1>(propCast->getSon()),dbVarProp);
    }
    return false;
}

template<>
ptr<ASubTheorem> SubRule<Let<ASubRule, ASubRule > >::
applyPriv(const std::string &thName, DbVarProp &dbVarProp) const
{
    return std::make_shared<const SubTheorem<Let<ASubTheorem, ASubTheorem> >>(thName,
    get<0>(*m_son)->applyPriv(thName+"==L",dbVarProp),get<1>(*m_son)->applyPriv(thName+"==R",dbVarProp));
}

template<>
ptr<ASubTheorem> SubRule<Let<ASubRule, ASubRule > >::
applyFirstPriv(const std::string &thName, DbVarProp &dbVarProp) const
{
    return std::make_shared<const Theorem<Let<ASubTheorem, ASubTheorem>>>(thName,
    get<0>(*m_son)->applyPriv(thName+"==L",dbVarProp),get<1>(*m_son)->applyPriv(thName+"==R",dbVarProp));
}
