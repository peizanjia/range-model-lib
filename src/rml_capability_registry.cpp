#include <QJsonArray>

#include "rml_boundary_condition.h"
#include "rml_capability_registry.h"
#include "rml_element.h"
#include "rml_element_descriptor.h"
#include "rml_environment_condition.h"
#include "rml_initial_condition.h"
#include "rml_material_property.h"
#include "rml_mesh_generator.h"
#include "rml_problem.h"

namespace
{

const QString FullProfileId = QStringLiteral("full");
const QString StructuralProfileId = QStringLiteral("structural");

QStringList profiles(bool structural)
{
    QStringList values{FullProfileId};
    if (structural)
    {
        values.append(StructuralProfileId);
    }
    return values;
}

RProblemTypeMask registeredProblemTypeMask()
{
    return R_PROBLEM_ACOUSTICS |
           R_PROBLEM_FLUID_PARTICLE |
           R_PROBLEM_ELECTROSTATICS |
           R_PROBLEM_MAGNETOSTATICS |
           R_PROBLEM_FLUID |
           R_PROBLEM_FLUID_HEAT |
           R_PROBLEM_HEAT |
           R_PROBLEM_RADIATIVE_HEAT |
           R_PROBLEM_STRESS |
           R_PROBLEM_STRESS_MODAL |
           R_PROBLEM_POTENTIAL |
           R_PROBLEM_WAVE |
           R_PROBLEM_MESH;
}

bool problemImplemented(RProblemType type)
{
    return type != R_PROBLEM_POTENTIAL;
}

bool problemTested(RProblemType type)
{
    return type == R_PROBLEM_ACOUSTICS ||
           type == R_PROBLEM_STRESS ||
           type == R_PROBLEM_STRESS_MODAL;
}

bool problemValidated(RProblemType type)
{
    return type == R_PROBLEM_STRESS || type == R_PROBLEM_STRESS_MODAL;
}

bool structuralProblem(RProblemType type)
{
    return type == R_PROBLEM_STRESS || type == R_PROBLEM_STRESS_MODAL;
}

bool structuralElement(RElementType type)
{
    return type == R_ELEMENT_POINT ||
           type == R_ELEMENT_TRUSS1 ||
           type == R_ELEMENT_TRUSS2 ||
           type == R_ELEMENT_BEAM1 ||
           type == R_ELEMENT_BEAM2 ||
           type == R_ELEMENT_TRI1 ||
           type == R_ELEMENT_TRI2 ||
           type == R_ELEMENT_QUAD1 ||
           type == R_ELEMENT_QUAD2 ||
           type == R_ELEMENT_TETRA1 ||
           type == R_ELEMENT_WEDGE1 ||
           type == R_ELEMENT_HEXA1;
}

bool testedElement(RElementType type)
{
    return type == R_ELEMENT_TRUSS1 ||
           type == R_ELEMENT_TRUSS2 ||
           type == R_ELEMENT_BEAM1 ||
           type == R_ELEMENT_BEAM2 ||
           type == R_ELEMENT_TRI1 ||
           type == R_ELEMENT_TRI2 ||
           type == R_ELEMENT_QUAD1 ||
           type == R_ELEMENT_QUAD2 ||
           type == R_ELEMENT_TETRA1 ||
           type == R_ELEMENT_WEDGE1 ||
           type == R_ELEMENT_HEXA1;
}

bool validatedElement(RElementType type)
{
    // Keep independent from testedElement: validation requires a separate
    // analytical, patch, or convergence reference, not only automated coverage.
    return type == R_ELEMENT_TRUSS1 ||
           type == R_ELEMENT_TRUSS2 ||
           type == R_ELEMENT_BEAM1 ||
           type == R_ELEMENT_BEAM2 ||
           type == R_ELEMENT_TRI1 ||
           type == R_ELEMENT_TRI2 ||
           type == R_ELEMENT_QUAD1 ||
           type == R_ELEMENT_QUAD2 ||
           type == R_ELEMENT_TETRA1 ||
           type == R_ELEMENT_WEDGE1 ||
           type == R_ELEMENT_HEXA1;
}

RCapabilityRegistry buildRegistry()
{
    RCapabilityRegistry registry;

    const std::vector<RProblemType> problemTypes =
        RProblem::getTypes(registeredProblemTypeMask());
    for (RProblemType type : problemTypes)
    {
        const bool implemented = problemImplemented(type);
        const bool tested = problemTested(type);
        const bool validated = problemValidated(type);
        registry.add(RCapability(RCapabilityDomain::Analysis,
                                 RProblem::getId(type),
                                 RProblem::getName(type),
                                 true,
                                 implemented,
                                 tested,
                                 validated,
                                 profiles(structuralProblem(type)),
                                 implemented ? QString() : QStringLiteral("Registered for model compatibility; formulation is not implemented.")));
        registry.add(RCapability(RCapabilityDomain::Solver,
                                 RProblem::getId(type),
                                 RProblem::getName(type) + QStringLiteral(" solver"),
                                 true,
                                 implemented,
                                 tested,
                                 validated,
                                 profiles(structuralProblem(type)),
                                 implemented ? QString() : QStringLiteral("No solver implementation is registered.")));
    }

    for (int value = int(R_ELEMENT_POINT); value < int(R_ELEMENT_N_TYPES); ++value)
    {
        const RElementType type = RElementType(value);
        const bool implemented = structuralElement(type);
        const bool tested = testedElement(type);
        const bool validated = validatedElement(type);
        QJsonObject metadata;
        metadata.insert(QStringLiteral("auxiliary"),type == R_ELEMENT_POINT);
        metadata.insert(QStringLiteral("descriptor"),RElementDescriptor::get(type).toJson());
        registry.add(RCapability(RCapabilityDomain::Element,
                                 RElement::getId(type),
                                 RElement::getName(type),
                                 true,
                                 implemented,
                                 tested,
                                 validated,
                                 profiles(implemented),
                                 implemented ? QString() : QStringLiteral("Registered for file compatibility; structural formulation is not implemented."),
                                 metadata));
    }

    for (int value = int(RMaterialProperty::None) + 1; value < int(RMaterialProperty::nTypes); ++value)
    {
        const RMaterialProperty::Type type = RMaterialProperty::Type(value);
        const bool structural = (RMaterialProperty::getProblemTypeMask(type) &
                                 (R_PROBLEM_STRESS | R_PROBLEM_STRESS_MODAL)) != 0;
        registry.add(RCapability(RCapabilityDomain::Material,
                                 RMaterialProperty::getId(type),
                                 RMaterialProperty::getName(type),
                                 true,true,false,false,profiles(structural)));
    }

    for (int value = int(R_BOUNDARY_CONDITION_NONE) + 1; value < int(R_BOUNDARY_CONDITION_N_TYPES); ++value)
    {
        const RBoundaryConditionType type = RBoundaryConditionType(value);
        const bool structural = (RBoundaryCondition::getProblemTypeMask(type) &
                                 (R_PROBLEM_STRESS | R_PROBLEM_STRESS_MODAL)) != 0;
        registry.add(RCapability(RCapabilityDomain::BoundaryCondition,
                                 RBoundaryCondition::getId(type),
                                 RBoundaryCondition::getName(type),
                                 true,true,false,false,profiles(structural)));
    }

    for (int value = int(R_INITIAL_CONDITION_NONE) + 1; value < int(R_INITIAL_CONDITION_N_TYPES); ++value)
    {
        const RInitialConditionType type = RInitialConditionType(value);
        registry.add(RCapability(RCapabilityDomain::InitialCondition,
                                 RInitialCondition::getId(type),
                                 RInitialCondition::getName(type),
                                 true,true,false,false,profiles(false)));
    }

    for (int value = int(R_ENVIRONMENT_CONDITION_NONE) + 1; value < int(R_ENVIRONMENT_CONDITION_N_TYPES); ++value)
    {
        const REnvironmentConditionType type = REnvironmentConditionType(value);
        const bool structural = (REnvironmentCondition::getProblemTypeMask(type) &
                                 (R_PROBLEM_STRESS | R_PROBLEM_STRESS_MODAL)) != 0;
        registry.add(RCapability(RCapabilityDomain::EnvironmentCondition,
                                 REnvironmentCondition::getId(type),
                                 REnvironmentCondition::getName(type),
                                 true,true,false,false,profiles(structural)));
    }

    registry.add(RCapability(RCapabilityDomain::Mesher,
                             QStringLiteral("tetgen.volume"),
                             QStringLiteral("TetGen volume mesher"),
                             true,true,true,false,profiles(true),
                             QStringLiteral("Automated closed-surface smoke coverage exists; independent mesh-quality and assignment-mapping validation is pending.")));

#ifdef RANGE_HAVE_GMSH
    const QStringList availableBackends = RMeshGenerator::getAvailableBackends();
    if (availableBackends.contains(QStringLiteral("gmsh-4")))
    {
        registry.add(RCapability(RCapabilityDomain::Mesher,
                                 QStringLiteral("gmsh-4"),
                                 QStringLiteral("Gmsh 4 mesher"),
                                 true,true,true,false,profiles(true),
                                 QStringLiteral("Automated 1D/2D/3D backend, quality, and named-set tests exist; independent mesh-reference or convergence validation is pending.")));
    }
#endif

    return registry;
}

}

RCapability::RCapability(RCapabilityDomain domain,
                         const QString &id,
                         const QString &name,
                         bool registered,
                         bool implemented,
                         bool tested,
                         bool validated,
                         const QStringList &exposedProfiles,
                         const QString &diagnostic,
                         const QJsonObject &metadata)
    : domain(domain)
    , id(id)
    , name(name)
    , registered(registered)
    , implemented(implemented)
    , tested(tested)
    , validated(validated)
    , exposedProfiles(exposedProfiles)
    , diagnostic(diagnostic)
    , metadata(metadata)
{
}

RCapabilityDomain RCapability::getDomain() const { return this->domain; }
const QString &RCapability::getId() const { return this->id; }
const QString &RCapability::getName() const { return this->name; }
bool RCapability::isRegistered() const { return this->registered; }
bool RCapability::isImplemented() const { return this->implemented; }
bool RCapability::isTested() const { return this->tested; }
bool RCapability::isValidated() const { return this->validated; }
bool RCapability::isExposed(const QString &profileId) const { return this->exposedProfiles.contains(profileId); }
const QStringList &RCapability::getExposedProfiles() const { return this->exposedProfiles; }
const QString &RCapability::getDiagnostic() const { return this->diagnostic; }
const QJsonObject &RCapability::getMetadata() const { return this->metadata; }

QJsonObject RCapability::toJson() const
{
    QJsonArray profilesJson;
    for (const QString &profile : this->exposedProfiles)
    {
        profilesJson.append(profile);
    }
    return QJsonObject{
        {QStringLiteral("domain"),RCapability::domainToId(this->domain)},
        {QStringLiteral("id"),this->id},
        {QStringLiteral("name"),this->name},
        {QStringLiteral("registered"),this->registered},
        {QStringLiteral("implemented"),this->implemented},
        {QStringLiteral("tested"),this->tested},
        {QStringLiteral("validated"),this->validated},
        {QStringLiteral("exposedProfiles"),profilesJson},
        {QStringLiteral("diagnostic"),this->diagnostic},
        {QStringLiteral("metadata"),this->metadata}
    };
}

bool RCapability::fromJson(const QJsonObject &json, RCapability &capability, QString *error)
{
    RCapabilityDomain domain;
    const QString domainId = json.value(QStringLiteral("domain")).toString();
    if (!RCapability::domainFromId(domainId,domain))
    {
        if (error) *error = QStringLiteral("Unknown capability domain '%1'.").arg(domainId);
        return false;
    }
    const QString id = json.value(QStringLiteral("id")).toString();
    if (id.isEmpty())
    {
        if (error) *error = QStringLiteral("Capability id is empty.");
        return false;
    }
    QStringList profiles;
    for (const QJsonValue &value : json.value(QStringLiteral("exposedProfiles")).toArray())
    {
        profiles.append(value.toString());
    }
    capability = RCapability(domain,
                             id,
                             json.value(QStringLiteral("name")).toString(),
                             json.value(QStringLiteral("registered")).toBool(),
                             json.value(QStringLiteral("implemented")).toBool(),
                             json.value(QStringLiteral("tested")).toBool(),
                             json.value(QStringLiteral("validated")).toBool(),
                             profiles,
                             json.value(QStringLiteral("diagnostic")).toString(),
                             json.value(QStringLiteral("metadata")).toObject());
    return true;
}

QString RCapability::domainToId(RCapabilityDomain domain)
{
    switch (domain)
    {
        case RCapabilityDomain::Analysis: return QStringLiteral("analysis");
        case RCapabilityDomain::Element: return QStringLiteral("element");
        case RCapabilityDomain::Material: return QStringLiteral("material");
        case RCapabilityDomain::BoundaryCondition: return QStringLiteral("boundary-condition");
        case RCapabilityDomain::InitialCondition: return QStringLiteral("initial-condition");
        case RCapabilityDomain::EnvironmentCondition: return QStringLiteral("environment-condition");
        case RCapabilityDomain::Mesher: return QStringLiteral("mesher");
        case RCapabilityDomain::Solver: return QStringLiteral("solver");
    }
    return QString();
}

bool RCapability::domainFromId(const QString &id, RCapabilityDomain &domain)
{
    for (RCapabilityDomain candidate : {RCapabilityDomain::Analysis,
                                        RCapabilityDomain::Element,
                                        RCapabilityDomain::Material,
                                        RCapabilityDomain::BoundaryCondition,
                                        RCapabilityDomain::InitialCondition,
                                        RCapabilityDomain::EnvironmentCondition,
                                        RCapabilityDomain::Mesher,
                                        RCapabilityDomain::Solver})
    {
        if (RCapability::domainToId(candidate) == id)
        {
            domain = candidate;
            return true;
        }
    }
    return false;
}

void RCapabilityRegistry::add(const RCapability &capability)
{
    for (RCapability &current : this->capabilities)
    {
        if (current.getDomain() == capability.getDomain() && current.getId() == capability.getId())
        {
            current = capability;
            return;
        }
    }
    this->capabilities.append(capability);
}

const RCapability *RCapabilityRegistry::find(RCapabilityDomain domain, const QString &id) const
{
    for (const RCapability &capability : this->capabilities)
    {
        if (capability.getDomain() == domain && capability.getId() == id)
        {
            return &capability;
        }
    }
    return nullptr;
}

QList<RCapability> RCapabilityRegistry::list(RCapabilityDomain domain,
                                             const QString &profileId,
                                             bool exposedOnly) const
{
    QList<RCapability> values;
    for (const RCapability &capability : this->capabilities)
    {
        if (capability.getDomain() != domain)
        {
            continue;
        }
        if (exposedOnly && !capability.isExposed(profileId))
        {
            continue;
        }
        values.append(capability);
    }
    return values;
}

const QList<RCapability> &RCapabilityRegistry::getAll() const
{
    return this->capabilities;
}

QJsonObject RCapabilityRegistry::toJson() const
{
    QJsonArray capabilityJson;
    for (const RCapability &capability : this->capabilities)
    {
        capabilityJson.append(capability.toJson());
    }
    return QJsonObject{
        {QStringLiteral("schemaVersion"),RCapabilityRegistry::SchemaVersion},
        {QStringLiteral("capabilities"),capabilityJson}
    };
}

bool RCapabilityRegistry::fromJson(const QJsonObject &json,
                                   RCapabilityRegistry &registry,
                                   QString *error)
{
    if (json.value(QStringLiteral("schemaVersion")).toInt(-1) != RCapabilityRegistry::SchemaVersion)
    {
        if (error) *error = QStringLiteral("Unsupported capability registry schema version.");
        return false;
    }
    RCapabilityRegistry parsed;
    const QJsonArray capabilityJson = json.value(QStringLiteral("capabilities")).toArray();
    for (const QJsonValue &value : capabilityJson)
    {
        RCapability capability;
        if (!RCapability::fromJson(value.toObject(),capability,error))
        {
            return false;
        }
        if (parsed.find(capability.getDomain(),capability.getId()))
        {
            if (error) *error = QStringLiteral("Duplicate capability '%1/%2'.")
                                    .arg(RCapability::domainToId(capability.getDomain()),capability.getId());
            return false;
        }
        parsed.add(capability);
    }
    registry = parsed;
    return true;
}

const RCapabilityRegistry &RCapabilityRegistry::getBuiltIn()
{
    static const RCapabilityRegistry registry = buildRegistry();
    return registry;
}
