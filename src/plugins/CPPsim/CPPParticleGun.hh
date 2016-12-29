
#ifndef _CPPPARTICLEGUN_
#define _CPPPARTICLEGUN_

#include "G4ParticleGun.hh"

/// This class is here solely to allow us to access the
/// protected member "particle_energy" so we can reset
/// it to 0 before setting a new momentum value. Unbelievably,
/// Geant4 prints a message every time the momentum is
/// changed if the energy is not zero. It of course, sets
/// the energy to a non-zero value then and prints a message
/// if you try setting it to zero using the provided setter method.

class CPPParticleGun: public G4ParticleGun
{
public:
	CPPParticleGun(){}
	virtual ~CPPParticleGun(){}
	
	void Reset(void){
		particle_energy = 0.0;
	}
};


#endif // _CPPPARTICLEGUN_

