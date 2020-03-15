#include "theorem.h"
#include "rule.h"
#include "Operator/operator.h"
#include "Formula/Arithmetic/allarithformula.h"
#include "Formula/Boolean/allproposition.h"

using namespace N_Logic;

void ASubTheorem::addAbstractTerm(const std::string &varName, std::vector<std::shared_ptr<VariableContainer>> &varList,
                                  DbVarContainer &dbVar)
{
    std::shared_ptr<VariableContainer> varContainer=nullptr;
    std::shared_ptr<AbstractTerm> var=nullptr;
    if(dbVar.contains(varName))
    {
        varContainer=dbVar.find(varName);
    }
    else
    {
        var=AbstractTerm::create(varName);
        varContainer=std::make_shared<VariableContainer>(var);
        if(!var->isConstant())
        {
            dbVar.insert(varContainer);
        }
    }
    varList.push_back(varContainer);
}

/**
 * @brief Property::parseContent
 * @param content
 * Ex: content="a&&b=>!c||d" -> (a&&b)=>((!c)||d)
 */
ptr<ASubTheorem> N_Logic::createTheorem(const std::string &name, const std::string &content)
{
    //parse hypothesis
    char c='\0';

    //look for operators and variables
    size_t numPar=0;
    std::vector<ParenthesisParam> parenthesisParams;
    parenthesisParams.push_back(ParenthesisParam());
    Name crtNameOpe=NONE;
    std::string crtVar="";
    std::vector<OperatorOrdering> opeList;
    //std::vector<unsigned short> parLvlList; //parenthesis level list
    std::vector<OperatorOrdering> hyps;
    std::vector<std::shared_ptr<VariableContainer>> varList;
    DbVarContainer dbVar;
    for(size_t k=0;k<content.size();k++)
    {
        c=content[k];
        switch(c)
        {
            case '(':
            {
                parenthesisParams.push_back(ParenthesisParam(numPar,0,opeList.size()));
                numPar++;
                continue;
            }
            case ')':
            {
                if(hyps.size() && hyps.back().nbPar==numPar)
                {
                    hyps.pop_back();
                }
                numPar--;
                parenthesisParams.pop_back();
                continue;
            }
            case ' ':
            {
                continue;
            }
            //hypothesis cases
            case '{':
            {
                parenthesisParams.push_back(ParenthesisParam(numPar,0,opeList.size()));
                numPar++;
                continue;
            }
            case ',':
            {
                if(numPar==parenthesisParams.back().nbPar+1)
                {
                    parenthesisParams.back().nbArgs++;
                }
                else
                {
                    throw std::runtime_error("Syntax error in the last function. Exit...");
                }
                continue;
            }
            case '}':
            {
                if(numPar==parenthesisParams.back().nbPar+1)
                {
                    parenthesisParams.back().nbArgs++;
                    if(parenthesisParams.back().nbArgs>0)
                    {
                        parenthesisParams.back().nbArgs++; //increment to add implication of hypothesis operator in its arity
                        auto it=opeList.begin()+static_cast<long long>(parenthesisParams.back().indexInOpeList);
                        OperatorOrdering opeOrdering(createRuleOperator(HYP,parenthesisParams.back().nbArgs),
                                                     parenthesisParams.back().nbPar,parenthesisParams[parenthesisParams.size()-2].nbArgs);
                        opeList.insert(it,opeOrdering);
                        hyps.push_back(opeOrdering);
                    }
                    numPar--;
                    parenthesisParams.pop_back();
                }
                else
                {
                    throw std::runtime_error("Syntax error in hypothesis operator. Exit...");
                }
                continue;
            }
            default:
            break;
        }

        //skip strings if it is possible
        IOperator::skipStrings(content,k);
        size_t crtK=k;
        if((crtNameOpe=IOperator::getNextOpeName(content,k)))
        {
            addTheoremOperator(crtNameOpe,opeList,hyps,numPar,parenthesisParams.back().nbArgs,name,k);
            crtNameOpe=NONE;
            k--;
        }
        else
        {
            crtVar=getVarName(content,k);
            if(crtVar!="")
            {
                ASubRule::addAbstractTerm(crtVar,varList,dbVar);
                crtVar="";
                k--;
            }
            else if(k==crtK)
            {
                std::string ch="";
                ch+=c;
                throw std::runtime_error("Unexpected '"+ch+"' character at "+sizeToString(k)+" index");
            }
        }
    }

    //order operators
    std::vector<OperatorOrdering> orderedOpeList=orderOperator(opeList);
    return std::dynamic_pointer_cast<const ASubTheorem>(createTheorem(name,opeList,orderedOpeList,varList));
}

/**---------------------------------------------------------------
 * Constructor methods from subProperties
 * ---------------------------------------------------------------
 */
Theorem<Hyp<ASubTheorem>>::Theorem(const std::string &name_, const std::vector<ptr<ASubTheorem>> &subProps):
    SubTheorem<SubPropertyType>(name_,subProps)
{

}

Theorem<Not<ASubTheorem>>::Theorem(const std::string &name_, const ptr<ASubTheorem> &subProp):
    SubTheorem<SubPropertyType>(name_,subProp)
{

}

Theorem<Boolean>::Theorem(const std::string &name_, const std::shared_ptr<Boolean>& son):
    SubTheorem<SubPropertyType>(name_,son)
{

}

Theorem<ConstBoolean>::Theorem(const std::string &name_, const bool& val):
    SubTheorem<SubPropertyType>(name_,val)
{

}


/**---------------------------------------------------------------
 * Copy constructors from SubTheorems
 * ---------------------------------------------------------------
 */
Theorem<ConstBoolean>::Theorem(const SubRule<ConstBoolean>& prop):
    SubTheorem<ConstBoolean>(prop)
{

}

/**---------------------------------------------------------------
 * ruleApply methods
 * ---------------------------------------------------------------
 */
template<typename SubPropertyType>
ptr<IISubTheoremFormula> N_Logic::Theorem<SubPropertyType>::ruleApply(const IISubRuleFormula &rule, std::vector<size_t> &indexes,
                                                                  const size_t &actionKey) const
{
    Arity index=indexes[0];
    indexes.erase(indexes.begin());
    auto ruleCast=dynamic_cast<const ASubRule*>(&rule);
    if(index==0)
    {
        return std::make_shared<const Theorem<SubPropertyType>>(this->name(),
        std::dynamic_pointer_cast<const ASubTheorem>(ruleCast->applyAnnexe(actionKey,(*this->m_son)[0],indexes)), (*this->m_son)[1]);
    }
    else
    {
        return std::make_shared<const Theorem<SubPropertyType>>(this->name(),
        (*this->m_son)[0], std::dynamic_pointer_cast<const ASubTheorem>(ruleCast->applyAnnexe(actionKey,(*this->m_son)[1],indexes)));
    }
}

ptr<IISubTheoremFormula> N_Logic::Theorem<Hyp<ASubTheorem>>::ruleApply(const IISubRuleFormula &rule, std::vector<size_t> &indexes,
                                                                   const size_t &actionKey) const
{
    Arity index=indexes[0];
    indexes.erase(indexes.begin());
    std::vector<ptr<ASubTheorem>> sons;
    for(size_t k=0;k<index;k++)
    {
        sons.push_back((*m_son)[k]);
    }
    auto ruleCast=dynamic_cast<const ASubRule*>(&rule);
    sons.push_back(std::dynamic_pointer_cast<const ASubTheorem>(ruleCast->applyAnnexe(actionKey,((*m_son)[index]),indexes)));

    for(size_t k=index+1;k<m_son->arity();k++)
    {
        sons.push_back((*m_son)[k]);
    }

    return std::make_shared<const Theorem<SubPropertyType>>(name(),sons);
}

ptr<IISubTheoremFormula> N_Logic::Theorem<Not<ASubTheorem>>::ruleApply(const IISubRuleFormula &rule, std::vector<size_t> &indexes,
                                                                           const size_t &actionKey) const
{
    indexes.erase(indexes.begin());
    auto ruleCast=dynamic_cast<const ASubRule*>(&rule);
    return std::make_shared<const SubTheorem<Not<ASubTheorem>>>(name(),
    std::dynamic_pointer_cast<const ASubTheorem>(ruleCast->applyAnnexe(actionKey,(*m_son)[0],indexes)));
}

ptr<IISubTheoremFormula> N_Logic::Theorem<Boolean>::ruleApply(const IISubRuleFormula &, std::vector<size_t> &, const size_t &) const
{
    throw std::runtime_error("Theorem Boolean connot call ruleApply method");
}

ptr<IISubTheoremFormula> N_Logic::Theorem<ConstBoolean>::ruleApply(const IISubRuleFormula &, std::vector<size_t> &,
                                                                       const size_t &) const
{
    throw std::runtime_error("Theorem ConstBoolean connot call ruleApply method");
}


/**---------------------------------------------------------------
 * createTheorem function
 * ---------------------------------------------------------------
 */
ptr<ValueTypeObject> N_Logic::createTheorem(const std::string &name, std::vector<OperatorOrdering> &opeList,
                                            std::vector<OperatorOrdering> &orderedOpeList,
                                                std::vector<std::shared_ptr<VariableContainer> > &varList)
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
            std::vector<ptr<IISubTheoremFormula>> subProps;
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
                subProps.push_back(std::dynamic_pointer_cast<const IISubTheoremFormula>
                                   (createSubTheorem(name+ope->symbol()+sizeToString(k),topOpeList,topOrderedOpeList,varList)));
                topOrderedOpeList.clear();
                queueOrderedOpeList.clear();
            }

            switch (ope->name())
            {
                case HYP:
                {
                    ptr<ASubTheorem> subPropCast=nullptr;
                    std::vector<ptr<ASubTheorem>> subPropCasts;
                    for(auto subProp: subProps)
                    {
                        if((subPropCast=std::dynamic_pointer_cast<const ASubTheorem>(subProp)))
                        {
                            subPropCasts.push_back(subPropCast);
                        }
                        else
                        {
                            throw std::runtime_error("Expected SubTheorem in hypothesis operator");
                        }
                    }
                    return std::make_shared<const Theorem<Hyp<ASubTheorem>>>(name+"HYP",subPropCasts);
                }
                default:
                {
                    throw std::runtime_error("Empty Theorem");
                }
            }
        }
        else
        {
            switch (ope->arity())
            {
                case 1:
                {
                    std::vector<OperatorOrdering> queueOpeList(opeList.begin()+1,opeList.end());
                    std::vector<OperatorOrdering> queueOrderedOpeList(orderedOpeList.begin()+1,orderedOpeList.end());
                    switch(ope->name())
                    {
                        case NOT:
                        {
                            auto subOpe=createSubTheorem(name+"!",queueOpeList,queueOrderedOpeList,varList);
                            switch (subOpe->valueType())
                            {
                                case BOOL_TYPE:
                                {
                                    auto subOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(subOpe);
                                    return std::make_shared<const Theorem<Not<ASubTheorem>>>(name,subOpeCast);
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
                    //separate operators for binary operators
                    std::vector<OperatorOrdering> queueOrderedOpeList(orderedOpeList.begin()+1,orderedOpeList.end());
                    std::vector<OperatorOrdering> leftOpeList;
                    std::vector<OperatorOrdering> rightOpeList;
                    std::vector<OperatorOrdering> leftOrderedOpeList;
                    std::vector<OperatorOrdering> rightOrderedOpeList;
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

                    switch(ope->name())
                    {
                        case AND:
                        {
                            auto leftOpe=createSubTheorem(name+"&&L",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubTheorem(name+"&&R",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case BOOL_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case BOOL_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(rightOpe);
                                            return std::make_shared<const Theorem<And<ASubTheorem>>>
                                            (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("And operator does not handle bool variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("And operator does not handle bool variable");
                                }
                            }
                        }
                        case EQUIVALENT:
                        {
                            auto leftOpe=createSubTheorem(name+"<=>L",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubTheorem(name+"<=>R",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case BOOL_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case BOOL_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(rightOpe);
                                            return std::make_shared<const Theorem<Equivalent<ASubTheorem>>>
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
                            auto leftOpe=createSubTheorem(name+"=>L",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubTheorem(name+"=>R",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case BOOL_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case BOOL_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(rightOpe);
                                            return std::make_shared<const Theorem<Implication<ASubTheorem >>>
                                            (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("Implication operator does not handle bool variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("Implication operator does not handle bool variable");
                                }
                            }
                        }
                        case OR:
                        {
                            auto leftOpe=createSubTheorem(name+"||L",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubTheorem(name+"||R",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case BOOL_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case BOOL_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(rightOpe);
                                            return std::make_shared<const Theorem<Or<ASubTheorem >>>
                                            (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("Or operator does not handle bool variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("Or operator does not handle bool variable");
                                }
                            }
                        }

                        case EQUAL:
                        {
                            auto leftOpe=createSubTheorem(name+"==L",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubTheorem(name+"==R",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case NATURAL_INT_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case NATURAL_INT_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubArithmeticTheorem<size_t>>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubArithmeticTheorem<size_t>>(rightOpe);
                                            return std::make_shared<const Theorem<Equal<ASubArithmeticTheorem<size_t>, ASubArithmeticTheorem<size_t> >>>
                                            (name,leftOpeCast,rightOpeCast);
                                        }
                                        default:
                                        {
                                            throw std::runtime_error("Equal operator does not handle boolean variable");
                                        }
                                    }
                                }
                                default:
                                {
                                    throw std::runtime_error("Equal operator does not handle boolean variable");
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
                                rightOpe=createSubTheorem(name+"€R",rightOpeList,rightOrderedOpeList,varList);
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
                                auto leftOpe=createSubTheorem(name+"€L",leftOpeList,leftOrderedOpeList,varList);
                            }
                            else
                            {
                                leftOpe=createSubTheorem(name+"€L",leftOpeList,leftOrderedOpeList,varList);
                                rightOpe=createSubTheorem(name+"€R",rightOpeList,rightOrderedOpeList,varList);
                            }
                            switch(leftOpe->valueType())
                            {
                                case NATURAL_INT_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case NATURAL_INTEGERSET_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubArithmeticTheorem<size_t>>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubArithmeticTheorem<NaturalIntegerSet>>(rightOpe);
                                            return std::make_shared<const Theorem<BelongsTo<ASubArithmeticTheorem<size_t>,
                                                    ASubArithmeticTheorem<NaturalIntegerSet> >>>(name,leftOpeCast,rightOpeCast);
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
                            auto leftOpe=createSubTheorem(name+"letL",leftOpeList,leftOrderedOpeList,varList);
                            auto rightOpe=createSubTheorem(name+"letR",rightOpeList,rightOrderedOpeList,varList);
                            switch(leftOpe->valueType())
                            {
                                case VOID_TYPE:
                                {
                                    switch(rightOpe->valueType())
                                    {
                                        case BOOL_TYPE:
                                        {
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubArithmeticTheorem<void>>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(rightOpe);
                                            return std::make_shared<const Theorem<Let<ASubArithmeticTheorem<void>,ASubTheorem>>>
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
                                            auto leftOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(leftOpe);
                                            auto rightOpeCast=std::dynamic_pointer_cast<const ASubTheorem>(rightOpe);
                                            return std::make_shared<const Theorem<Let<ASubTheorem,ASubTheorem>>>
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
                        default:
                        {
                            throw std::runtime_error("Only boolean operators in top of theorem");
                        }
                    }
                }
                default:
                {
                    throw std::runtime_error("Empty Theorem");
                }
            }
        }
    }
    else if(varList.size())
    {
        auto var=varList[0]->var;
        varList.erase(varList.begin());
        if(var->isConstant())
        {
            switch(var->valueType())
            {
                case VALUE_TYPE::BOOL_TYPE:
                {
                    return std::make_shared<const Theorem<ConstBoolean>>(name,
                                            std::dynamic_pointer_cast<ConstBoolean>(var)->evaluate());
                }
                default:
                {
                    throw std::runtime_error("Only boolean variable in top of theorem");
                }
            }
        }
        else
        {
            switch(var->valueType())
            {
                case VALUE_TYPE::BOOL_TYPE:
                {
                    return std::make_shared<const Theorem<Boolean>>(name,
                                            std::dynamic_pointer_cast<Boolean>(var));
                }
                default:
                {
                    throw std::runtime_error("Only boolean variable in top of theorem");
                }
            }
        }
    }
    else
    {
        throw std::runtime_error("Empty Theorem");
    }
    return nullptr;
}
