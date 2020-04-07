#ifndef SUBRULECONSTBOOLEAN_H
#define SUBRULECONSTBOOLEAN_H
#include "Formula/Boolean/subrule.h"
#include "Variable/Boolean/constboolean.h"

namespace N_Logic
{
template<>
class SubRule<ConstBoolean>: public ASubRule
{
public:
    typedef ConstBoolean SubPropertyType;

    SubRule(const bool& son);

    bool evaluate() const override final;
    constexpr PropType type() const override final;

    bool isEqual(const ASubRule& prop) const override final;
    bool isEqual(const ASubTheorem& prop) const override final;
    bool operator==(const SubRule& prop) const;
    bool operator==(const SubTheorem<SubPropertyType>& prop) const;


    std::string toString(unsigned short priorityParent=1000) const override final;
    const DbVar* getExtVars() const override final;
    const SubPropertyType& getSon() const;

    ~SubRule() override = default;

protected:
    bool identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const override final;
    ptr<ASubTheorem> applyPriv(DbVarProp& dbVarProp) const override final;
    ptr<ASubTheorem> applyFirstPriv(DbVarProp& dbVarProp) const override final;
    size_t arity() const override final;

protected:
    const std::unique_ptr<const SubPropertyType> m_son;
};
}

#endif // SUBRULECONSTBOOLEAN_H
