

#ifndef _CPPPRIMARYGENERATORACTION_H_
#define _CPPPRIMARYGENERATORACTION_H_

#include "CPPParticleGun.hh"

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleDefinition.hh"

#include "globals.hh"

//#include <HDDM/hddm_s.hpp>

#include <pthread.h>
#include <fstream>
using namespace std;

class G4Event;



class CPPPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:

	enum source_type_t{
		SOURCE_TYPE_NONE,
		SOURCE_TYPE_HDDM,
		SOURCE_TYPE_PARTICLE_GUN
	};

    CPPPrimaryGeneratorAction();
	~CPPPrimaryGeneratorAction();

	virtual void GeneratePrimaries(G4Event* anEvent);
	void GeneratePrimariesHDDM(G4Event* anEvent);
	void GeneratePrimariesParticleGun(G4Event* anEvent);

private:

    CPPParticleGun* fParticleGun;
	source_type_t source_type;

	int partgun_geanttype;
	G4ParticleDefinition *partgun_type;
	G4ThreeVector partgun_pos;
	G4ThreeVector partgun_mom;
	double partgun_delta_p;
	double partgun_delta_theta;
	double partgun_delta_phi;
	double partgun_delta_t;

	G4ParticleDefinition *part_gamma;
	G4ParticleDefinition *part_electron;
	G4ParticleDefinition *part_positron;
	G4ParticleDefinition *part_mup;
	G4ParticleDefinition *part_mum;
	G4ParticleDefinition *part_pip;
	G4ParticleDefinition *part_pim;
	G4ParticleDefinition *part_eta;
	G4ParticleDefinition *part_eta_prime;
	G4ParticleDefinition *part_proton;
	G4ParticleDefinition *part_neutron;
	G4ParticleDefinition *part_Kp;
	G4ParticleDefinition *part_Km;
	G4ParticleDefinition *part_pi0;

};

#endif
