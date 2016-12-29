

#ifndef _CPPSENSITIVEDETECTORFCAL_
#define _CPPSENSITIVEDETECTORFCAL_

#include <map>
using namespace std;

#include "G4VSensitiveDetector.hh"
#include "G4SystemOfUnits.hh"

#include "CPPHitFCAL.hh"

#undef TWO_HIT_RESOL  // (this seems to be defined in DFDCGeometry.h

class CPPSensitiveDetectorFCAL: public G4VSensitiveDetector
{
public:
	CPPSensitiveDetectorFCAL(const G4String& name):G4VSensitiveDetector(name){
		collectionName.insert("fcalhits");
		
		TWO_HIT_RESOL   = 75.0;   // ns   
		ATTEN_LENGTH    = 100.0;  // cm
		C_EFFECTIVE     = 15.0;   // cm/ns
		WIDTH_OF_BLOCK  = 4.0;    // cm
		LENGTH_OF_BLOCK = 45.0;   // cm
		THRESH_MEV      =  5.0;   // MeV
		ACTIVE_RADIUS   = 120.0;  // cm
		ACTIVE_RADIUS2  = ACTIVE_RADIUS*ACTIVE_RADIUS;
		CENTRAL_ROW     = 29;
		CENTRAL_COLUMN  = 29;
	}
	
	virtual ~CPPSensitiveDetectorFCAL(){}
	
	
	//------------------------------
	// Initialize
	void Initialize(G4HCofThisEvent* hitCollection)
	{
		if(hitCollection == (void*)0x1) cout<< "Huh?"; // avoid compiler warning

		hitsMap.clear();
	}
	
	//------------------------------
	// ProcessHits
	G4bool ProcessHits(G4Step* step, G4TouchableHistory* history)
	{
		if(history == (void*)0x1) cout<< "Huh?"; // avoid compiler warning
		
		G4double dEsum = step->GetTotalEnergyDeposit() / MeV;
		
		if (dEsum==0.) return false;
		
		// Get pre-step and post-step points
		G4StepPoint *sp_pre = step->GetPreStepPoint();
		G4StepPoint *sp_post = step->GetPostStepPoint();
		const G4TouchableHandle &hand = sp_pre->GetTouchableHandle();

		// Consider point where the particle is at as the mean between the
		// pre-step and post-step		
		G4ThreeVector pos = 0.5*(sp_pre->GetPosition() + sp_post->GetPosition());
		G4ThreeVector mom = 0.5*(sp_pre->GetMomentum() + sp_post->GetMomentum());
		G4double t =  0.5*(sp_pre->GetGlobalTime() + sp_post->GetGlobalTime()) / ns;

		// Convert the position from world coordinates to local coordinates within the block
		G4ThreeVector localPos = hand->GetHistory()->GetTopTransform().TransformPoint(pos);
		G4double dist = 0.5*LENGTH_OF_BLOCK - localPos.z()/cm; // ... relative to center of block


		// Correct Edep for attenuation and t for effective c
		G4double dEcorr = dEsum * exp(-dist/ATTEN_LENGTH);
		G4double tcorr = t + dist/C_EFFECTIVE;

		// HDDS divides FCAL into 4 sections and fills them with repeating structures.
		// The conversion to GDML seems to loose the row and column values so we have
		// to derive them from the copy numbers and the section they are in.
		G4int row = hand->GetCopyNumber(2);
		G4int col = hand->GetCopyNumber(1);
	
		// Get section by name of physical volume a few levels up. This has got to be terribly
		// inefficient and is suseptible to changes to HDDS or even the GDML conversion!
		string section = hand->GetVolume(3)->GetName();
		if(section == "LGDT_PV"){
			row += 31;
		}else if(section == "LGDB_PV"){
			// do nothing
		}else if(section == "LGDN_PV"){
			col += 31;
			row += 28;
		}else if(section == "LGDS_PV"){
			row += 28;
		}

		// Make index based on row and col values and get reference to hits
		// vector for that block.
		int idx = row + 100*col;
		vector<CPPHitFCAL*> &hits = hitsMap[idx];

		// Check if there's already a hit close in time to this one
		CPPHitFCAL *hit = NULL;
		for(unsigned int i=0; i<hits.size(); i++){
			if (fabs(hits[i]->tavg - tcorr) < TWO_HIT_RESOL){
				hit = hits[i];
				break;
			}
		}
		
		// If not close to another hit (in time) then make a new hit
		if(!hit){
			hit = new CPPHitFCAL(row, col);
			hits.push_back(hit);
		}
		
		// Add energy and calculate energy weighted time average
		G4double t_weighted = hit->tavg*hit->Ecorr + tcorr*dEcorr;
		hit->Ecorr += dEcorr;
		hit->tavg  = t_weighted/hit->Ecorr;
		
		return false;
	}
	
	//------------------------------
	// EndOfEvent
	void EndOfEvent(G4HCofThisEvent* hitCollection)
	{
		//if(hitsMap.empty()) return;

		// Create hits collection 
		CPPHitFCALCollection *aHC = new CPPHitFCALCollection("FCAL", "fcalhits");
		
		// Add selected hits to the hits collection
		map<G4int, vector<CPPHitFCAL*> >::iterator iter;
		for(iter=hitsMap.begin(); iter!=hitsMap.end(); iter++){
			vector<CPPHitFCAL*> &hits = iter->second;
			for(unsigned int j=0; j<hits.size(); j++) {

				// Here we drop hits that are either too little
				// energy or outside of the active radius of the
				// FCAL. (Recall that the FCAL is defined as a
				// square shaped block in the simulation for
				// simplicity while it reality, the blocks are
				// stacked into a round shape.
				CPPHitFCAL *hit = hits[j];
				
				G4double y0 = (G4double)(hit->row - CENTRAL_ROW)*WIDTH_OF_BLOCK;
         		G4double x0 = (G4double)(hit->col - CENTRAL_COLUMN)*WIDTH_OF_BLOCK;
         		G4double dist2 = x0*x0 + y0*y0;
				
				bool keep_hit = (hit->Ecorr > THRESH_MEV) && (dist2 < ACTIVE_RADIUS2);

				if(keep_hit){
					aHC->insert(hit); // add hit to collection
				}else{
					delete hit;       // reject hit
				}
			}
		}

		// Add hit collection to the event
		G4int HCID = GetCollectionID(0); // get first ID of this sensitive detector
		hitCollection->AddHitsCollection(HCID, aHC);
	}
	
private:
	map<G4int, vector<CPPHitFCAL*> > hitsMap;
	
	G4double TWO_HIT_RESOL;
	G4double ATTEN_LENGTH;
	G4double C_EFFECTIVE;
	G4double WIDTH_OF_BLOCK;
	G4double LENGTH_OF_BLOCK;
	G4double THRESH_MEV;
	G4double ACTIVE_RADIUS;
	G4double ACTIVE_RADIUS2;
	G4int    CENTRAL_ROW;
	G4int    CENTRAL_COLUMN;

};


#endif // _CPPSENSITIVEDETECTORFCAL_


