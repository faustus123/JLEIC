

#ifndef _CPPMAGNETICFIELD_
#define _CPPMAGNETICFIELD_

#include "G4MagneticField.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

#include "JEventSource_CPPsim.h"

// The following are used to work around a problem that ROOT defines
// some functions named Info, Warning, and Error that conflict with
// the enum values of the same names defined in GEANT4.
#define Info InfoXX
#define Warning WarningXX
#define Error ErrorXX

//#include <DANA/DApplication.h>
//#include <HDGEOMETRY/DMagneticFieldMap.h>

//extern DApplication *dapp;
extern G4ThreeVector LAB_OFFSET; // Location of LASS volume in World volume. Used to translate to lab coordinates
static string &BFIELD_MAP= JEventSource_CPPsim::BFIELD_MAP; // Set in CPPsim.cc
static int &RUN_NUMBER = JEventSource_CPPsim::RUN_NUMBER;

class CPPMagneticField: public G4MagneticField{

public:

	//-----------------------
	// CPPMagneticField
	//-----------------------
	CPPMagneticField(){

		//bfield = NULL;

		// if(dapp) bfield = dapp->GetBfield(RUN_NUMBER);
		// if(!bfield){
		// 	cout << "***** UNABLE TO CREATE DMagneticFieldMap ****" << endl;
		// 	exit(-1);
		// }else{
		// 	cout << "DMagneticFieldMap created at: 0x" << hex << bfield << dec <<endl;

			extern CPPMagneticField *gCPPMagneticField;
			gCPPMagneticField = this;
		//}
	}

	//-----------------------
	// ~CPPMagneticField
	//-----------------------
	virtual ~CPPMagneticField(){

	}

	//-----------------------
	// GetFieldValue
	//-----------------------
	void GetFieldValue(const double Point[4], double *Bfield) const{

		/// Routine is called by GEANT4 when tracking

		// Shift position into nominal GlueX lab coordinates and convert into cm
//		double x = (Point[0] - LAB_OFFSET.x()) / cm;
//		double y = (Point[1] - LAB_OFFSET.y()) / cm;
//		double z = (Point[2] - LAB_OFFSET.z()) / cm;

		(void)Point; // dodge unused variable warning

		// Get field from HDGEOMETRY library
		// bfield->GetField(x, y, z, Bfield[0], Bfield[1], Bfield[2]);
		Bfield[0] = 0.0;
		Bfield[1] = 0.0;
		Bfield[2] = 2.0;
// printf("%s:%d B(%4.3f, %4.3f, %4.3f) = (%4.3f, %4.3f, %4.3f)\n", __FILE__, __LINE__, x, y, z, Bfield[0], Bfield[1], Bfield[2]);
		// B-field is returned in Tesla so convert to native units of GEANT4
		Bfield[0] *= tesla;
		Bfield[1] *= tesla;
		Bfield[2] *= tesla;
	}

	//-----------------------
	// GetFieldValue
	//-----------------------
	void GetFieldValue(const G4ThreeVector &pos, G4ThreeVector &B) const{
		// "pos" should be in global Geant4 coordinates (i.e. mm) withOUT
		// having subtracting the LAB_OFFSET (that is done internally)
		double Point[4] = {pos.x(), pos.y(), pos.z(), 0.0};
		double Bfield[3];
		GetFieldValue(Point, Bfield);
		B.set(Bfield[0], Bfield[1], Bfield[2]);
	}

private:
	//DMagneticFieldMap *bfield;
};

extern CPPMagneticField *gCPPMagneticField;


#endif // _CPPMAGNETICFIELD_
