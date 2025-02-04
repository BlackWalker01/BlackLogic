/*===--- theoremboolean.h - include for DarkLogic library --------------*- C++ -*-===*
*
*   Part of the DarkLogic Project, under the CC0 license.
*   See https://creativecommons.org/publicdomain/zero/1.0/deed.fr for license information.
*
*===----------------------------------------------------------------------===*
*
*   This file defines Theorem<Boolean> in DarkLogic Library
*   Author: BlackWalker
*   Date: april 12 2020
*
*===----------------------------------------------------------------------===*/

#ifndef DARK_LOGIC_THEOREMBOOLEAN_H
#define DARK_LOGIC_THEOREMBOOLEAN_H
#include "Formula/Boolean/theorem.h"

namespace N_DarkLogic
{
template<>
class Theorem<Boolean>: public SubTheorem<Boolean>
{
public:
    typedef Boolean SubPropertyType;

    Theorem(const std::shared_ptr<Boolean>& son);

    ptr<IISubTheoremFormula> ruleApply(const IISubRuleFormula& rule, DbVarProp& dbVarProp,
        std::vector<Arity>& indexes, const Action::Id& actionKey) const override final;
    ptr<IISubTheoremFormula> ruleApply(const IISubRuleFormula& rule, DbVarProp& dbVarProp,
        std::vector<Arity>& indexes, const Action::Id& actionKey, const size_t& logicIdx) const override final;

    ~Theorem() override = default;


};
}
#endif // DARK_LOGIC_THEOREMBOOLEAN_H
