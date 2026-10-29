#ifndef RML_STRUCTURAL_PIEZO_LAYERED_SETUP_H
#define RML_STRUCTURAL_PIEZO_LAYERED_SETUP_H
#include "rml_structural_piezo_readout_setup.h"

enum class RStructuralPiezoLayeredModelFamily { PlanarBeam = 0, FlatPlate = 1, Solid3D = 2 };
enum class RStructuralPiezoMaterialForm { StressCharge = 0, StrainCharge = 1 };
enum class RStructuralPiezoMaterialSourceKind { Example = 0, Vendor = 1, Calibration = 2 };
enum class RStructuralPiezoPhysicalDof { UX = 0, UY = 1, UZ = 2, RX = 3, RY = 4, RZ = 5 };
enum class RStructuralPiezoDampingKind { None = 0, Rayleigh = 1, UniformModal = 2, PerMode = 3, PerCluster = 4, Imported = 5 };
struct RStructuralPiezoLayeredFrame
{
    std::array<double,3> origin{{0,0,0}}, ex{{1,0,0}}, ey{{0,1,0}}, ez{{0,0,1}};
};
struct RStructuralPiezoLayeredMaterial
{
    QString materialId;
    RStructuralPiezoMaterialForm form = RStructuralPiezoMaterialForm::StressCharge;
    std::array<double,36> elasticityOrCompliance{};
    std::array<double,18> coupling{};
    std::array<double,9> permittivity{};
    double density = 0;
    bool piezoelectric = false;
    RStructuralPiezoMaterialSourceKind sourceKind = RStructuralPiezoMaterialSourceKind::Example;
    QString sourceId, sourceSha256, notes;
};
struct RStructuralPiezoLayeredModelLayer
{
    QString materialId;
    double lowerZ = 0, upperZ = 0, width = 1;
    QString portId; //!< Empty for a mechanical layer.
    int polarity = 1;
    std::array<double,4> coverage{{-1,1,-1,1}}; //!< Parent xi/eta rectangle.
};
struct RStructuralPiezoLayeredModelMember
{
    uint physicalElementId = 0;
    //! Solid3D: exactly one whole-volume material declaration. Active lowerZ/
    //! upperZ are electrode planes in frame coordinates, never added thickness.
    //! Passive solids use neutral lowerZ=0,upperZ=1,width=1 and full coverage.
    std::vector<RStructuralPiezoLayeredModelLayer> layers;
};
struct RStructuralPiezoLayeredPort
{
    RStructuralPiezoReadoutPort readout;
    double initialValue = 0; //!< Actuator voltage, open sensor fixed charge, otherwise zero.
};
struct RStructuralPiezoLayeredSupport
{
    uint physicalNodeId = 0;
    RStructuralPiezoPhysicalDof dof = RStructuralPiezoPhysicalDof::UX;
    double value = 0;
};
struct RStructuralPiezoLayeredNodeField
{
    uint physicalNodeId = 0;
    std::array<double,6> values{}; //!< Global translations/rotations or forces/moments, SI.
};
struct RStructuralPiezoLayeredNodeWorkpoint
{
    uint physicalNodeId = 0;
    std::array<double,6> q0{}, f0{};
};
struct RStructuralPiezoLayeredWorkpoint
{
    bool available = false;
    std::vector<RStructuralPiezoLayeredNodeWorkpoint> nodes;
    std::vector<RStructuralPiezoPortWorkpoint> ports;
};
//! Exactly one user damping authority; importedMatrix is row-major in supported generalized DOF order.
struct RStructuralPiezoLayeredDamping
{
    RStructuralPiezoDampingKind kind = RStructuralPiezoDampingKind::None;
    double rayleighAlpha = 0, rayleighBeta = 0, uniformRatio = 0;
    std::vector<double> ratios;
    std::vector<uint> clusterEnds; //!< Exclusive contiguous mode-cluster ends.
    uint importedDimension = 0; //!< Sorted physical-node IDs times 3 (beam/solid) or 5 (plate).
    std::vector<double> importedMatrix;
    QString sourceId, sourceSha256;
};
//! Persisted source declaration only. Numerical material admissibility, projected
//! kinematics and equilibrium are certified by the native solver, never this model.
struct RStructuralPiezoLayeredSetup
{
    bool enabled = false;
    RStructuralPiezoLayeredModelFamily family = RStructuralPiezoLayeredModelFamily::PlanarBeam;
    RStructuralPiezoLayeredFrame frame;
    QString sourceFingerprint;
    std::vector<RStructuralPiezoLayeredMaterial> materials;
    std::vector<RStructuralPiezoLayeredModelMember> members;
    std::vector<RStructuralPiezoLayeredPort> ports;
    std::vector<RStructuralPiezoLayeredSupport> supports;
    std::vector<RStructuralPiezoLayeredNodeField> backgroundForces, initialDisplacements, initialVelocities;
    RStructuralPiezoLayeredWorkpoint workpoint;
    RStructuralPiezoLayeredDamping damping;
    QByteArray excitationConfiguration, controllerConfiguration, designConfiguration;
    void validate() const;
    void validate(const RModel &model) const;
    bool hasNonDefaultData() const;
    QByteArray identityBytes() const;
    QString computeSourceFingerprint(const RModel &model) const;
    void bindToModel(const RModel &model);
};
//! Opaque accepted-state storage; the model makes no physical validity claim.
struct RStructuralPiezoLayeredRestart
{
    bool available = false;
    QString sourceFingerprint;
    QByteArray stateBytes;
    void validate() const;
};
#endif
