#include <rbl_error.h>

#include "rml_problem_setup.h"

void RProblemSetup::_init(const RProblemSetup *pProblemSetup)
{
    if (pProblemSetup)
    {
        this->restart = pProblemSetup->restart;
        this->radiationSetup = pProblemSetup->radiationSetup;
        this->modalSetup = pProblemSetup->modalSetup;
        this->meshSetup = pProblemSetup->meshSetup;
        this->acousticSetup = pProblemSetup->acousticSetup;
        this->massMatrixType = pProblemSetup->massMatrixType;
        this->structuralInteractionSetup = pProblemSetup->structuralInteractionSetup;
        this->structuralDynamicsSetup = pProblemSetup->structuralDynamicsSetup;
        this->structuralControlSetup = pProblemSetup->structuralControlSetup;
        this->structuralCableSetup = pProblemSetup->structuralCableSetup;
        this->structuralPiezoSetup = pProblemSetup->structuralPiezoSetup;
        this->structuralThermalControlSetup = pProblemSetup->structuralThermalControlSetup;
        this->structuralDynamicsRestartData = pProblemSetup->structuralDynamicsRestartData;
        this->structuralTrussDynamicsSetup = pProblemSetup->structuralTrussDynamicsSetup;
        this->structuralTrussDynamicsRestartData = pProblemSetup->structuralTrussDynamicsRestartData;
        this->structuralNativeControlSpec = pProblemSetup->structuralNativeControlSpec;
        this->structuralNativeControlState = pProblemSetup->structuralNativeControlState;
        this->structuralPiezoReadoutSetup = pProblemSetup->structuralPiezoReadoutSetup;
        this->structuralPiezoLayeredSetup = pProblemSetup->structuralPiezoLayeredSetup;
        this->structuralPiezoLayeredRestart = pProblemSetup->structuralPiezoLayeredRestart;
        this->structuralPiezoAxialSetup = pProblemSetup->structuralPiezoAxialSetup;
        this->structuralPiezoAxialRestart = pProblemSetup->structuralPiezoAxialRestart;
        this->parallelExecutionConfig = pProblemSetup->parallelExecutionConfig;
    }
}

RProblemSetup::RProblemSetup()
    : restart(false)
    , massMatrixType(RMassMatrixType::Consistent)
{
    this->_init();
}

RProblemSetup::RProblemSetup(const RProblemSetup &problemSetup)
{
    this->_init(&problemSetup);
}

RProblemSetup::~RProblemSetup()
{

}

RProblemSetup &RProblemSetup::operator =(const RProblemSetup &problemSetup)
{
    this->_init(&problemSetup);
    return (*this);
}

bool RProblemSetup::getRestart(void) const
{
    return this->restart;
}

void RProblemSetup::setRestart(bool restart)
{
    this->restart = restart;
}

const RRadiationSetup &RProblemSetup::getRadiationSetup(void) const
{
    return this->radiationSetup;
}

RRadiationSetup &RProblemSetup::getRadiationSetup(void)
{
    return this->radiationSetup;
}

void RProblemSetup::setRadiationSetup(const RRadiationSetup &radiationSetup)
{
    this->radiationSetup = radiationSetup;
}

const RModalSetup &RProblemSetup::getModalSetup(void) const
{
    return this->modalSetup;
}

RModalSetup &RProblemSetup::getModalSetup(void)
{
    return this->modalSetup;
}

void RProblemSetup::setModalSetup(const RModalSetup &modalSetup)
{
    this->modalSetup = modalSetup;
}

const RMeshSetup &RProblemSetup::getMeshSetup() const
{
    return this->meshSetup;
}

RMeshSetup &RProblemSetup::getMeshSetup()
{
    return this->meshSetup;
}

void RProblemSetup::setMeshSetup(const RMeshSetup &meshSetup)
{
    this->meshSetup = meshSetup;
}

const RAcousticSetup &RProblemSetup::getAcousticSetup() const
{
    return this->acousticSetup;
}

RAcousticSetup &RProblemSetup::getAcousticSetup()
{
    return this->acousticSetup;
}

void RProblemSetup::setAcousticSetup(const RAcousticSetup &acousticSetup)
{
    this->acousticSetup = acousticSetup;
}

RMassMatrixType RProblemSetup::getMassMatrixType() const
{
    return this->massMatrixType;
}

void RProblemSetup::setMassMatrixType(RMassMatrixType massMatrixType)
{
    if (massMatrixType != RMassMatrixType::Consistent &&
        massMatrixType != RMassMatrixType::Lumped)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Unsupported structural mass-matrix type (%d).",
                     int(massMatrixType));
    }
    this->massMatrixType = massMatrixType;
}

const RStructuralInteractionSetup &RProblemSetup::getStructuralInteractionSetup() const
{
    return this->structuralInteractionSetup;
}

RStructuralInteractionSetup &RProblemSetup::getStructuralInteractionSetup()
{
    return this->structuralInteractionSetup;
}

void RProblemSetup::setStructuralInteractionSetup(const RStructuralInteractionSetup &setup)
{
    this->structuralInteractionSetup = setup;
}

const RStructuralDynamicsSetup &RProblemSetup::getStructuralDynamicsSetup() const
{
    return this->structuralDynamicsSetup;
}

RStructuralDynamicsSetup &RProblemSetup::getStructuralDynamicsSetup()
{
    return this->structuralDynamicsSetup;
}

void RProblemSetup::setStructuralDynamicsSetup(const RStructuralDynamicsSetup &setup)
{
    setup.validate();
    this->structuralDynamicsSetup = setup;
}

const RStructuralControlSetup &RProblemSetup::getStructuralControlSetup() const
{
    return this->structuralControlSetup;
}

RStructuralControlSetup &RProblemSetup::getStructuralControlSetup()
{
    return this->structuralControlSetup;
}

void RProblemSetup::setStructuralControlSetup(const RStructuralControlSetup &setup)
{
    setup.validate();
    this->structuralControlSetup = setup;
}

const RStructuralCableSetup &RProblemSetup::getStructuralCableSetup() const
{
    return this->structuralCableSetup;
}

RStructuralCableSetup &RProblemSetup::getStructuralCableSetup()
{
    return this->structuralCableSetup;
}

void RProblemSetup::setStructuralCableSetup(const RStructuralCableSetup &setup)
{
    setup.validate();
    this->structuralCableSetup = setup;
}

const RStructuralPiezoSetup &RProblemSetup::getStructuralPiezoSetup() const
{
    return this->structuralPiezoSetup;
}

RStructuralPiezoSetup &RProblemSetup::getStructuralPiezoSetup()
{
    return this->structuralPiezoSetup;
}

void RProblemSetup::setStructuralPiezoSetup(const RStructuralPiezoSetup &setup)
{
    setup.validate();
    this->structuralPiezoSetup = setup;
}

const RStructuralThermalControlSetup &RProblemSetup::getStructuralThermalControlSetup() const
{
    return this->structuralThermalControlSetup;
}

RStructuralThermalControlSetup &RProblemSetup::getStructuralThermalControlSetup()
{
    return this->structuralThermalControlSetup;
}

void RProblemSetup::setStructuralThermalControlSetup(const RStructuralThermalControlSetup &setup)
{
    setup.validate();
    this->structuralThermalControlSetup = setup;
}

const RStructuralDynamicsRestartData &RProblemSetup::getStructuralDynamicsRestartData() const
{
    return this->structuralDynamicsRestartData;
}

RStructuralDynamicsRestartData &RProblemSetup::getStructuralDynamicsRestartData()
{
    return this->structuralDynamicsRestartData;
}

void RProblemSetup::setStructuralDynamicsRestartData(const RStructuralDynamicsRestartData &data)
{
    data.validate();
    this->structuralDynamicsRestartData = data;
}

const RStructuralTrussDynamicsSetup &RProblemSetup::getStructuralTrussDynamicsSetup() const
{
    return this->structuralTrussDynamicsSetup;
}

RStructuralTrussDynamicsSetup &RProblemSetup::getStructuralTrussDynamicsSetup()
{
    return this->structuralTrussDynamicsSetup;
}

void RProblemSetup::setStructuralTrussDynamicsSetup(const RStructuralTrussDynamicsSetup &data)
{
    data.validate();
    this->structuralTrussDynamicsSetup = data;
}

const RStructuralTrussDynamicsRestartData &RProblemSetup::getStructuralTrussDynamicsRestartData() const
{
    return this->structuralTrussDynamicsRestartData;
}

RStructuralTrussDynamicsRestartData &RProblemSetup::getStructuralTrussDynamicsRestartData()
{
    return this->structuralTrussDynamicsRestartData;
}

void RProblemSetup::setStructuralTrussDynamicsRestartData(const RStructuralTrussDynamicsRestartData &data)
{
    data.validate();
    this->structuralTrussDynamicsRestartData = data;
}

const RStructuralNativeControlSpec &RProblemSetup::getStructuralNativeControlSpec() const { return structuralNativeControlSpec; }
RStructuralNativeControlSpec &RProblemSetup::getStructuralNativeControlSpec() { return structuralNativeControlSpec; }
void RProblemSetup::setStructuralNativeControlSpec(const RStructuralNativeControlSpec &spec) { spec.validate(); structuralNativeControlSpec=spec; }
const RStructuralNativeControlState &RProblemSetup::getStructuralNativeControlState() const { return structuralNativeControlState; }
RStructuralNativeControlState &RProblemSetup::getStructuralNativeControlState() { return structuralNativeControlState; }
void RProblemSetup::setStructuralNativeControlState(const RStructuralNativeControlState &state) { state.validate(); structuralNativeControlState=state; }

const RParallelExecutionConfig &RProblemSetup::getParallelExecutionConfig() const
{
    return this->parallelExecutionConfig;
}

RParallelExecutionConfig &RProblemSetup::getParallelExecutionConfig()
{
    return this->parallelExecutionConfig;
}

void RProblemSetup::setParallelExecutionConfig(const RParallelExecutionConfig &config)
{
    config.validate();
    this->parallelExecutionConfig = config;
}

QString RProblemSetup::toString() const
{
    return "{ Restart: " + QString(this->restart?"True":"False")
            + ", Radiation setup: " + this->radiationSetup.toString()
            + ", Modal setup: " + this->modalSetup.toString()
            + ", Mesh setup: " + this->meshSetup.toString()
            + ", Acoustic setup: " + this->acousticSetup.toString()
            + ", Mass matrix: "
            + QString(this->massMatrixType == RMassMatrixType::Lumped ? "Lumped" : "Consistent")
            + ", Structural constraints: " + QString::number(this->structuralInteractionSetup.getLinearConstraints().size())
            + ", Couplings: " + QString::number(this->structuralInteractionSetup.getCouplings().size())
            + ", Sensors: " + QString::number(this->structuralInteractionSetup.getSensors().size())
            + ", Actuators: " + QString::number(this->structuralInteractionSetup.getActuators().size())
            + ", Structural dynamics: " + this->structuralDynamicsSetup.toString()
            + ", Thermal control: " + QString(this->structuralThermalControlSetup.enabled ? "Enabled" : "Disabled")
            + ", Structural restart: " + QString(this->structuralDynamicsRestartData.available ? "Available" : "None")
            + ", Parallel execution: " + this->parallelExecutionConfig.toString()
            + " }";
}

const RStructuralPiezoReadoutSetup &RProblemSetup::getStructuralPiezoReadoutSetup() const
{
    return this->structuralPiezoReadoutSetup;
}
RStructuralPiezoReadoutSetup &RProblemSetup::getStructuralPiezoReadoutSetup()
{
    return this->structuralPiezoReadoutSetup;
}
void RProblemSetup::setStructuralPiezoReadoutSetup(const RStructuralPiezoReadoutSetup &setup)
{
    this->structuralPiezoReadoutSetup = setup;
}

const RStructuralPiezoLayeredSetup &RProblemSetup::getStructuralPiezoLayeredSetup() const { return structuralPiezoLayeredSetup; }
RStructuralPiezoLayeredSetup &RProblemSetup::getStructuralPiezoLayeredSetup() { return structuralPiezoLayeredSetup; }
void RProblemSetup::setStructuralPiezoLayeredSetup(const RStructuralPiezoLayeredSetup &setup) { setup.validate(); structuralPiezoLayeredSetup=setup; }
const RStructuralPiezoLayeredRestart &RProblemSetup::getStructuralPiezoLayeredRestart() const { return structuralPiezoLayeredRestart; }
RStructuralPiezoLayeredRestart &RProblemSetup::getStructuralPiezoLayeredRestart() { return structuralPiezoLayeredRestart; }
void RProblemSetup::setStructuralPiezoLayeredRestart(const RStructuralPiezoLayeredRestart &state) { state.validate(); structuralPiezoLayeredRestart=state; }

const RStructuralPiezoAxialSetup &RProblemSetup::getStructuralPiezoAxialSetup() const { return structuralPiezoAxialSetup; }
RStructuralPiezoAxialSetup &RProblemSetup::getStructuralPiezoAxialSetup() { return structuralPiezoAxialSetup; }
void RProblemSetup::setStructuralPiezoAxialSetup(const RStructuralPiezoAxialSetup &setup) { setup.validate(); structuralPiezoAxialSetup=setup; }
const RStructuralPiezoAxialRestart &RProblemSetup::getStructuralPiezoAxialRestart() const { return structuralPiezoAxialRestart; }
RStructuralPiezoAxialRestart &RProblemSetup::getStructuralPiezoAxialRestart() { return structuralPiezoAxialRestart; }
void RProblemSetup::setStructuralPiezoAxialRestart(const RStructuralPiezoAxialRestart &state) { state.validate(); structuralPiezoAxialRestart=state; }
