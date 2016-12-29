
#ifndef _CPPHITFCAL_
#define _CPPHITFCAL_

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class CPPHitFCAL:public G4VHit
{
public:

	CPPHitFCAL():Ecorr(0.0),tavg(0.0){}
	CPPHitFCAL(int myrow, int mycol):row(myrow),col(mycol),Ecorr(0.0),tavg(0.0){}
	~CPPHitFCAL(){}

	int row;
	int col;
	double Ecorr;
	double tavg;
};

typedef G4THitsCollection<CPPHitFCAL> CPPHitFCALCollection;


#endif // _CPPHITFCAL_
