/*===--- booleanset.cpp - source for DarkLogic library --------------*- C++ -*-===*
*
*   Part of the DarkLogic Project, under the CC0 license.
*   See https://creativecommons.org/publicdomain/zero/1.0/deed.fr for license information.
*
*===----------------------------------------------------------------------===*
*
*   This file implements boolean set class for DarkLogic Library
*   Author: BlackWalker
*   Date: april 12 2020
*
*===----------------------------------------------------------------------===*/

#include "booleanset.h"
#include "Formula/Boolean/rule.h"
#include "set.h"
#include "Formula/parser.h"

using namespace N_DarkLogic;

//template<> const std::unique_ptr<AxiomSet<BooleanSet>> Set<BooleanSet>::s_axiomSet=nullptr;
template<> const std::unique_ptr<DbRule> Set<BooleanSet>::s_rules = std::make_unique<DbRule>();

template<> template<RuleType ruleType>
void Set<BooleanSet>::insert(const ptr<ruleType >& rule)
{
    s_rules->insert(rule);
}

void BooleanSet::init()
{
    //AXIOME Rule
    auto ax=std::static_pointer_cast<const Rule<Equivalent<ASubRule>>>(Parser::createFormula<ASubRule>("ax","true<=>({p,HYP}p)"));
    insert(ax);

    //AND rules
    auto andI=std::static_pointer_cast<const Rule<Equivalent<ASubRule>>>(Parser::createFormula<ASubRule>("&&I","(({HYP}p)&&({HYP}q))<=>({HYP}p&&q)"));
    insert(andI);
    auto andE=std::static_pointer_cast<const Rule<Equivalent<ASubRule>>>(Parser::createFormula<ASubRule>("&&E","({HYP}p&&q)<=>(({HYP}p)&&({HYP}q))"));
    insert(andE);

    auto andEl=std::static_pointer_cast<const Rule<Implication<ASubRule>>>(Parser::createFormula<ASubRule>("&&Eg","({HYP}p&&q)=>({HYP}p)")); //complicated to use!
    insert(andEl);
    auto andEr=std::static_pointer_cast<const Rule<Implication<ASubRule>>>(Parser::createFormula<ASubRule>("&&Ed","({HYP}p&&q)=>({HYP}q)")); //complicated to use!
    insert(andEr);

    //OR Rules
    auto orI=std::static_pointer_cast<const Rule<Equivalent<ASubRule>>>(Parser::createFormula<ASubRule>("||I","(({HYP}p)||({HYP}q))<=>({HYP}p||q)"));
    insert(orI);
    auto orIl=std::static_pointer_cast<const Rule<Implication<ASubRule>>>(Parser::createFormula<ASubRule>("||Ig","({HYP}p)=>({HYP}p||q)"));
    insert(orIl);
    auto orIr=std::static_pointer_cast<const Rule<Implication<ASubRule>>>(Parser::createFormula<ASubRule>("||Id","({HYP}q)=>({HYP}p||q)"));
    insert(orIr);
    auto orE=std::static_pointer_cast<const Rule<Implication<ASubRule>>>(Parser::createFormula<ASubRule>("||E","(({HYP}p||q)&&({p,HYP}r)&&({q,HYP}r))=>(({HYP}r)")); //complicated to use!
    insert(orE);

    //IMPLICATION Rules
    auto implI=std::static_pointer_cast<const Rule<Implication<ASubRule>>>(Parser::createFormula<ASubRule>("=>I","({p,HYP}q)=>({HYP}p=>q)"));
    insert(implI);
    auto implE=std::static_pointer_cast<const Rule<Implication<ASubRule>>>(Parser::createFormula<ASubRule>("=>E","(({HYP}p)&&({HYP}p=>q))=>({p,p=>q,HYP}q)"));
    insert(implE);


    //FALSE Rules
    auto fi=std::static_pointer_cast<const Rule<Implication<ASubRule>>>(Parser::createFormula<ASubRule>("FI","({HYP}(p&&!p))=>({HYP}false)"));
    insert(fi);
    auto fe=std::static_pointer_cast<const Rule<Equivalent<ASubRule>>>(Parser::createFormula<ASubRule>("FE","({p,HYP}false)<=>({HYP}!p)"));
    insert(fe);
    auto fiNot=std::static_pointer_cast<const Rule<Implication<ASubRule>>>(Parser::createFormula<ASubRule>("FI!","({!p,HYP}p)=>({!p,HYP}false)"));
    insert(fiNot);

    //WEAKENING Rule
    auto weak=std::static_pointer_cast<const Rule<Implication<ASubRule>>>(Parser::createFormula<ASubRule>("weakening","({HYP}p)=>({q,HYP}p)"));
    insert(weak);

    //ABSURD REASONNING Rules (ARR)
    auto arr=std::static_pointer_cast<const Rule<Equivalent<ASubRule>>>(Parser::createFormula<ASubRule>("arr","({!p,HYP}false)<=>({HYP}p)"));
    insert(arr);
    auto doubleNot=std::static_pointer_cast<const Rule<Equivalent<ASubRule>>>(Parser::createFormula<ASubRule>("!!","({HYP}!!p)<=>({HYP}p)"));
    insert(doubleNot);
}