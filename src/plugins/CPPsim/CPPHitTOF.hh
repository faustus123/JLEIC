
#ifndef _CPPHITTOF_
#define _CPPHITTOF_

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class CPPHitTOF:public G4VHit
{
public:

	enum{
		kNorth,
		kSouth
	};

	class tofExtra_t{
		public:
			int ptype;
			int itrack;
			double px;
			double py;
			double pz;
			double x;
			double y;
			double z;
			double E;
			double dist;
	};

	CPPHitTOF(int mybar=0, int myplane=0, int myend=kNorth, double mydE=0.0, double myt=0.0):bar(mybar),plane(myplane),end(myend),dE(mydE),t(myt){}
	~CPPHitTOF(){}

	// ftofTruthHit
	int bar;
	int plane;
	int end;
	double dE;
	double t;

	// ftofTruthExtra
	vector<tofExtra_t> tofExtras;
};

typedef G4THitsCollection<CPPHitTOF> CPPHitTOFCollection;


#endif // _CPPHITTOF_
