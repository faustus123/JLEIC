// $Id$
//
//    File: JEventSourceGenerator_vtx_mc.cc
// Created: Wed Jan  4 19:53:11 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#include <string>
using std::string;

#include "JEventSourceGenerator_vtx_mc.h"
#include "JFactoryGenerator_MCVertexTrackerHit.h"
using namespace jana;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddEventSourceGenerator(new JEventSourceGenerator_vtx_mc);
	app->AddFactoryGenerator(new JFactoryGenerator_MCVertexTrackerHit);
}
} // "C"


//---------------------------------
// Description
//---------------------------------
const char* JEventSourceGenerator_vtx_mc::Description(void)
{
	return "MC data from JLEIC vertex detector in ROOT format";
}

//---------------------------------
// CheckOpenable
//---------------------------------
double JEventSourceGenerator_vtx_mc::CheckOpenable(string source)
{
	// This should return a value between 0 and 1 inclusive
	// with 1 indicating it definitely can read events from
	// the specified source and 0 meaning it definitely can't.
	// Typically, this will just check the file suffix.
	
	// should try opening file and checking for appropriate
	// TTree, but for now just check if file contains ".root"
	if(source.find(".root") != string::npos) return 0.5;
	
	return 0.0; 
}

//---------------------------------
// MakeJEventSource
//---------------------------------
JEventSource* JEventSourceGenerator_vtx_mc::MakeJEventSource(string source)
{
	return new JEventSource_vtx_mc(source.c_str());
}

