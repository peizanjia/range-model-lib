#ifndef RML_LINE_H
#define RML_LINE_H

#include <vector>

#include <rbl_r3vector.h>

#include "rml_element.h"
#include "rml_element_group.h"

//! Line element group class.
class RLine : public RElementGroup
{

    public:

        //! Default entity name.
        const static QString defaultName;

    private:

        //! Internal initialization function.
        void _init ( const RLine *pLine = nullptr );

    protected:

        //! Line artificial cross area.
        double crossArea;
        //! Second moment of area about the section local Y axis.
        double secondMomentY;
        //! Second moment of area about the section local Z axis.
        double secondMomentZ;
        //! Saint-Venant torsional constant.
        double torsionalConstant;
        //! Elastic section modulus about the section local Y axis.
        double sectionModulusY;
        //! Elastic section modulus about the section local Z axis.
        double sectionModulusZ;
        //! Torsional section modulus used for maximum Saint-Venant shear stress.
        double torsionalSectionModulus;
        //! Preferred section local Y direction in global coordinates.
        RR3Vector localYDirection;

    public:

        //! Constructor.
        RLine ();

        //! Copy constructor.
        RLine (const RLine &line);

        //! Destructor.
        ~RLine ();

        //! Return line group artificial cross area.
        double getCrossArea ( void ) const;

        //! Set line group artificial cross area.
        void setCrossArea ( double cross_area );

        double getSecondMomentY() const;
        void setSecondMomentY(double secondMomentY);

        double getSecondMomentZ() const;
        void setSecondMomentZ(double secondMomentZ);

        double getTorsionalConstant() const;
        void setTorsionalConstant(double torsionalConstant);

        double getSectionModulusY() const;
        void setSectionModulusY(double sectionModulusY);

        double getSectionModulusZ() const;
        void setSectionModulusZ(double sectionModulusZ);

        double getTorsionalSectionModulus() const;
        void setTorsionalSectionModulus(double torsionalSectionModulus);

        const RR3Vector &getLocalYDirection() const;
        void setLocalYDirection(const RR3Vector &localYDirection);

        //! Find line length.
        double findLength(const std::vector<RNode> &nodes, const std::vector<RElement> &elements) const;

        //! Assignment operator.
        RLine & operator = ( const RLine &line );

        //! Allow RFileIO to access private members.
        friend class RFileIO;

};

#endif /* RML_LINE_H */
