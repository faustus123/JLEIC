// $Id$
//
//    File: JEventSourceGenerator_vtx_mc.h
// Created: Wed Jan  4 19:53:11 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _JEventSourceGenerator_vtx_mc_
#define _JEventSourceGenerator_vtx_mc_

#include <JANA/jerror.h>
#include <JANA/JEventSourceGenerator.h>

#include "JEventSource_vtx_mc.h"

class JEventSourceGenerator_vtx_mc: public jana::JEventSourceGenerator{
	public:
		JEventSourceGenerator_vtx_mc(){}
		virtual ~JEventSourceGenerator_vtx_mc(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "JEventSourceGenerator_vtx_mc";}
		
		const char* Description(void);
		double CheckOpenable(string source);
		jana::JEventSource* MakeJEventSource(string source);
};

#endif // _JEventSourceGenerator_vtx_mc_

