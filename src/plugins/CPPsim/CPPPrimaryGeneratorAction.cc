
#include "JEventSource_CPPsim.h"
#include "CPPPrimaryGeneratorAction.hh"
#include "CPPUserEventInformation.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

using namespace std;
// using namespace hddm_s;

#include <JANA/jerror.h>

unsigned int Nevents = 0;

extern G4ThreeVector LAB_OFFSET; // Location of LASS volume in World volume. Used to translate to lab coordinates
extern unsigned int MAXEVENTS;

static string &INPUT_FILENAME     = JEventSource_CPPsim::INPUT_FILENAME;
static vector<double>  &KINE_VALS = JEventSource_CPPsim::KINE_VALS;
static vector<double>  &SCAP_VALS = JEventSource_CPPsim::SCAP_VALS;


static pthread_mutex_t hddm_mutex = PTHREAD_MUTEX_INITIALIZER;
static ifstream *ifs = NULL;
//static hddm_s::istream *fin = NULL;
// static uint64_t Nhddm = 0;
static uint64_t Nevents_generated = 0;
static uint32_t Nhddm_users = 0; // keeps track of when a destructor can delete ifs, and fin

//--------------------------------------------
// CPPPrimaryGeneratorAction (constructor)
//--------------------------------------------
CPPPrimaryGeneratorAction::CPPPrimaryGeneratorAction() : G4VUserPrimaryGeneratorAction(), fParticleGun(0)
{
	// Create particle gun. This is used to generate all particles whether they
	// come from an input file or the built-in "particle gun"
	fParticleGun = new CPPParticleGun();
	source_type = SOURCE_TYPE_NONE;

	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
	part_gamma    = particleTable->FindParticle("gamma");
	part_electron = particleTable->FindParticle("electron");
	part_positron = particleTable->FindParticle("positron");
	part_mup      = particleTable->FindParticle("mu+");
	part_mum      = particleTable->FindParticle("mu-");
	part_pip      = particleTable->FindParticle("pi+");
	part_pim      = particleTable->FindParticle("pi-");
	part_eta      = particleTable->FindParticle("eta");
	part_eta_prime= particleTable->FindParticle("eta'");
	part_proton   = particleTable->FindParticle("proton");
	part_neutron  = particleTable->FindParticle("neutron");
	part_Kp       = particleTable->FindParticle("K+");
	part_Km       = particleTable->FindParticle("K-");
	part_pi0      = particleTable->FindParticle("pi0");

	if(INPUT_FILENAME!=""){

		pthread_mutex_lock(&hddm_mutex);
		Nhddm_users++;

		// Open HDDM input file and associate stream with HDDM record
		if(Nhddm_users==1){  // only first user should open input file !
			ifs = new ifstream(INPUT_FILENAME.c_str());
			if (!ifs->is_open()) {
				G4cerr << "Unable to open HDDM input file: " << INPUT_FILENAME << " !!" << endl;
				pthread_mutex_unlock(&hddm_mutex);
				exit(-1);
			}
//			fin = new hddm_s::istream(*ifs);
			G4cout << "Opened input file: " << INPUT_FILENAME << endl;
		}
		source_type = SOURCE_TYPE_HDDM;

		pthread_mutex_unlock(&hddm_mutex);

	}else if(!KINE_VALS.empty()){
		source_type = SOURCE_TYPE_PARTICLE_GUN;
		partgun_geanttype = (int)KINE_VALS[0] - 100;
		switch(partgun_geanttype){
			case  1: partgun_type = part_gamma; break;
			case  3: partgun_type = part_electron; break;
			case  2: partgun_type = part_positron; break;
			case  5: partgun_type = part_mup; break;
			case  6: partgun_type = part_mum; break;
			case  8: partgun_type = part_pip; break;
			case  9: partgun_type = part_pim; break;
			case 11: partgun_type = part_Kp; break;
			case 12: partgun_type = part_Km; break;
			case  7: partgun_type = part_pi0; break;
			case 14: partgun_type = part_proton; break;
			case 13: partgun_type = part_neutron; break;

			default:
				G4cerr << "Unknown GEANT particle type: " << partgun_geanttype << endl;
				G4cerr << "(it may need to be added to CPPPrimaryGeneratorAction.cc)" << endl;
				exit(-1);
		}

		double x            = SCAP_VALS[0]*cm;
		double y            = SCAP_VALS[1]*cm;
		double z            = SCAP_VALS[2]*cm;
		double p            = KINE_VALS[1]*GeV;
		double theta        = KINE_VALS[2];
		double phi          = KINE_VALS[3];
		partgun_delta_p     = KINE_VALS[4]*GeV;
		partgun_delta_theta = KINE_VALS[5]*0.01745329251994;  // convert to radians
		partgun_delta_phi   = KINE_VALS[6]*0.01745329251994;  // convert to radians
		partgun_delta_t     = 2.0; // ns

		partgun_pos.set(x, y, z);
		partgun_mom.set(0.0, 0.0, p); // caution! don't use setMag here without initializing vector!
		partgun_mom.setTheta(theta * 0.01745329251994);  // convert to radians
		partgun_mom.setPhi(phi * 0.01745329251994);      // convert to radians
	}

}

//--------------------------------------------
// ~CPPPrimaryGeneratorAction (destructor)
//--------------------------------------------
CPPPrimaryGeneratorAction::~CPPPrimaryGeneratorAction()
{
	pthread_mutex_lock(&hddm_mutex);

	Nhddm_users--;

	if(fParticleGun) delete fParticleGun;
	fParticleGun = NULL;

	if(Nhddm_users==0){
		// if(fin) delete fin;
		if(ifs) delete ifs;

		// fin = NULL;
		ifs = NULL;
	}

	pthread_mutex_unlock(&hddm_mutex);
}

//--------------------------------------------
// GeneratePrimaries
//--------------------------------------------
void CPPPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{

	pthread_mutex_lock(&hddm_mutex);

	// Dispatch to the appropriate particle generator
	switch(source_type){
		case SOURCE_TYPE_HDDM:
			GeneratePrimariesHDDM(anEvent);
			break;
		case SOURCE_TYPE_PARTICLE_GUN:
			GeneratePrimariesParticleGun(anEvent);
			break;
		default:
			G4cout << "Generated particle input type unsupported!" << endl;
			pthread_mutex_unlock(&hddm_mutex);
			exit(-1);
	}

	Nevents_generated++;

	pthread_mutex_unlock(&hddm_mutex);

}

//--------------------------------------------
// GeneratePrimariesParticleGun
//--------------------------------------------
void CPPPrimaryGeneratorAction::GeneratePrimariesParticleGun(G4Event* anEvent)
{
	// Unbelievably, GEANT4's G4ParticleGun class insists on printing
	// a message whenever the momentum or energy is changed, unless
	// the other is 0. Here, we reset the particle gun energy using
	// our own derived class. (Sheesh!!)
	fParticleGun->Reset();

	G4ThreeVector pos(partgun_pos + LAB_OFFSET);
	G4ThreeVector mom(partgun_mom);

	// Optionally smear momentum
	if(partgun_delta_p     != 0.0) mom.setMag(   partgun_mom.mag()   + (partgun_delta_p     * (G4UniformRand()-0.5)) );
	if(partgun_delta_theta != 0.0) mom.setTheta( partgun_mom.theta() + (partgun_delta_theta * (G4UniformRand()-0.5)) );
	if(partgun_delta_phi   != 0.0) mom.setPhi(   partgun_mom.phi()   + (partgun_delta_phi   * (G4UniformRand()-0.5)) );

	// Smear start time of primary and shift so it would cross z=65 at t_unsmeared=0
	double lab_z = (pos-LAB_OFFSET).z()/cm;
	double t_offset = (65.0 - lab_z)/30.0;  // offset in photon flight time in ns from center of z-range (not yet implemented)
	double t = t_offset + partgun_delta_t*G4RandGauss::shoot();

	fParticleGun->SetParticleTime(t*ns);
	fParticleGun->SetParticlePosition(pos);
	fParticleGun->SetParticleMomentum(mom);
	fParticleGun->SetParticleDefinition(partgun_type);
	fParticleGun->GeneratePrimaryVertex(anEvent);

	// Set event number
	anEvent->SetEventID(Nevents_generated+1); // Add one so we start counting from 1

	// Store generated particle info so it can be written to output file
	pos -= LAB_OFFSET;
	pos *= 1.0/cm; // convert to cm
	anEvent->SetUserInformation(new CPPUserEventInformation(partgun_geanttype, pos, mom));
}

//--------------------------------------------
// GeneratePrimariesHDDM
//--------------------------------------------
void CPPPrimaryGeneratorAction::GeneratePrimariesHDDM(G4Event* anEvent)
{
	int Nprimaries = 0;

	// Read event from HDDM file
// 	if(fin){
//
// 		// Check maximum number of events limit
// 		Nevents++;
// 		if(MAXEVENTS>0 && Nevents>MAXEVENTS){
// 			anEvent->SetEventAborted();
// 			return;
// 		}
//
// 		// Check that the input stream is still open and good
// 		if(!ifs->good()) {
// 			anEvent->SetEventAborted();
// 			return;
// 		}
//
// 		// Read in new HDDM event
// 		HDDM *hddmevent = new HDDM;
// 		try{
// 			(*fin) >> (*hddmevent);
// 		}catch(std::exception e){
// 			G4cout << e.what() << endl;
// 			anEvent->SetEventAborted();
// 			return;
// 		}
// 		Nhddm++;
//
// 		// Store pointer to event so it can be used to write event out
// 		// and deleted later.
// 		anEvent->SetUserInformation(new CPPUserEventInformation(hddmevent));
//
// 		// Get all vertices
// 		VertexList vertices = hddmevent->getVertices();
// 		if(vertices.size()==0) {
// 			G4cout << "No vertices in input HDDM event!" << G4endl;
// 			anEvent->SetEventAborted();
// 			return;
// 		}
//
// 		// Loop over Vertices
// 		VertexList::iterator it_vertex;
// 		for(it_vertex=vertices.begin(); it_vertex!=vertices.end(); it_vertex++){
//
// 			// ------------ Event Number --------------
// 			anEvent->SetEventID(it_vertex->getEventNo());
//
// 			// ------------ Origin --------------
// 			Origin      &origin   = it_vertex->getOrigin();
// 			double x = origin.getVx()*cm + LAB_OFFSET.x();
// 			double y = origin.getVy()*cm + LAB_OFFSET.y();
// 			double z = origin.getVz()*cm + LAB_OFFSET.z();
// 			G4ThreeVector pos(x, y, z);
//
// 			// ------------ Products --------------
// 			ProductList &products = it_vertex->getProducts();
// 			ProductList::iterator it_product;
// 			for(it_product=products.begin(); it_product!=products.end(); it_product++){
//
// 				Momentum &momentum = it_product->getMomentum();
// 				double px = momentum.getPx() * GeV;
// 				double py = momentum.getPy() * GeV;
// 				double pz = momentum.getPz() * GeV;
// 				G4ThreeVector mom(px, py, pz);
//
// 				int pdgtype = it_product->getPdgtype();
// 				int mech    = it_product->getMech();
//
// 				// Not sure how to ignore intermediaries here. It looks like
// 				// modern bggen set mech=1 while genr8_2_hddm sets it 0. gen_2mu
// 				// will also set it to 3. For now, ignore any particles that are
// 				// not one of these three values.
// 				if(mech!=0 && mech!=1 && mech!=2) continue;
//
// 				// Unbelievably, GEANT4's G4ParticleGun class insists on printing
// 				// a message whenever the momentum or energy is changed, unless
// 				// the other is 0. Here, we reset the particle gun energy using
// 				// our own derived class. (Sheesh!!)
// 				fParticleGun->Reset();
//
// 				G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
// 				G4ParticleDefinition *partdef = particleTable->FindParticle(pdgtype);
//
// 				fParticleGun->SetParticlePosition(pos);
// 				fParticleGun->SetParticleMomentum(mom);
// 				fParticleGun->SetParticleDefinition(partdef);
//
// // 				switch(pdgtype){
// // 					case   22: fParticleGun->SetParticleDefinition(part_gamma); break;
// // 					case   11: fParticleGun->SetParticleDefinition(part_electron); break;
// // 					case  -11: fParticleGun->SetParticleDefinition(part_positron); break;
// // 					case   13: fParticleGun->SetParticleDefinition(part_mup); break;
// // 					case  -13: fParticleGun->SetParticleDefinition(part_mum); break;
// // 					case  211: fParticleGun->SetParticleDefinition(part_pip); break;
// // 					case -211: fParticleGun->SetParticleDefinition(part_pim); break;
// // 					case  221: fParticleGun->SetParticleDefinition(part_eta); break;
// // 					case  331: fParticleGun->SetParticleDefinition(part_eta_prime); break;
// // 					case  321: fParticleGun->SetParticleDefinition(part_Kp); break;
// // 					case -321: fParticleGun->SetParticleDefinition(part_Km); break;
// // 					case  111: fParticleGun->SetParticleDefinition(part_pi0); break;
// // 					case 2212: fParticleGun->SetParticleDefinition(part_proton); break;
// // 					case 2112: fParticleGun->SetParticleDefinition(part_neutron); break;
// // 					default:
// // 						G4cerr << "Unknown PDG type: " << pdgtype << endl;
// // 						G4cerr << "(it may need to be added to CPPPrimaryGeneratorAction.cc)" << endl;
// // 						exit(-1);
// // 				}
//
// 				fParticleGun->GeneratePrimaryVertex(anEvent);
// 				Nprimaries++;
// 			}
// 		}
// 	}

	if(Nprimaries == 0){
		G4cerr << "Number of primaries in event is zero!!" << endl;
		anEvent->SetEventAborted();
	}
}
