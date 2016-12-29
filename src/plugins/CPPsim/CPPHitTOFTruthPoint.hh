
#ifndef _CPPHitTOFTruthPoint_
#define _CPPHitTOFTruthPoint_

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class CPPHitTOFTruthPoint:public G4VHit
{
public:

	
	CPPHitTOFTruthPoint(G4ThreeVector &mom, G4ThreeVector &pos):E(0.0),primary(true),ptype(0),t(0.0),track(0),itrack(0){
		px = mom.x();
		py = mom.y();
		pz = mom.z();

		x = pos.x();
		y = pos.y();
		z = pos.z();
	}
	~CPPHitTOFTruthPoint(){}
	
	// This holds data to be written to the ftofTruthPoint structure in HDDM
	
	double E;
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

typedef G4THitsCollection<CPPHitTOFTruthPoint> CPPHitTOFTruthPointCollection;


#endif // _CPPHitTOFTruthPoint_
