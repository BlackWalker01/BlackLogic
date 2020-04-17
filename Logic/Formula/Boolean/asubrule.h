#ifndef ASUBRULE_H
#define ASUBRULE_H
#include "Operator/ioperator.h"
#include "iproposition.h"
#include "Formula/isubruleformula.h"

namespace N_Logic
{

class ASubTheorem;
class IISubTheoremFormula;

class ASubRule: public IProposition, public ISubRuleFormula<ASubTheorem>
{
public:
    ASubRule() = default;

    ASubRule(const ASubRule&) = delete;

    virtual std::string name() const;
    virtual ptr<IISubTheoremFormula> applyAnnexe(const size_t& actionKey,
                                                        const ptr<IISubTheoremFormula> &theorem, std::vector<Arity>& indexes) const;
    virtual bool isSymetric() const;
    virtual void clearAllIdentifications() const;

    static void addAbstractTerm(const std::string& varName, std::vector<std::shared_ptr<VariableContainer> > &varList, DbVarContainer &dbVar);

    virtual ~ASubRule() = default;
};

template<> struct ToTheoremStruct<ASubRule> { using Type = ASubTheorem; };

ptr<IOperator> createRuleOperator(const Name name, const Arity& arity=0);
void addRuleOperator(const Name name, std::vector<OperatorOrdering> &opeList,
                     const std::vector<OperatorOrdering>& hyps, const size_t &numPar);
}

#endif // ASUBRULE_H
