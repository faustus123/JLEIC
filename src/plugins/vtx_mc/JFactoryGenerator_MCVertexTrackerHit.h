// $Id$
//
//    File: JFactoryGenerator_MCVertexTrackerHit.h
// Created: Wed Jan  4 21:21:40 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _JFactoryGenerator_MCVertexTrackerHit_
#define _JFactoryGenerator_MCVertexTrackerHit_

#include <JANA/jerror.h>
#include <JANA/JFactoryGenerator.h>

#include "MCVertexTrackerHit.h"

class JFactoryGenerator_MCVertexTrackerHit: public jana::JFactoryGenerator{
	public:
		JFactoryGenerator_MCVertexTrackerHit(){}
		virtual ~JFactoryGenerator_MCVertexTrackerHit(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "JFactoryGenerator_MCVertexTrackerHit";}
		
		jerror_t GenerateFactories(jana::JEventLoop *loop){
			loop->AddFactory(new jana::JFactory<MCVertexTrackerHit>());
			return NOERROR;
		}

};

#endif // _JFactoryGenerator_MCVertexTrackerHit_

