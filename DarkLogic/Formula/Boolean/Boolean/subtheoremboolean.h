/*===--- subtheoremboolean.h - include for DarkLogic library --------------*- C++ -*-===*
*
*   Part of the DarkLogic Project, under the CC0 license.
*   See https://creativecommons.org/publicdomain/zero/1.0/deed.fr for license information.
*
*===----------------------------------------------------------------------===*
*
*   This file defines SubTheorem<Boolean> in DarkLogic Library
*   Author: BlackWalker
*   Date: april 12 2020
*
*===----------------------------------------------------------------------===*/

#ifndef DARK_LOGIC_SUBTHEOREMBOOLEAN_H
#define DARK_LOGIC_SUBTHEOREMBOOLEAN_H
#include "Formula/Boolean/subtheorem.h"

namespace N_DarkLogic
{
template<>
class SubTheorem<Boolean>: public ASubTheorem
{
public:
    typedef Boolean SubPropertyType;

    SubTheorem(const std::shared_ptr<Boolean>& val);

    bool isEvaluated() const override final;
    bool evaluate() const override final;
    const State& getState() const override final;
    bool canBeDemonstrated() const override final;
    bool testEvaluate(const Evaluater::ConfigEval& configEval) const override final;
    bool getHiddenValue() const override final;
    std::unordered_map<IDVar, IDVar> getVarToEval() const override final;
    std::vector<std::pair<Evaluater::ConfigEval, bool>> getConfigEvals() const override final;
    std::vector<Evaluater::ConfigEval> getCompatibleConfigs(const Evaluater::ConfigEval& commonConfig,
        const std::unordered_map<IDVar, IDVar>& internalVars) const override final;
    constexpr PropType type() const override final
    {
        return PropType::VAR_PROP;
    }

    bool isEqual(const ASubTheorem& prop) const override final;
    bool operator==(const SubTheorem& prop) const;
    bool isEqual(const ASubRule& prop) const override final;
    bool operator==(const SubRule<SubPropertyType>& prop) const;

    std::string toString(unsigned short priorityParent=1000) const override final;
    const DbVar* getExtVars() const override final;

    ptr<ASubTheorem> copyTheorem() const override final;
    const SubPropertyType& getSon() const;
    inline bool isHypProp() const override final
    {
        return m_son->isHypVariable();
    }
    IDVar variableId() const override final;

    ptr<IISubTheoremFormula> ruleApply(const IISubRuleFormula& rule, DbVarProp& dbVarProp,
        std::vector<Arity>& indexes, const Action::Id& actionKey) const override;
    ptr<IISubTheoremFormula> ruleApply(const IISubRuleFormula& rule, DbVarProp& dbVarProp,
        std::vector<Arity>& indexes, const Action::Id& actionKey, const size_t& logicIdx) const override;

    ~SubTheorem() override = default;
protected:
    const std::vector<std::vector<Arity> > &computeAllPaths() override final;
    const std::vector<std::vector<Arity>>& computeImplPaths() override final;

private:
    void initEval();
protected:
    size_t arity() const override final;

    const std::shared_ptr<SubPropertyType> m_son;
    const DbVar m_extVars;
    const std::unique_ptr<Evaluater> m_eval;
    const State m_state;
};
}
#endif // DARK_LOGIC_SUBTHEOREMBOOLEAN_H
