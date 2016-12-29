

#include <thread>
#include <mutex>
using namespace std;

#include "CPPDetectorConstruction.hh"

#include "G4GDMLParser.hh"

#include "G4FieldManager.hh"
#include "G4EqMagElectricField.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "G4ClassicalRK4.hh"
#include "G4ChordFinder.hh"
#include "G4SDManager.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"

#include "JEventSource_CPPsim.h"
#include "CPPMagneticField.hh"
#include "CPPSensitiveDetectorFCAL.hh"
#include "CPPSensitiveDetectorTOF.hh"
#include "CPPSensitiveDetectorFDC.hh"
#include "CPPSensitiveDetectorFMWPC.hh"


// The following may be set to "1" in order to define a trivial
// geometry (single box of air) that makes some things much quicker
// when running if you're just trying to debug stuff. Usually, this
// should be set to "0".
#define CPP_TRIVIAL_GEOMETRY 0

// The following is used to hold a globally accessible pointer to B-field.
// This is accessed by CPPSensitiveDetectorFDC for Lorentz deflections.
CPPMagneticField *gCPPMagneticField=NULL; // used to hold globally accessible pointer to B-field



G4ThreeVector LAB_OFFSET; // Location of LASS volume in World volume. Used to translate to lab coordinates
//-------------------------------------------
// CPPDetectorConstruction (constructor)
//-------------------------------------------
CPPDetectorConstruction::CPPDetectorConstruction(string gdmlfilename)
{
	fGDMLfilename = gdmlfilename;
	fWorld = NULL;
}

//-------------------------------------------
// ~CPPDetectorConstruction (destructor)
//-------------------------------------------
CPPDetectorConstruction::~CPPDetectorConstruction()
{

}

//-------------------------------------------
// Construct
//-------------------------------------------
G4VPhysicalVolume* CPPDetectorConstruction::Construct(void)
{
#if CPP_TRIVIAL_GEOMETRY

G4NistManager* nist = G4NistManager::Instance();
G4Material* air_mat = nist->FindOrBuildMaterial("G4_AIR");

G4double world_sizeXY = 10.0*m;
G4Box* solidWorld = new G4Box("World",  0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeXY);

G4LogicalVolume* logicWorld =
new G4LogicalVolume(solidWorld,          //its solid
					air_mat,         //its material
					"World");            //its name

G4VPhysicalVolume* physWorld =
new G4PVPlacement(0,                     //no rotation
				  G4ThreeVector(),       //at (0,0,0)
				  logicWorld,            //its logical volume
				  "World",               //its name
				  0,                     //its mother  volume
				  false,                 //no boolean operation
				  0,                     //copy number
				  true);       // checking overlaps
return physWorld;

#else // CPP_TRIVIAL_GEOMETRY

	// Read and parse the GDML file to get the geometry
	G4GDMLParser parser;
	parser.Read(fGDMLfilename, false); // false=don't validate

	// Get pointer to world volume
	fWorld = parser.GetWorldVolume();

	// OK, this is very confusing but here goes. Originally, the GDML
	// file was created by hand by running the hddsroot.C macro and
	// then doing gGeoManager->Export(...). This worked great, but the
	// coordinates of everything had a global offset of (-150, 350, 500)
	// which needed to be subtracted off to get into our "normal" world
	// coordinates. This was attributed to the offset of the LASS volume
	// since it was poitioned using those numbers (well, the negative of
	// those). Now, when the GDML generation was automated, it was done
	// by running root in batch mode which supresses graphics. Thus, the
	// window was not opened and the graphics not drawn. Now, instead of
	// "HALL" being the world volume, "SITE" is the world volume and
	// "HALL" a daughter of it. "LASS is demoted to granddaughter of the
	// world volume. (Sounds crazy, I know!) As it turns out, HALL has
	// an offset from SITE of exactly the same values as LASS is offset
	// from HALL. This may have been the origin of the problem all along.
	// So, in order to handle the situation in both cases, we look for
	// either HALL or LASS as a daughter of the world volume. If HALL
	// is found then we use the negative of that for LAB_OFFSET. If we
	// find LASS then we use it as found. Because this can be screwed
	// up if neither volume is found, we check to make sure at least one
	// is and exit with an error if neither is found.
	G4LogicalVolume *fWorld_log = fWorld->GetLogicalVolume();
	int N = fWorld_log->GetNoDaughters();
	bool found_LASS = false;
	bool found_HALL = false;
	for(int i=0; i< N; i++){
		G4VPhysicalVolume *physvol = fWorld_log->GetDaughter(i);
		G4String name = physvol->GetLogicalVolume()->GetName();
		if(name == "LASS"){
			LAB_OFFSET = physvol->GetObjectTranslation();
			G4cout << "LASS volume origin found. LAB_OFFSET is : " <<  " : " << LAB_OFFSET << G4endl;
			found_LASS = true;
			break;
		}
		if(name == "HALL"){
//			LAB_OFFSET = -physvol->GetObjectTranslation();
			G4cout << "HALL volume origin found. LAB_OFFSET is : " <<  " : " << LAB_OFFSET << G4endl;
			found_HALL = true;
			break;
		}
	}

	if( !(found_LASS || found_HALL) ){
		cerr << "*** ERROR: Didn't find LASS or HALL volume among world volume's            ***" << endl;
		cerr << "***        daughters. Coordinate translations will not be correct so I'm   ***" << endl;
		cerr << "***        quitting now. (Error is coming from CPPDetectorConstruction.cc) ***" << endl;
		exit(-1);
	}

	// Create single magnetic field object
	gCPPMagneticField = new CPPMagneticField();

	return fWorld;
#endif // CPP_TRIVIAL_GEOMETRY
}

//-------------------------------------------
// ConstructSDandField
//-------------------------------------------
void CPPDetectorConstruction::ConstructSDandField(void)
{
	G4SDManager::GetSDMpointer()->SetVerboseLevel(0);

#if CPP_TRIVIAL_GEOMETRY

#else // CPP_TRIVIAL_GEOMETRY
	// Add FCAL Sensitive detector
	CPPSensitiveDetectorFCAL *fcalSD = new CPPSensitiveDetectorFCAL("FCAL");
	//G4SDManager::GetSDMpointer()->AddNewDetector(fcalSD);
	SetSensitiveDetector("LGBL", fcalSD);

	// Add TOF Sensitive detector
	CPPSensitiveDetectorTOF *tofSD = new CPPSensitiveDetectorTOF("TOF");
	//G4SDManager::GetSDMpointer()->AddNewDetector(tofSD);
	SetSensitiveDetector("FTOC", tofSD);
	SetSensitiveDetector("FTOH", tofSD);
	SetSensitiveDetector("FTOX", tofSD);

	// Add FDC Sensitive detector
	CPPSensitiveDetectorFDC *fdcSD = new CPPSensitiveDetectorFDC("FDC");
	//G4SDManager::GetSDMpointer()->AddNewDetector(fdcSD);
	SetSensitiveDetector("FDA1", fdcSD);
	SetSensitiveDetector("FDA2", fdcSD);
	SetSensitiveDetector("FDA3", fdcSD);
	SetSensitiveDetector("FDA4", fdcSD);

	// Add FMWPC Sensitive detector
	CPPSensitiveDetectorFMWPC *fmwpcSD = new CPPSensitiveDetectorFMWPC("FMWPC");
	//G4SDManager::GetSDMpointer()->AddNewDetector(fdcSD);
	SetSensitiveDetector("CPPG", fmwpcSD);

	// Create separate field manager for each thread
	G4FieldManager* fman = new G4FieldManager();
	fman->SetDetectorField(gCPPMagneticField);
	fman->CreateChordFinder(gCPPMagneticField);

	G4LogicalVolume *pWorld = fWorld->GetLogicalVolume();
	pWorld->SetFieldManager(fman, true);

	// The following will print a message indicating whether geometry optimization
	// is on or not. It is complicated because we want the message to show up
	// as the last thing printed while the program appears to be stalled since
	// that is what the user will associate with the delay.
	static once_flag geomopt_mess_printed;
	call_once(geomopt_mess_printed,
		[](void){
			if(JEventSource_CPPsim::GEOMOPT){
				thread([](void){
					sleep(2);
					cout << endl;
					cout << "Optimizing Geometry.... this may take 30 sec..." << endl;
					cout << "(see GEOMOPT in control.in for details)" << endl;
				}).detach();
			}else{
				G4cout << "Geometry optimization is OFF. Event processing will be slower." << endl;
				G4cout << "(see GEOMOPT in control.in for details)" << endl;
			}
		});

#endif // CPP_TRIVIAL_GEOMETRY
}
