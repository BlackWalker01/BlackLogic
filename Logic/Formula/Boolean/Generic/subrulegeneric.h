#ifndef SUBRULEGENERIC_H
#define SUBRULEGENERIC_H
#include "Formula/Boolean/subrule.h"
#include "Formula/Boolean/asubpurerule.h"
#include "Operator/Boolean/and.h"
#include "Operator/Boolean/equivalent.h"
#include "Operator/Boolean/implication.h"
#include "Operator/Boolean/or.h"

namespace N_Logic
{
template<SubRuleProperty SubPropertyType_>
class SubRule: public ASubPureRule
{
public:
    using SubPropertyType = SubPropertyType_;

    SubRule(const ptr<ASubRule>& leftSubProp, const ptr<ASubRule>& rightSubProp);

    bool evaluate() const override final;
    constexpr PropType type() const override final
    {
        if constexpr (std::is_same_v<SubPropertyType, And<ASubRule>>)
        {
            return AND_PROP;
        }
        else if constexpr (std::is_same_v<SubPropertyType, Equivalent<ASubRule>>)
        {
            return EQUIV_PROP;
        }
        else if constexpr (std::is_same_v<SubPropertyType, Implication<ASubRule>>)
        {
            return IMPL_PROP;
        }
        else if constexpr (std::is_same_v<SubPropertyType, Not<ASubRule>>)
        {
            return NOT_PROP;
        }
        else if constexpr (std::is_same_v<SubPropertyType, Or<ASubRule>>)
        {
            return OR_PROP;
        }
    }

    bool isEqual(const ASubRule& prop) const override final;
    bool isEqual(const ASubTheorem& prop) const override final;
    bool operator==(const SubRule& prop) const;
    bool operator==(const SubTheorem<ToTheoremOpe<SubPropertyType>>& prop) const;


    std::string toString(unsigned short priorityParent=1000) const override final;
    const SubPropertyType& getSon() const;
    const DbVar* getExtVars() const override final;

    ~SubRule() override = default;

protected:
    bool identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const override final;
    ptr<ASubTheorem> applyPriv(DbVarProp& dbVarProp) const override final;
    ptr<ASubTheorem> applyFirstPriv(DbVarProp& dbVarProp) const override final;

    const ptr<ASubRule>& operator[](const size_t& index) const override final;
    size_t arity() const override final;

protected:
    const std::unique_ptr<const SubPropertyType> m_son;
    const DbVar m_extVars;
};

template<SubRuleProperty SubPropertyType> 
struct ToTheoremStruct<SubRule<SubPropertyType>> { using Type = SubTheorem<SubPropertyType>; };

/**
 * IdentifyPriv methods
 */
template<>
bool SubRule<And<ASubRule>>::identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const;

template<>
bool SubRule<Equivalent<ASubRule>>::identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const;

template<>
bool SubRule<Implication<ASubRule>>::identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const;

template<>
bool SubRule<Or<ASubRule>>::identifyPriv(const ptr<ASubTheorem>& prop, DbVarProp& dbVarProp) const;


/**
 * applyPriv methods
 */
template<>
ptr<ASubTheorem> SubRule<And<ASubRule>>::applyPriv(DbVarProp& dbVarProp) const;

template<>
ptr<ASubTheorem> SubRule<Equivalent<ASubRule>>::applyPriv(DbVarProp& dbVarProp) const;

template<>
ptr<ASubTheorem> SubRule<Implication<ASubRule>>::applyPriv(DbVarProp& dbVarProp) const;

template<>
ptr<ASubTheorem> SubRule<Or<ASubRule>>::applyPriv(DbVarProp& dbVarProp) const;

/**
 * applyFirstPriv methods
 */
template<>
ptr<ASubTheorem> SubRule<And<ASubRule>>::applyFirstPriv(DbVarProp& dbVarProp) const;

template<>
ptr<ASubTheorem> SubRule<Equivalent<ASubRule>>::applyFirstPriv(DbVarProp& dbVarProp) const;

template<>
ptr<ASubTheorem> SubRule<Implication<ASubRule>>::applyFirstPriv(DbVarProp& dbVarProp) const;

template<>
ptr<ASubTheorem> SubRule<Or<ASubRule>>::applyFirstPriv(DbVarProp& dbVarProp) const;

template<SubRuleProperty SubPropertyType>
const DbVar* SubRule<SubPropertyType>::getExtVars() const
{
    return &m_extVars;
}
}

#endif // SUBRULEGENERIC_H
