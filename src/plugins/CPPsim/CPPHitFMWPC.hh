
#ifndef _CPPHITFMWPC_
#define _CPPHITFMWPC_

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class CPPHitFMWPC:public G4VHit
{
public:
	
	CPPHitFMWPC():layer(0),wire(0),dE(0.0),dx(0.0),t(0.0){}
	CPPHitFMWPC(int layer, int wire):layer(layer),wire(wire),dE(0.0),dx(0.0),t(0.0){}
	~CPPHitFMWPC(){}
	
	int layer;
	int wire;
	double dE;
	double dx;
	double t;
};

typedef G4THitsCollection<CPPHitFMWPC> CPPHitFMWPCCollection;


#endif // _CPPHITFMWPC_
