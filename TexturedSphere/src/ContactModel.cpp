/*****************************************************************************      Copyright (c) 2004 SensAble Technologies, Inc. All rights reserved.

OpenHaptics(TM) toolkit. The material embodied in this software and use of
this software is subject to the terms and conditions of the clickthrough
Development License Agreement.

For questions, comments or bug reports, go to forums at: 
    http://dsc.sensable.com                

Module Name:

  ContactModel.cpp

Description:

  Defines the equation of the force field.

*******************************************************************************/

#include "Constants.h" 
#include "ContactModel.h"

/*******************************************************************************
  Constructor. The constructor initializes the center positions and assumes
  there is no contact between spheres. Initializes the radii for spheres.
*******************************************************************************/

ContactModel::ContactModel(double fixedRadius,
			   const hduVector3Dd fixed,
			   double visitorRadius,
			   const hduVector3Dd visitor)
{
  m_fixedCenter = fixed;
  
  // Intersection of two spheres = intersection of point and sphere of 
  // arm length equal to the sum of the two radii

  m_armsLength = fixedRadius + visitorRadius;
  UpdateEffectorPosition(visitor);
}

void ContactModel::UpdateEffectorPosition(const hduVector3Dd visitor)
{

  m_currentDistance = 0.0;
  m_effectorPosition = visitor;
  m_centerToEffector = m_effectorPosition - m_fixedCenter;
  
  m_currentDistance = m_centerToEffector.magnitude();
  if(m_currentDistance > m_armsLength)
    {
      m_visitorPosition = m_effectorPosition;
      m_forceOnVisitor.set(0.0, 0.0, 0.0);
    }
  else
    {
      // If near center, don't recalculate; center is a singular point
      if (m_currentDistance > EPSILON)
	{
	  double scale = m_armsLength/m_currentDistance;
	  // project effector point on efefctive sphere surface at arms L
	  m_visitorPosition = m_fixedCenter + scale*m_centerToEffector;

	  // Force is calulated from F=kx
	  // k = stiffness, x = vector magnitude and direction.
	  // k = STIFFNESS, x = penetration vector from currPos to desiredPos
	  m_forceOnVisitor = STIFFNESS*(m_visitorPosition-m_effectorPosition);
	}
    }
}

/*******************************************************************************
  Gets force on visitor particle, given current displacement
*******************************************************************************/
hduVector3Dd ContactModel::GetCurrentForceOnVisitor()
{
  return m_forceOnVisitor;
}

/*******************************************************************************
  Get current contact point, center of visitor sphere
*******************************************************************************/
hduVector3Dd ContactModel::GetCurrentContactPoint()
{
  return m_visitorPosition;
}

/****************** E * O * F *********************/
