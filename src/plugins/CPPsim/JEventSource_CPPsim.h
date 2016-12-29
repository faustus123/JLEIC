// $Id$
//
//    File: JEventSource_CPPsim.h
// Created: Wed Dec 28 15:53:34 EST 2016
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _JEventSource_CPPsim_
#define _JEventSource_CPPsim_

#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

#include "G4UIExecutive.hh"

#include <JANA/jerror.h>
#include <JANA/JEventSource.h>
#include <JANA/JEvent.h>

class JEventSource_CPPsim: public jana::JEventSource{
	public:
		JEventSource_CPPsim(const char* source_name);
		virtual ~JEventSource_CPPsim();
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "JEventSource_CPPsim";}

		jerror_t GetEvent(jana::JEvent &event);
		    void FreeEvent(jana::JEvent &event);
		jerror_t GetObjects(jana::JEvent &event, jana::JFactory_base *factory);

		    void AdjustPhysicsProcesses(G4UImanager* UImanager);
		    void ReadControl_in(void);
		    void PrintConfigInfo(void);

		 static string gGDMLfilename;
		 static unsigned int MAXEVENTS;
		 static string INPUT_FILENAME;
		 static string OUTPUT_FILENAME;
		 static string BFIELD_MAP;
		 vector<string> GEANT4_MACROS;
		 static bool MODE_MULS;
		 static bool MODE_DCAY;
		 static bool MODE_HADR;
		 static int MODE_ELOSS;
		 static int NG4THREADS;
		 static int RNDM;
		 static int RUN_NUMBER;
		 static bool GEOMOPT;
		 static vector<double> KINE_VALS;
		 static vector<double> SCAP_VALS;

		 G4UIExecutive* ui;
		 G4MTRunManager* runManager;
		 G4UImanager* UImanager;
};

#endif // _JEventSourceGenerator_CPPsim_
