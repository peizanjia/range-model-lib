#ifndef RML_PROBLEM_SETUP_H
#define RML_PROBLEM_SETUP_H

#include "rml_acoustic_setup.h"
#include "rml_mesh_setup.h"
#include "rml_modal_setup.h"
#include "rml_parallel_execution_config.h"
#include "rml_radiation_setup.h"
#include "rml_structural_dynamics_setup.h"
#include "rml_structural_control_setup.h"
#include "rml_structural_dynamics_restart_data.h"
#include "rml_structural_interaction.h"
#include "rml_structural_cable_setup.h"
#include "rml_structural_piezo_setup.h"
#include "rml_structural_thermal_control_setup.h"
#include "rml_structural_truss_dynamics_setup.h"
#include "rml_structural_truss_dynamics_restart_data.h"
#include "rml_structural_native_control.h"

//! Structural mass-matrix assembly used by transient and modal solvers.
enum class RMassMatrixType
{
    Consistent = 0,
    Lumped
};

class RProblemSetup
{

    protected:

        //! Restart solver.
        bool restart;
        //! Radiation setup.
        RRadiationSetup radiationSetup;
        //! Modal setup.
        RModalSetup modalSetup;
        //! Mesh setup.
        RMeshSetup meshSetup;
        //! Acoustic setup.
        RAcousticSetup acousticSetup;
        //! Structural mass-matrix type.
        RMassMatrixType massMatrixType;
        //! Structural constraints, couplings, sensors and actuators.
        RStructuralInteractionSetup structuralInteractionSetup;
        //! Optional second-order structural transient setup.
        RStructuralDynamicsSetup structuralDynamicsSetup;
        //! Versioned controller configuration; dynamic state is in restart data.
        RStructuralControlSetup structuralControlSetup;
        //! Optional model-bound cable form-finding configuration.
        RStructuralCableSetup structuralCableSetup;
        //! Optional model-bound static terminal piezo configuration.
        RStructuralPiezoSetup structuralPiezoSetup;
        //! Optional model-bound multirate thermal-control configuration.
        RStructuralThermalControlSetup structuralThermalControlSetup;
        //! Last accepted second-order transient state for disk restart.
        RStructuralDynamicsRestartData structuralDynamicsRestartData;
        RStructuralTrussDynamicsSetup structuralTrussDynamicsSetup;
        RStructuralTrussDynamicsRestartData structuralTrussDynamicsRestartData;
        RStructuralNativeControlSpec structuralNativeControlSpec;
        RStructuralNativeControlState structuralNativeControlState;
        //! Solver execution policy.
        RParallelExecutionConfig parallelExecutionConfig;

    private:

        //! Internal initialization function.
        void _init(const RProblemSetup *pProblemSetup = nullptr);

    public:

        //! Constructor.
        RProblemSetup();

        //! Copy constructor.
        RProblemSetup(const RProblemSetup &problemSetup);

        //! Destructor.
        ~RProblemSetup();

        //! Assignment operator.
        RProblemSetup &operator =(const RProblemSetup &problemSetup);

        //! Return true if solver should be restarted.
        bool getRestart(void) const;

        //! Set wether the solver should be restarted.
        void setRestart(bool restart);

        //! Get const reference to radiation setup.
        const RRadiationSetup &getRadiationSetup(void) const;

        //! Get reference to radiation setup.
        RRadiationSetup &getRadiationSetup(void);

        //! Set radiation setup.
        void setRadiationSetup(const RRadiationSetup &radiationSetup);

        //! Get const reference to modal setup.
        const RModalSetup &getModalSetup(void) const;

        //! Get reference to modal setup.
        RModalSetup &getModalSetup(void);

        //! Set modal setup.
        void setModalSetup(const RModalSetup &modalSetup);

        //! Get const reference to mesh setup.
        const RMeshSetup &getMeshSetup(void) const;

        //! Get reference to mesh setup.
        RMeshSetup &getMeshSetup(void);

        //! Set mesh setup.
        void setMeshSetup(const RMeshSetup &meshSetup);

        //! Get const reference to acoustic setup.
        const RAcousticSetup &getAcousticSetup(void) const;

        //! Get reference to acoustic setup.
        RAcousticSetup &getAcousticSetup(void);

        //! Set acoustic setup.
        void setAcousticSetup(const RAcousticSetup &acousticSetup);

        //! Return structural mass-matrix type.
        RMassMatrixType getMassMatrixType(void) const;

        //! Set structural mass-matrix type.
        void setMassMatrixType(RMassMatrixType massMatrixType);

        const RStructuralInteractionSetup &getStructuralInteractionSetup() const;
        RStructuralInteractionSetup &getStructuralInteractionSetup();
        void setStructuralInteractionSetup(const RStructuralInteractionSetup &setup);

        const RStructuralDynamicsSetup &getStructuralDynamicsSetup() const;
        RStructuralDynamicsSetup &getStructuralDynamicsSetup();
        void setStructuralDynamicsSetup(const RStructuralDynamicsSetup &setup);

        const RStructuralControlSetup &getStructuralControlSetup() const;
        RStructuralControlSetup &getStructuralControlSetup();
        void setStructuralControlSetup(const RStructuralControlSetup &setup);

        const RStructuralCableSetup &getStructuralCableSetup() const;
        RStructuralCableSetup &getStructuralCableSetup();
        void setStructuralCableSetup(const RStructuralCableSetup &setup);

        const RStructuralPiezoSetup &getStructuralPiezoSetup() const;
        RStructuralPiezoSetup &getStructuralPiezoSetup();
        void setStructuralPiezoSetup(const RStructuralPiezoSetup &setup);

        const RStructuralThermalControlSetup &getStructuralThermalControlSetup() const;
        RStructuralThermalControlSetup &getStructuralThermalControlSetup();
        void setStructuralThermalControlSetup(const RStructuralThermalControlSetup &setup);

        const RStructuralDynamicsRestartData &getStructuralDynamicsRestartData() const;
        RStructuralDynamicsRestartData &getStructuralDynamicsRestartData();
        void setStructuralDynamicsRestartData(const RStructuralDynamicsRestartData &data);

        const RStructuralTrussDynamicsSetup &getStructuralTrussDynamicsSetup() const;
        RStructuralTrussDynamicsSetup &getStructuralTrussDynamicsSetup();
        void setStructuralTrussDynamicsSetup(const RStructuralTrussDynamicsSetup &setup);

        const RStructuralTrussDynamicsRestartData &getStructuralTrussDynamicsRestartData() const;
        RStructuralTrussDynamicsRestartData &getStructuralTrussDynamicsRestartData();
        void setStructuralTrussDynamicsRestartData(const RStructuralTrussDynamicsRestartData &data);

        const RStructuralNativeControlSpec &getStructuralNativeControlSpec() const;
        RStructuralNativeControlSpec &getStructuralNativeControlSpec();
        void setStructuralNativeControlSpec(const RStructuralNativeControlSpec &spec);

        const RStructuralNativeControlState &getStructuralNativeControlState() const;
        RStructuralNativeControlState &getStructuralNativeControlState();
        void setStructuralNativeControlState(const RStructuralNativeControlState &state);

        const RParallelExecutionConfig &getParallelExecutionConfig() const;
        RParallelExecutionConfig &getParallelExecutionConfig();
        void setParallelExecutionConfig(const RParallelExecutionConfig &config);

        //! Convert to printable string.
        QString toString() const;

        //! Allow RFileIO to access private members.
        friend class RFileIO;

};

#endif // RML_PROBLEM_SETUP_H
