/*******************************************************************************

Copyright (c) 2004 SensAble Technologies, Inc. All rights reserved.

OpenHaptics(TM) toolkit. The material embodied in this software and use of
this software is subject to the terms and conditions of the clickthrough
Development License Agreement.

For questions, comments or bug reports, go to forums at: 
    http://dsc.sensable.com
	
Module Name:

  ContactModel.c

Description:

  Defines the constants of the simulation

*******************************************************************************/

#ifndef ContactModel_H_
#define ContactModel_H_

#include <HDU/hduVector.h>

/*******************************************************************************
  Simple case of two spheres contact
 *******************************************************************************/
class ContactModel
{
 public:
  /* A contact model depends on two objects' relative positions,
     the fixed and the visitor. The constructor initializes radius for
     both spheres and assumes no contact */
  
  ContactModel( double fixedRadius,
		const hduVector3Dd fixed,
		double VisitorRadius,
		const hduVector3Dd visitor);
  
  /* Updates the end effector position and sets the new object position,
     and their relative displacement */
  void UpdateEffectorPosition(const hduVector3Dd visitor);
  
  /* Given current spatial configuration, calculate the force on visitor */
  hduVector3Dd GetCurrentForceOnVisitor(void);

  /* This is the center of the object, after it has interacted with contact
     model. If the visitor does not touch the fixed sphere, the new contact
     point is the projection of efefctor position on fixed sphere's surface */
  hduVector3Dd GetCurrentContactPoint(void);

 protected:
  hduVector3Dd m_fixedCenter;
  hduVector3Dd m_effectorPosition;
  hduVector3Dd m_visitorPosition; 
  
  hduVector3Dd m_centerToEffector;
  hduVector3Dd m_visitorToCenter;

  /* Force on effector */
  hduVector3Dd m_forceOnVisitor;
  double m_currentDistance;
  
  /* Effective position where contact is established. equals to sum of the
     two radii of spheres */
  double m_armsLength;
};

#endif /* ContactModel_H_ */

/************ E * O * F ***********/
