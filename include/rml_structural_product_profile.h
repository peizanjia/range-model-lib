#ifndef RML_STRUCTURAL_PRODUCT_PROFILE_H
#define RML_STRUCTURAL_PRODUCT_PROFILE_H

#include <vector>

#include <QString>

#include "rml_boundary_condition.h"
#include "rml_element_capability.h"
#include "rml_environment_condition.h"
#include "rml_initial_condition.h"
#include "rml_material_property.h"
#include "rml_problem_capability.h"
#include "rml_variable.h"

class RModel;

//! Result of checking a model against a product profile.
class RProductProfileValidation
{

    private:

        //! Problem types which are outside the product profile.
        RProblemTypeMask unsupportedProblemTypeMask;
        //! Element types without a structural formulation.
        std::vector<RElementType> unsupportedElementTypes;
        //! True if the model requests the unverified legacy time solver.
        bool unsupportedTimeSolver;
        //! Human-readable rejection reason. Empty for an accepted model.
        QString diagnostic;

    public:

        //! Constructor.
        RProductProfileValidation(RProblemTypeMask unsupportedProblemTypeMask = R_PROBLEM_NONE,
                                  const std::vector<RElementType> &unsupportedElementTypes = std::vector<RElementType>(),
                                  bool unsupportedTimeSolver = false,
                                  const QString &diagnostic = QString());

        //! Return true if the model is accepted by the profile.
        bool isAccepted() const;

        //! Return problem types which are outside the profile.
        RProblemTypeMask getUnsupportedProblemTypeMask() const;

        //! Return element types without a structural formulation.
        const std::vector<RElementType> &getUnsupportedElementTypes() const;

        //! Return true if the model requests an unsupported time solver.
        bool getUnsupportedTimeSolver() const;

        //! Return the human-readable rejection reason.
        const QString &getDiagnostic() const;

};

//! Structural-only product policy shared by the GUI and solver executable.
class RStructuralProductProfile
{

    public:

        //! Return the stable profile identifier.
        static QString getId();

        //! Return the display name.
        static QString getName();

        //! Return problem types exposed by the structural product.
        static RProblemTypeMask getEnabledProblemTypeMask();

        //! Return problem types exposed by the structural product.
        static std::vector<RProblemType> getEnabledProblemTypes();

        //! Return capability evidence for a registered problem type.
        static RProblemCapability getProblemCapability(RProblemType problemType);

        //! Return capability evidence for an element type.
        static RElementCapability getElementCapability(RElementType elementType);

        //! Return true if a problem type is exposed by the product.
        static bool isProblemTypeEnabled(RProblemType problemType);

        //! Return true if an element can be submitted to the structural solver.
        static bool isElementTypeEnabled(RElementType elementType);

        //! Return true if a boundary condition belongs to structural analysis.
        static bool isBoundaryConditionEnabled(RBoundaryConditionType type);

        //! Return true if an initial condition is enabled by the product.
        static bool isInitialConditionEnabled(RInitialConditionType type);

        //! Return true if an environment condition belongs to structural analysis.
        static bool isEnvironmentConditionEnabled(REnvironmentConditionType type);

        //! Return true if a material property belongs to structural analysis.
        static bool isMaterialPropertyEnabled(RMaterialProperty::Type type);

        //! Return true if a result variable may be presented by the product.
        static bool isResultVariableEnabled(RVariableType type);

        //! Return whether the legacy time solver is exposed by this product.
        static bool isTimeSolverEnabled();

        //! Validate a model problem mask against the structural product.
        static RProductProfileValidation validate(RProblemTypeMask problemTypeMask);

        //! Validate the analysis and element types of a model.
        static RProductProfileValidation validate(const RModel &model);

};

#endif // RML_STRUCTURAL_PRODUCT_PROFILE_H
