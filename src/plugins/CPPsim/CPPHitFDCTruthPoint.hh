
#ifndef _CPPHitFDCTruthPoint_
#define _CPPHitFDCTruthPoint_

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class CPPHitFDCTruthPoint:public G4VHit
{
public:

	
	CPPHitFDCTruthPoint(int module, int layer, G4ThreeVector &mom, G4ThreeVector &pos):module(module),layer(layer),E(0.0),dEdx(0.0),dradius(0.0),primary(true),ptype(0),t(0.0),track(0),itrack(0){
		px = mom.x();
		py = mom.y();
		pz = mom.z();

		x = pos.x();
		y = pos.y();
		z = pos.z();
	}
	~CPPHitFDCTruthPoint(){}
	
	// This holds data to be written to the ftofTruthPoint structure in HDDM
	
	int module;
	int layer;
	double E;
	double dEdx;
	double dradius;
	bool primary;
	double ptype;
	double px;
	double py;
	double pz;
	double t;
	int track;
	double x;
	double y;
	double z;
	int itrack;  // for trackID
};

typedef G4THitsCollection<CPPHitFDCTruthPoint> CPPHitFDCTruthPointCollection;


#endif // _CPPHitFDCTruthPoint_
