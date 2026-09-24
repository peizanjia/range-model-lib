#include <cmath>

#include <rbl_error.h>

#include "rml_structural_dynamics_setup.h"

namespace
{
void requireFiniteNonnegative(double value, const char *name)
{
    if (!std::isfinite(value) || value < 0.0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"%s must be finite and nonnegative.",name);
    }
}
}

bool RStructuralDynamicsSetup::getEnabled() const { return this->enabled; }
void RStructuralDynamicsSetup::setEnabled(bool enabled) { this->enabled = enabled; }

RStructuralTimeIntegrationMethod RStructuralDynamicsSetup::getMethod() const { return this->method; }
void RStructuralDynamicsSetup::setMethod(RStructuralTimeIntegrationMethod method)
{
    if (method != RStructuralTimeIntegrationMethod::Newmark &&
        method != RStructuralTimeIntegrationMethod::GeneralizedAlpha &&
        method != RStructuralTimeIntegrationMethod::CentralDifference)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Unsupported structural integration method (%d).",int(method));
    }
    this->method = method;
}

double RStructuralDynamicsSetup::getTimeStep() const { return this->timeStep; }
void RStructuralDynamicsSetup::setTimeStep(double timeStep)
{
    if (!std::isfinite(timeStep) || timeStep <= 0.0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Structural time step must be finite and positive.");
    }
    this->timeStep = timeStep;
}

uint RStructuralDynamicsSetup::getNSteps() const { return this->nSteps; }
void RStructuralDynamicsSetup::setNSteps(uint nSteps)
{
    if (nSteps == 0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Structural step count must be positive.");
    }
    this->nSteps = nSteps;
}

uint RStructuralDynamicsSetup::getOutputFrequency() const { return this->outputFrequency; }
void RStructuralDynamicsSetup::setOutputFrequency(uint outputFrequency)
{
    if (outputFrequency == 0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Structural output frequency must be positive.");
    }
    this->outputFrequency = outputFrequency;
}

double RStructuralDynamicsSetup::getRayleighAlpha() const { return this->rayleighAlpha; }
void RStructuralDynamicsSetup::setRayleighAlpha(double alpha)
{
    requireFiniteNonnegative(alpha,"Rayleigh mass coefficient");
    this->rayleighAlpha = alpha;
}

double RStructuralDynamicsSetup::getRayleighBeta() const { return this->rayleighBeta; }
void RStructuralDynamicsSetup::setRayleighBeta(double beta)
{
    requireFiniteNonnegative(beta,"Rayleigh stiffness coefficient");
    this->rayleighBeta = beta;
}

double RStructuralDynamicsSetup::getRhoInfinity() const { return this->rhoInfinity; }
void RStructuralDynamicsSetup::setRhoInfinity(double rhoInfinity)
{
    if (!std::isfinite(rhoInfinity) || rhoInfinity < 0.0 || rhoInfinity > 1.0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Generalized-alpha spectral radius must be within [0,1].");
    }
    this->rhoInfinity = rhoInfinity;
}

double RStructuralDynamicsSetup::getNewmarkGamma() const { return this->newmarkGamma; }
void RStructuralDynamicsSetup::setNewmarkGamma(double gamma)
{
    if (!std::isfinite(gamma) || gamma < 0.5)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Newmark gamma must be finite and at least 0.5.");
    }
    this->newmarkGamma = gamma;
}

double RStructuralDynamicsSetup::getNewmarkBeta() const { return this->newmarkBeta; }
void RStructuralDynamicsSetup::setNewmarkBeta(double beta)
{
    if (!std::isfinite(beta) || beta <= 0.0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Newmark beta must be finite and positive.");
    }
    this->newmarkBeta = beta;
}

const std::vector<double> &RStructuralDynamicsSetup::getModalDampingRatios() const
{
    return this->modalDampingRatios;
}

void RStructuralDynamicsSetup::setModalDampingRatios(const std::vector<double> &ratios)
{
    for (double ratio : ratios)
    {
        if (!std::isfinite(ratio) || ratio < 0.0 || ratio >= 1.0)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,"Modal damping ratios must be finite and within [0,1).");
        }
    }
    this->modalDampingRatios = ratios;
}

void RStructuralDynamicsSetup::validate() const
{
    if (this->method != RStructuralTimeIntegrationMethod::Newmark &&
        this->method != RStructuralTimeIntegrationMethod::GeneralizedAlpha &&
        this->method != RStructuralTimeIntegrationMethod::CentralDifference)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Unsupported structural integration method (%d).",int(this->method));
    }
    if (!std::isfinite(this->timeStep) || this->timeStep <= 0.0 || this->nSteps == 0 || this->outputFrequency == 0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Invalid structural transient time grid or output frequency.");
    }
    requireFiniteNonnegative(this->rayleighAlpha,"Rayleigh mass coefficient");
    requireFiniteNonnegative(this->rayleighBeta,"Rayleigh stiffness coefficient");
    if (!std::isfinite(this->rhoInfinity) || this->rhoInfinity < 0.0 || this->rhoInfinity > 1.0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Invalid generalized-alpha spectral radius.");
    }
    if (!std::isfinite(this->newmarkGamma) || this->newmarkGamma < 0.5 ||
        !std::isfinite(this->newmarkBeta) || this->newmarkBeta <= 0.0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Invalid Newmark integration parameters.");
    }
    for (double ratio : this->modalDampingRatios)
    {
        if (!std::isfinite(ratio) || ratio < 0.0 || ratio >= 1.0)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,"Invalid modal damping ratio.");
        }
    }
}

QString RStructuralDynamicsSetup::toString() const
{
    return QString("{ Enabled: %1, Method: %2, Time step: %3, Steps: %4, Output frequency: %5, Rayleigh: (%6, %7), Rho infinity: %8, Newmark: (%9, %10), Modal damping ratios: %11 }")
        .arg(this->enabled ? "True" : "False").arg(int(this->method))
        .arg(this->timeStep).arg(this->nSteps).arg(this->outputFrequency)
        .arg(this->rayleighAlpha).arg(this->rayleighBeta).arg(this->rhoInfinity)
        .arg(this->newmarkGamma).arg(this->newmarkBeta).arg(this->modalDampingRatios.size());
}
