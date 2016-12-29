// $Id$
//
//    File: JEventSourceGenerator_CPPsim.h
// Created: Wed Dec 28 15:53:34 EST 2016
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _JEventSourceGenerator_CPPsim_
#define _JEventSourceGenerator_CPPsim_

#include <JANA/jerror.h>
#include <JANA/JEventSourceGenerator.h>

#include "JEventSource_CPPsim.h"

class JEventSourceGenerator_CPPsim: public jana::JEventSourceGenerator{
	public:
		JEventSourceGenerator_CPPsim(){}
		virtual ~JEventSourceGenerator_CPPsim(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "JEventSourceGenerator_CPPsim";}
		
		const char* Description(void);
		double CheckOpenable(string source);
		jana::JEventSource* MakeJEventSource(string source);
};

#endif // _JEventSourceGenerator_CPPsim_

