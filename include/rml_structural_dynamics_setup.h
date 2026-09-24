#ifndef RML_STRUCTURAL_DYNAMICS_SETUP_H
#define RML_STRUCTURAL_DYNAMICS_SETUP_H

#include <QString>
#include <vector>

//! Second-order structural time-integration scheme.
enum class RStructuralTimeIntegrationMethod
{
    Newmark = 0,
    GeneralizedAlpha = 1,
    CentralDifference = 2
};

//! Persisted, solver-independent transient dynamics configuration.
class RStructuralDynamicsSetup
{
    public:
        RStructuralDynamicsSetup() = default;

        bool getEnabled() const;
        void setEnabled(bool enabled);
        RStructuralTimeIntegrationMethod getMethod() const;
        void setMethod(RStructuralTimeIntegrationMethod method);
        double getTimeStep() const;
        void setTimeStep(double timeStep);
        uint getNSteps() const;
        void setNSteps(uint nSteps);
        uint getOutputFrequency() const;
        void setOutputFrequency(uint outputFrequency);
        double getRayleighAlpha() const;
        void setRayleighAlpha(double alpha);
        double getRayleighBeta() const;
        void setRayleighBeta(double beta);
        //! Generalized-alpha high-frequency spectral radius in [0,1].
        double getRhoInfinity() const;
        void setRhoInfinity(double rhoInfinity);
        double getNewmarkGamma() const;
        void setNewmarkGamma(double gamma);
        double getNewmarkBeta() const;
        void setNewmarkBeta(double beta);
        //! Optional per-mode damping ratios, ordered by extracted mode index.
        const std::vector<double> &getModalDampingRatios() const;
        void setModalDampingRatios(const std::vector<double> &ratios);

        void validate() const;
        QString toString() const;

    private:
        bool enabled = false;
        RStructuralTimeIntegrationMethod method = RStructuralTimeIntegrationMethod::Newmark;
        double timeStep = 0.01;
        uint nSteps = 1;
        uint outputFrequency = 1;
        double rayleighAlpha = 0.0;
        double rayleighBeta = 0.0;
        double rhoInfinity = 1.0;
        double newmarkGamma = 0.5;
        double newmarkBeta = 0.25;
        std::vector<double> modalDampingRatios;
};

#endif // RML_STRUCTURAL_DYNAMICS_SETUP_H
