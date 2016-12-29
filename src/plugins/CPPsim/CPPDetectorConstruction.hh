

#ifndef _CPPDETECTORCONSTRUCTION_H_
#define _CPPDETECTORCONSTRUCTION_H_

#include "G4VUserDetectorConstruction.hh"

#include <string>
using namespace std;

class CPPDetectorConstruction : public G4VUserDetectorConstruction
{
public:

    CPPDetectorConstruction(string gdmlfilename);
    virtual ~CPPDetectorConstruction();

    virtual G4VPhysicalVolume *Construct(void);
    virtual void ConstructSDandField(void);

	string fGDMLfilename;
   G4VPhysicalVolume *fWorld;
};

#endif  // _CPPDETECTORCONSTRUCTION_H_
