// $Id$
//
//    File: JEventSourceGenerator_CPPsim.cc
// Created: Wed Dec 28 15:53:34 EST 2016
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#include <string>
using std::string;

#include <JANA/JApplication.h>
#include "JEventSourceGenerator_CPPsim.h"
using namespace jana;

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddEventSourceGenerator( new JEventSourceGenerator_CPPsim());
}
} // "C"
//---------------------------------
// Description
//---------------------------------
const char* JEventSourceGenerator_CPPsim::Description(void)
{
	return "CPPsim";
}

//---------------------------------
// CheckOpenable
//---------------------------------
double JEventSourceGenerator_CPPsim::CheckOpenable(string source)
{
	// This should return a value between 0 and 1 inclusive
	// with 1 indicating it definitely can read events from
	// the specified source and 0 meaning it definitely can't.
	// Typically, this will just check the file suffix.

	return 0.1;
}

//---------------------------------
// MakeJEventSource
//---------------------------------
JEventSource* JEventSourceGenerator_CPPsim::MakeJEventSource(string source)
{
	return new JEventSource_CPPsim(source.c_str());
}
