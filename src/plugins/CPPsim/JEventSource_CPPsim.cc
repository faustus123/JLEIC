// $Id$
//
//    File: JEventSource_CPPsim.cc
// Created: Wed Dec 28 15:53:34 EST 2016
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#include <unistd.h>
#include <signal.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
using namespace std;

#include "G4HadronicProcessStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4TransportationManager.hh"

#include "QGSP_FTFP_BERT.hh"

#include "CPPUserActionInitialization.hh"
#include "CPPDetectorConstruction.hh"


#include <EParticle.h>


#include <JANA/JApplication.h>
using namespace jana;

#include "JEventSource_CPPsim.h"
using namespace jana;

string JEventSource_CPPsim::gGDMLfilename = "";
unsigned int JEventSource_CPPsim::MAXEVENTS = -1;
string JEventSource_CPPsim::INPUT_FILENAME = "";
string JEventSource_CPPsim::OUTPUT_FILENAME = "CPPsim.root";
string JEventSource_CPPsim::BFIELD_MAP = "default";
bool JEventSource_CPPsim::MODE_MULS = true;
bool JEventSource_CPPsim::MODE_DCAY = true;
bool JEventSource_CPPsim::MODE_HADR = true;
int JEventSource_CPPsim::MODE_ELOSS = 1;
int JEventSource_CPPsim::NG4THREADS = 1;
int JEventSource_CPPsim::RNDM = 123;
int JEventSource_CPPsim::RUN_NUMBER = 2;
bool JEventSource_CPPsim::GEOMOPT = true;
vector<double> JEventSource_CPPsim::KINE_VALS;
vector<double> JEventSource_CPPsim::SCAP_VALS;

//----------------
// Constructor
//----------------
JEventSource_CPPsim::JEventSource_CPPsim(const char* source_name):JEventSource(source_name)
{
_DBG__;
	gPARMS->SetDefaultParameter("GDMLFILENAME", gGDMLfilename, "Name of GDML geometery file to use.");
	gPARMS->SetDefaultParameter("MULS", MODE_MULS, "Set to 0 to turn off multiple scattering");
	gPARMS->SetDefaultParameter("DCAY", MODE_DCAY, "Set to 0 to turn off particle decays");
	gPARMS->SetDefaultParameter("HADR", MODE_HADR, "Set to 0 to turn off hadronic interactions");
	gPARMS->SetDefaultParameter("ELOSS", MODE_ELOSS, "Set to 0 to turn off energy loss");
	gPARMS->SetDefaultParameter("NG4THREADS", NG4THREADS, "Number of threads for G4 to use");
	gPARMS->SetDefaultParameter("RNDM", RNDM, "Seed for random number generator");
	gPARMS->SetDefaultParameter("GEOMOPT", GEOMOPT, "Set to 0 to turn off geometry optimization (results in faster startup but slower running)");

_DBG__;
	// Read the control.in file
	ReadControl_in();
_DBG__;
	// Confirm existence of all specified macros
	bool all_macros_exist = true;
	for(unsigned int i=0; i<GEANT4_MACROS.size(); i++){
		ifstream ifs(GEANT4_MACROS[i].c_str());
		if(!ifs.is_open()){
			G4cout << "XXXX    Macro file \"" << GEANT4_MACROS[i] << "\" does not exist !   XXXX" << endl;
			all_macros_exist = false;
		}
	}
	if(!all_macros_exist) exit(-1);

	// Get command line arguments from JApplication and cast into
	// form that can be passed into G4UIExecutive constructor
	vector<string> args = japp->GetArgs();
	int argc = args.size();
	vector<char*> argv;
	for(auto arg : args) argv.push_back((char*)arg.c_str());
	ui = new G4UIExecutive(argc, &argv[0]);

	// Create G4RunManager and use it to initialize GEANT4
	runManager = new G4MTRunManager;
	runManager->SetNumberOfThreads(NG4THREADS);
	runManager->SetGeometryToBeOptimized(GEOMOPT);
	runManager->SetUserInitialization(new CPPDetectorConstruction(gGDMLfilename));
	runManager->SetUserInitialization(new QGSP_FTFP_BERT(0));
	runManager->SetUserInitialization(new CPPUserActionInitialization);

#ifdef G4VIS_USE
	G4VisManager* visManager = new G4VisExecutive;
	visManager->Initialize();
#endif

	// Suppress printing the hadronic interactions list.
	// (this trick obtained in response to bug report 1759
	// http://bugzilla-geant4.kek.jp/show_bug.cgi?id=1759)
	G4HadronicProcessStore::Instance()->SetVerbose(0);

	UImanager = G4UImanager::GetUIpointer();

	runManager->Initialize();

	// Set up user interface and graphical user interface

	// Adjust physics processes
	AdjustPhysicsProcesses(UImanager);

	// Print selected config. info
	PrintConfigInfo();

	// Run all macros
	for(unsigned int i=0; i<GEANT4_MACROS.size(); i++){
		string &macro = GEANT4_MACROS[i];
		G4cout << endl << "---- Running macro \"" << macro << "\" ... ----" << endl;
		UImanager->ApplyCommand(("/control/execute " + macro).c_str());
	}

	// Optionally start interactive mode
	if(MAXEVENTS>0){
		char cmd[256];
		sprintf(cmd,"/run/beamOn %d", MAXEVENTS);
		G4cout << "===== Starting event processing =====" << endl;
		G4cout << cmd << endl;
		UImanager->ApplyCommand(cmd);
	}

}

//----------------
// Destructor
//----------------
JEventSource_CPPsim::~JEventSource_CPPsim()
{
	// Tell user were done
	G4cout << endl << "------- Done -------" << endl;

	if(ui) delete ui;

#ifdef G4VIS_USE
	if(visManager) delete visManager;
#endif

	if(runManager) delete runManager;
}

//----------------
// GetEvent
//----------------
jerror_t JEventSource_CPPsim::GetEvent(JEvent &event)
{
	// Read an event from the source and copy the vital info into
	// the JEvent structure. The "Ref" of the JEventSource class
	// can be used to hold a pointer to an arbitrary object, though
	// you'll need to cast it to the correct pointer type
	// in the GetObjects method.
	event.SetJEventSource(this);
	event.SetEventNumber(++Nevents_read);
	event.SetRunNumber(1234);
	event.SetRef(NULL);

	// If an event was sucessfully read in, return NOERROR. Otherwise,
	// return NO_MORE_EVENTS_IN_SOURCE. By way of example, this
	// will return NO_MORE_EVENTS_IN_SOURCE after 100k events.
	if(Nevents_read>=100000)return NO_MORE_EVENTS_IN_SOURCE;

	return NOERROR;
}

//----------------
// FreeEvent
//----------------
void JEventSource_CPPsim::FreeEvent(JEvent &event)
{
	// If memory was allocated in GetEvent, it can be freed here. This
	// would typically be done by using event.GetRef() and casting the
	// returned void* into the proper pointer type so it can be deleted.
}

//----------------
// GetObjects
//----------------
jerror_t JEventSource_CPPsim::GetObjects(JEvent &event, JFactory_base *factory)
{
	// This callback is called to extract objects of a specific type from
	// an event and store them in the factory pointed to by JFactory_base.
	// The data type desired can be obtained via factory->GetDataClassName()
	// and the tag via factory->Tag().
	//
	// If the object is not one of a type this source can provide, then
	// it should return OBJECT_NOT_AVAILABLE. Otherwise, it should return
	// NOERROR;

	// We must have a factory to hold the data
	if(!factory)throw RESOURCE_UNAVAILABLE;

	// Get name of data class we're trying to extract and the factory tag
	string dataClassName = factory->GetDataClassName();
	string tag = factory->Tag();

	// Example for providing objects of type XXX
	//
	// // Get pointer to object of type MyEvent (this is optional)
	// MyEvent *myevent = (MyEvent*)event.GetRef();
	//
	// if(dataClassName == "XXX"){
	//
	//	 // Make objects of type XXX storing them in a vector
	//   vector<XXX*> xxx_objs;
	//	 ...
	//
	//	 JFactory<XXX> *fac = dynamic_cast<JFactory<XXX>*>(factory);
	//	 if(fac)fac->CopyTo(xxx_objs);
	//
	//	 return NOERROR;
	// }

	// For all other object types, just return OBJECT_NOT_AVAILABLE to indicate
	// we can't provide this type of object
	return OBJECT_NOT_AVAILABLE;
}

//--------------------
// AdjustPhysicsProcesses
//--------------------
void JEventSource_CPPsim::AdjustPhysicsProcesses(G4UImanager* UImanager)
{
	/// This adjusts which physics processes are turn on or off and
	/// and configurations of them based on user values found in
	/// control.in. The relevant settings are stored in global variables.

	G4cout << endl;

	// Multiple Scattering
	if(!MODE_MULS){
		string cmd = "/process/inactivate msc all";
		G4cout << "Turning off multiple scattering (MULS)" << endl;
		G4cout << cmd << endl << endl;
		UImanager->ApplyCommand(cmd.c_str());
	}

	// Energy Loss
	vector<string> cmds;
	switch(MODE_ELOSS){
		case 0:
			G4cout << "Turning off energy loss (LOSS=0)" << endl;
			cmds.push_back("/process/inactivate eIoni");
			cmds.push_back("/process/inactivate muIoni");
			cmds.push_back("/process/inactivate ionIoni");
			cmds.push_back("/process/inactivate hIoni");
			break;
		case 1:
		case 2:
			G4cout << "Default energy loss (LOSS=1,2)" << endl;
			break;
		case 4:
			G4cout << "Turning off energy loss fluctuations (LOSS=4)" << endl;
			cmds.push_back("/process/eLoss/fluct false");
			break;
		default:
			G4cout << "Unsuppoerted LOSS value ("<<MODE_ELOSS<<") shuould be 0,1,2, or 4" << endl;
			exit(-1);
	}
	for(uint32_t i=0; i<cmds.size(); i++){
		G4cout << cmds[i] << endl;
		UImanager->ApplyCommand(cmds[i].c_str());
	}
	G4cout << endl;

	// Decay
	if(!MODE_DCAY){
		string cmd = "/process/inactivate Decay all";
		G4cout << "Turning off decays (DCAY)" << endl;
		G4cout << cmd << endl << endl;
		UImanager->ApplyCommand(cmd.c_str());
	}

	// HADR
	cmds.clear();
	if(!MODE_HADR){
		// This is a subset of the possible reactions. A more complete review is needed.
		G4cout << "Tunining off (selected) Hadronic Interactions" << endl;
		cmds.push_back("/process/inactivate hadElastic");
		cmds.push_back("/process/inactivate neutronInelastic");
		cmds.push_back("/process/inactivate protonInelastic");
		cmds.push_back("/process/inactivate pi+Inelastic");
		cmds.push_back("/process/inactivate pi-Inelastic");
		cmds.push_back("/process/inactivate kaon+Inelastic");
		cmds.push_back("/process/inactivate kaon-Inelastic");
		cmds.push_back("/process/inactivate dInelastic");
		cmds.push_back("/process/inactivate ionInelastic");
	}
	for(uint32_t i=0; i<cmds.size(); i++){
		G4cout << cmds[i] << endl;
		UImanager->ApplyCommand(cmds[i].c_str());
	}

	// Random number sequence seeds
	char ccmd[256];
	sprintf(ccmd, "/random/setSequence %d", RNDM);
	G4cout << ccmd << endl;
	UImanager->ApplyCommand(ccmd);

	G4cout << endl;

}

//--------------------
// ReadControl_in
//--------------------
void JEventSource_CPPsim::ReadControl_in(void)
{
	ifstream ifs("control.in");
	if(!ifs.is_open()){
		jerr << endl << "No \"control.in\" file found!" << endl << endl;
		exit(-1);
	}
	jout << "Reading configuration from control.in ... " << endl;

	// Loop over file
	int iline=0;
	while(ifs.good()){

		// Read in line from file
		char line[512];
		ifs.getline(line, 512);
		iline++;
		if(ifs.gcount()<1) continue;

		// Tokenize line
		stringstream ss(line);
		vector<string> tokens;
		while(ss.good()){
			string s;
			ss >> s;
			if(!s.empty()) tokens.push_back(s);
		}

		// Skip empty or comment lines
		if(tokens.size() <2)continue; // we need the flag and at least one value for line to be useful
		string flag = tokens[0];
		for(unsigned int i=0; i<flag.size(); i++) flag[i] = toupper(flag[i]);
		if(flag[0] == 'C') continue;
		if(flag[0] == '*') continue;

		// Copy values into vals vectors, converting to appropriate forms
		vector<string> vals;
		vector<int> ivals;
		vector<double> fvals;
		for(unsigned int i=1; i<tokens.size(); i++){

			// Remove any single quotes from strings
			string val = "";
			for(unsigned int j=0; j<tokens[i].size(); j++){
				if(tokens[i][j] != '\'') val += tokens[i][j];
			}
			vals.push_back(val);

			// Convert ints and floats
			ivals.push_back(atoi(tokens[i].c_str()));
			fvals.push_back(atof(tokens[i].c_str()));
		}

		// Look for lines of interest
		if(flag == "TRIG"            ) MAXEVENTS = ivals[0];
		if(flag == "INFILE"          ) INPUT_FILENAME = vals[0];
		if(flag == "OUTFILE"         ) OUTPUT_FILENAME = vals[0];
		if(flag == "GEOMFILE"        ) gGDMLfilename = vals[0];
		if(flag == "GEOMOPT"         ) GEOMOPT = ivals[0];
		if(flag == "GEANT4_MACRO"    ) GEANT4_MACROS.push_back(vals[0]);
		if(flag == "BFIELDMAP"       ) BFIELD_MAP = vals[0];
		if(flag == "RNDM"            ) RNDM = ivals[0];
		if(flag == "MULS"            ) MODE_MULS = ivals[0];
		if(flag == "LOSS"            ) MODE_ELOSS = ivals[0];
		if(flag == "DCAY"            ) MODE_DCAY = ivals[0];
		if(flag == "HADR"            ) MODE_HADR = ivals[0];
		if(flag == "NTHREADS"        ) NG4THREADS = vals[0]=="max" ? 0:ivals[0];
		if(flag == "RUNG"            ) RUN_NUMBER = ivals[0];
		if(flag == "KINE"){
			if(fvals.size() != 7){
				G4cout << "KINE flag requires 7 arguments!!" << endl;
				exit(-1);
			}
			KINE_VALS = fvals;
		}
		if(flag == "SCAP"){
			if(fvals.size() != 3){
				G4cout << "SCAP flag requires 3 arguments!!" << endl;
				exit(-1);
			}
			SCAP_VALS = fvals;
		}

	}

	ifs.close();

	if( (!KINE_VALS.empty()) && SCAP_VALS.empty() ){
		SCAP_VALS.push_back(0.0);
		SCAP_VALS.push_back(0.0);
		SCAP_VALS.push_back(65.0);
	}

	// If geometry files was not set by user, then look for default
	if(gGDMLfilename == ""){
		G4cout << "GDML geometry file not specified." << G4endl;
		G4cout << "  looking for ./cpproot.gdml ... "; G4cout.flush();
		ifstream ifs("./cpproot.gdml");
		if(ifs.is_open()){
			ifs.close();
			G4cout << "yes" << G4endl;
			gGDMLfilename = "./cpproot.gdml";
		}else {
			G4cout << "no" << G4endl;

			const char *HDDS_HOME  = getenv("HDDS_HOME");
			const char *BMS_OSNAME = getenv("BMS_OSNAME");
			if(!HDDS_HOME ) G4cout << "HDDS_HOME  environment variable not set!" << G4endl;
			if(!BMS_OSNAME) G4cout << "BMS_OSNAME environment variable not set!" << G4endl;
			if(HDDS_HOME!=NULL && BMS_OSNAME!=NULL){

				string tmpname = string(HDDS_HOME) + "/" + BMS_OSNAME + "/src/cpproot.gdml";
				G4cout << "  looking for " + tmpname + " ... "; G4cout.flush();
				ifstream ifs(tmpname.c_str());
				if(ifs.is_open()){
					ifs.close();
					G4cout << "yes" << G4endl;
					gGDMLfilename = tmpname;
				}else{
					G4cout << "no" << G4endl;
				}
			}
		}
	}
	if(gGDMLfilename == ""){
		G4cout << "--- Unable to identify GDML geometry file! ---" << G4endl;
		G4cout << "--- Please specify it using \"GEOMFILE\" in control.in ---" << G4endl;
		exit(-1);
	}
}

//--------------------
// PrintConfigInfo
//--------------------
void JEventSource_CPPsim::PrintConfigInfo(void)
{
#ifdef G4MULTITHREADED
	bool mt_enabled = true;
	if(NG4THREADS==0) NG4THREADS = sysconf(_SC_NPROCESSORS_ONLN);
#else
	bool mt_enabled = false;
	if(NTHREADS==0) NTHREADS = 1;
	if(NTHREADS != 1) {
		G4cout << "---------------------------- WARNING ! ----------------------------" << endl;
		G4cout << "You have specified to run with " << NG4THREADS << " processing threads" << endl;
		G4cout << "but this executable is not capable of running with multiple" << endl;
		G4cout << "threads. The number of threads is therefore reduced to 1" << endl;
		G4cout << "--------------------------------------------------------------------" << endl;
		NTHREADS = 1;
	}
#endif


	G4cout << "         MAXEVENTS : " << MAXEVENTS << endl;
	if(INPUT_FILENAME!=""){
		G4cout << "    INPUT_FILENAME : " << (INPUT_FILENAME=="" ? "<none>":INPUT_FILENAME) << endl;
	}else if( !KINE_VALS.empty() ){
		G4cout << "    Particle gun ------------------------------" << endl;
		G4cout << "                       type: " << (int)KINE_VALS[0] << " (" << EParticle::ParticleName((EParticle::Type)(KINE_VALS[0]-100)) << ")" << endl;
		G4cout << "                          x: " << SCAP_VALS[0] << endl;
		G4cout << "                          y: " << SCAP_VALS[1] << endl;
		G4cout << "                          z: " << SCAP_VALS[2] << endl;
		G4cout << "                   momentum: " << KINE_VALS[1] << endl;
		G4cout << "                      theta: " << KINE_VALS[2] << endl;
		G4cout << "                        phi: " << KINE_VALS[3] << endl;
		G4cout << "             delta_momentum: " << KINE_VALS[4] << endl;
		G4cout << "                delta_theta: " << KINE_VALS[5] << endl;
		G4cout << "                  delta_phi: " << KINE_VALS[6] << endl;
		G4cout << "    -------------------------------------------" << endl;
	}
	G4cout << "   OUTPUT_FILENAME : " << OUTPUT_FILENAME << endl;
	G4cout << "     gGDMLfilename : " << gGDMLfilename << endl;
	G4cout << "          NTHREADS : " << NG4THREADS << (mt_enabled ? " (MT enabled)":"") << endl;
}
