#ifndef DBVAR_H
#define DBVAR_H
#include "unordered_map"
#include <vector>
#include <memory>

namespace N_Logic {

class AbstractTerm;

class DbVar
{
public:
    DbVar() = default;
    DbVar(const std::shared_ptr<const AbstractTerm>& var);
    DbVar(const DbVar *dbVar);
    DbVar(const DbVar* dbVar, const DbVar* dbVar2);
    DbVar(const std::vector<const DbVar*>& dbVars);

    void insert(AbstractTerm& var);
    void insert(DbVar& ext);

    std::vector<std::string> nameVars() const;
    std::vector<std::shared_ptr<const AbstractTerm>> getVars() const;
    const std::shared_ptr<const AbstractTerm>& find(const std::string &name);
    bool contains(const std::string& name) const;

    ~DbVar() = default;

    static std::unordered_map<std::string, std::shared_ptr<const AbstractTerm> > createDb(const std::shared_ptr<const AbstractTerm> &var);

    static std::unordered_map<std::string, std::shared_ptr<const AbstractTerm> > createDb(const DbVar* db);

    static std::unordered_map<std::string, std::shared_ptr<const AbstractTerm> > merge(const DbVar* db, const DbVar* db2);

    static std::unordered_map<std::string, std::shared_ptr<const AbstractTerm> > merge(const std::vector<const DbVar*>& dbs);

private:
    std::unordered_map<std::string,std::shared_ptr<const AbstractTerm>> db;
};

#include "dbvar.hxx"
}

#endif // DBVAR_H
