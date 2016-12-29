
#ifndef _CPPHITFDCAnode_
#define _CPPHITFDCAnode_

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class CPPHitFDCAnode:public G4VHit
{
public:
	
	CPPHitFDCAnode(int layer=0, int module=0, int wire=0, int itrack=-1, int ptype=-1):layer(layer),module(module),wire(wire),dE(0.0),t(1.0E6),t_unsmeared(0.0),d(0.0),itrack(itrack),ptype(ptype){}
	~CPPHitFDCAnode(){}
	
	int layer;
	int module;
	int wire;
	double dE;
	double t;
	double t_unsmeared;
	double d;
	int itrack;
	int ptype;	
};

typedef G4THitsCollection<CPPHitFDCAnode> CPPHitFDCAnodeCollection;


#endif // _CPPHITFDCAnode_
