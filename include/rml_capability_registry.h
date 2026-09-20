#ifndef RML_CAPABILITY_REGISTRY_H
#define RML_CAPABILITY_REGISTRY_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

//! Functional areas represented by the capability registry.
enum class RCapabilityDomain
{
    Analysis,
    Element,
    Material,
    BoundaryCondition,
    InitialCondition,
    EnvironmentCondition,
    Mesher,
    Solver
};

//! Queryable evidence for one product capability.
class RCapability
{
    private:

        RCapabilityDomain domain;
        QString id;
        QString name;
        bool registered;
        bool implemented;
        bool tested;
        bool validated;
        QStringList exposedProfiles;
        QString diagnostic;
        QJsonObject metadata;

    public:

        RCapability(RCapabilityDomain domain = RCapabilityDomain::Analysis,
                    const QString &id = QString(),
                    const QString &name = QString(),
                    bool registered = false,
                    bool implemented = false,
                    bool tested = false,
                    bool validated = false,
                    const QStringList &exposedProfiles = QStringList(),
                    const QString &diagnostic = QString(),
                    const QJsonObject &metadata = QJsonObject());

        RCapabilityDomain getDomain() const;
        const QString &getId() const;
        const QString &getName() const;
        bool isRegistered() const;
        bool isImplemented() const;
        bool isTested() const;
        bool isValidated() const;
        bool isExposed(const QString &profileId) const;
        const QStringList &getExposedProfiles() const;
        const QString &getDiagnostic() const;
        const QJsonObject &getMetadata() const;

        QJsonObject toJson() const;
        static bool fromJson(const QJsonObject &json, RCapability &capability, QString *error = nullptr);

        static QString domainToId(RCapabilityDomain domain);
        static bool domainFromId(const QString &id, RCapabilityDomain &domain);
};

//! Central registry used by product profiles, GUI discovery and agents.
class RCapabilityRegistry
{
    private:

        QList<RCapability> capabilities;

    public:

        static constexpr int SchemaVersion = 1;

        void add(const RCapability &capability);
        const RCapability *find(RCapabilityDomain domain, const QString &id) const;
        QList<RCapability> list(RCapabilityDomain domain,
                                const QString &profileId = QString(),
                                bool exposedOnly = false) const;
        const QList<RCapability> &getAll() const;

        QJsonObject toJson() const;
        static bool fromJson(const QJsonObject &json, RCapabilityRegistry &registry, QString *error = nullptr);

        //! Return the immutable registry compiled into this build.
        static const RCapabilityRegistry &getBuiltIn();
};

#endif // RML_CAPABILITY_REGISTRY_H
