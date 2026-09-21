#include <rbl_error.h>

#include "rml_element.h"
#include "rml_line.h"

const QString RLine::defaultName("Line");

RLine::RLine ()
    : crossArea(0.0)
    , secondMomentY(0.0)
    , secondMomentZ(0.0)
    , torsionalConstant(0.0)
    , sectionModulusY(0.0)
    , sectionModulusZ(0.0)
    , torsionalSectionModulus(0.0)
    , localYDirection(0.0,1.0,0.0)
{
    this->name = RLine::defaultName;
    this->_init ();
} /* RLine::RLine */


RLine::RLine (const RLine &line) : RElementGroup (line)
{
    this->_init (&line);
} /* RLine::RLine */


RLine::~RLine ()
{
} /* RLine::~RLine */


void RLine::_init (const RLine *pLine)
{
    if (pLine)
    {
        this->setCrossArea (pLine->getCrossArea());
        this->setSecondMomentY(pLine->getSecondMomentY());
        this->setSecondMomentZ(pLine->getSecondMomentZ());
        this->setTorsionalConstant(pLine->getTorsionalConstant());
        this->setSectionModulusY(pLine->getSectionModulusY());
        this->setSectionModulusZ(pLine->getSectionModulusZ());
        this->setTorsionalSectionModulus(pLine->getTorsionalSectionModulus());
        this->setLocalYDirection(pLine->getLocalYDirection());
    }
} /* RLine::_init */


double RLine::getCrossArea (void) const
{
    return this->crossArea;
} /* RLine::getCrossArea */


void RLine::setCrossArea (double crossArea)
{
    this->crossArea = crossArea;
} /* RLine::setCrossArea */


double RLine::getSecondMomentY() const
{
    return this->secondMomentY;
}


void RLine::setSecondMomentY(double secondMomentY)
{
    if (secondMomentY < 0.0)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Second moment Iy cannot be negative.");
    }
    this->secondMomentY = secondMomentY;
}


double RLine::getSecondMomentZ() const
{
    return this->secondMomentZ;
}


void RLine::setSecondMomentZ(double secondMomentZ)
{
    if (secondMomentZ < 0.0)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Second moment Iz cannot be negative.");
    }
    this->secondMomentZ = secondMomentZ;
}


double RLine::getTorsionalConstant() const
{
    return this->torsionalConstant;
}


void RLine::setTorsionalConstant(double torsionalConstant)
{
    if (torsionalConstant < 0.0)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Torsional constant J cannot be negative.");
    }
    this->torsionalConstant = torsionalConstant;
}


double RLine::getSectionModulusY() const
{
    return this->sectionModulusY;
}


void RLine::setSectionModulusY(double sectionModulusY)
{
    if (sectionModulusY < 0.0)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Section modulus Wy cannot be negative.");
    }
    this->sectionModulusY = sectionModulusY;
}


double RLine::getSectionModulusZ() const
{
    return this->sectionModulusZ;
}


void RLine::setSectionModulusZ(double sectionModulusZ)
{
    if (sectionModulusZ < 0.0)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Section modulus Wz cannot be negative.");
    }
    this->sectionModulusZ = sectionModulusZ;
}


double RLine::getTorsionalSectionModulus() const
{
    return this->torsionalSectionModulus;
}


void RLine::setTorsionalSectionModulus(double torsionalSectionModulus)
{
    if (torsionalSectionModulus < 0.0)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Torsional section modulus Wt cannot be negative.");
    }
    this->torsionalSectionModulus = torsionalSectionModulus;
}


const RR3Vector &RLine::getLocalYDirection() const
{
    return this->localYDirection;
}


void RLine::setLocalYDirection(const RR3Vector &localYDirection)
{
    RR3Vector direction(localYDirection);
    if (direction.normalize() <= 0.0)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Beam local Y direction cannot be zero.");
    }
    this->localYDirection = direction;
}


double RLine::findLength(const std::vector<RNode> &nodes, const std::vector<RElement> &elements) const
{
    double lineLength = 0.0;

    for (uint i=0;i<this->size();i++)
    {
        const RElement &rElement = elements[this->get(i)];

        double elementLength = 0.0;
        if (rElement.findLength(nodes,elementLength))
        {
            lineLength += elementLength;
        }
    }

    return lineLength;
} /* RLine::findLength */


RLine & RLine::operator = (const RLine &line)
{
    this->RElementGroup::operator = (line);
    this->_init (&line);
    return (*this);
} /* RLine::operator = */

