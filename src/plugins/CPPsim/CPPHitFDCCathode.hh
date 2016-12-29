
#ifndef _CPPHITFDCCathode_
#define _CPPHITFDCCathode_

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class CPPHitFDCCathode:public G4VHit
{
public:
	
	CPPHitFDCCathode(int layer=0, int module=0, int plane=0, int strip=0, int itrack=-1, int ptype=-1):layer(layer),module(module),plane(plane),strip(strip),q(0.0),t(1.0E6),itrack(itrack),ptype(ptype){}
	~CPPHitFDCCathode(){}
	
	int layer;
	int module;
	int plane;
	int strip;
	double q;
	double t;	
	int itrack;
	int ptype;	
};

typedef G4THitsCollection<CPPHitFDCCathode> CPPHitFDCCathodeCollection;


#endif // _CPPHITFDCCathode_
