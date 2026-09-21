#ifndef RML_MESH_GENERATOR_H
#define RML_MESH_GENERATOR_H

#include "rml_mesh_input.h"
#include "rml_mesh_pipeline.h"
#include "rml_model.h"
#include "rml_triangle.h"
#include "rml_tetrahedron.h"

class RMeshGenerator
{

    private:

        //! Constructor.
        RMeshGenerator() {}

    public:

        //! Generate volume mesh.
        static void generate(const RMeshInput &meshInput, RModel &model);

        //! Generate volume mesh from triangulated surface.
        static void generate(const std::vector<RNode> &nodes, const std::vector<RElement> &elements, std::vector<RNode> &steinerNodes, std::vector<RElement> &volumeElements);

        //! Tetrahedralize triangulated surface.
        static std::vector<RTetrahedron> generate(const std::vector<RTriangle> &triangles);

        //! Return mesh backends available in this build.
        static QStringList getAvailableBackends();

        //! Generate a one-, two- or three-dimensional primitive mesh through
        //! the backend-neutral Stage 3 contract.
        static RMeshGenerationResult generate(const RMeshGenerationRequest &request);

        //! Replace a model with nodes and target-dimensional elements from a
        //! generated mesh. Named sets stay in the result and are consumed by
        //! the assignment-transfer layer instead of becoming physical elements.
        static void apply(const RMeshGenerationResult &result, RModel &model);

};

#endif // RML_MESH_GENERATOR_H
