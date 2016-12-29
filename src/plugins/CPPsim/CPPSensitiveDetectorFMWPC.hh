


#ifndef __CPPSENSITIVEDETECTORFMWPC_
#define __CPPSENSITIVEDETECTORFMWPC_


using namespace std;

#include "G4VSensitiveDetector.hh"

#include "CPPUserEventInformation.hh"
#include "CPPHitFMWPC.hh"

// These are defined in other headers included alongside this one. 
#undef TWO_HIT_RESOL
#undef THRESH_KEV

class CPPSensitiveDetectorFMWPC: public G4VSensitiveDetector
{
public:
	CPPSensitiveDetectorFMWPC(const G4String& name):G4VSensitiveDetector(name){
		
		collectionName.insert("fmwpchits");        // CPPHitFMWPC
		
		TWO_HIT_RESOL = 400.0; // ns
		THRESH_KEV = 0.0;      // keV
	}
	virtual ~CPPSensitiveDetectorFMWPC(){}
	
	
	//------------------------------
	// Initialize
	void Initialize(G4HCofThisEvent* hitCollection)
	{
		if(hitCollection == (void*)0x1) cout<< "Huh?"; // avoid compiler warning

		hits.clear();
	}
	
	//------------------------------
	// ProcessHits
	G4bool ProcessHits(G4Step* step, G4TouchableHistory* history)
	{
		if(history == (void*)0x1) cout<< "Huh?"; // avoid compiler warning
		
		G4double dEsum = step->GetTotalEnergyDeposit() / GeV;
		if (dEsum==0.) return false;
		
		// Get pre-step and post-step points
		G4StepPoint *sp_pre = step->GetPreStepPoint();
		G4StepPoint *sp_post = step->GetPostStepPoint();
		const G4TouchableHandle &touchable = sp_pre->GetTouchableHandle();

		// Convert the position from world coordinates to local coordinates within the block
		G4ThreeVector localPos = touchable->GetHistory()->GetTopTransform().TransformPoint(sp_pre->GetPosition());

		double dx = ((sp_post->GetPosition() - sp_pre->GetPosition()).mag()) /cm;

		// Get Layer number from z-position of hit within MWPC volume. The hierarchy is:
		//
		// (SITE)   <- Not always present
		//    |_ HALL
		//        |_ MWPC
		//            |_ CPPF
		//                |_ CPPG
		//
		// When accessing history, the "depth=0" volume will be the CPPG volume while
		// "depth=1" is CPPF and so on. 
		// We look for the "depth=2" transform since that should be the MWPC volume.
		// Note that the MWPC volume is defined as the mother volume of the entire
		// MWPC detector, including all layers and absorbers. The upstream face of
		// the gas volume of the most upstream chamber is at z=0 in the MWPC volume.
		const G4AffineTransform &TransformGlobalToMWPC = touchable->GetHistory()->GetTransform(2);
		G4ThreeVector pos_mwpc = (TransformGlobalToMWPC.TransformPoint(sp_pre->GetPosition()) - LAB_OFFSET)/cm;

		// Copy number of CPPF volume is layer number
		int layer = touchable->GetVolume(1)->GetCopyNo();
		
		// HDDS geometry does not include wires nor plane rotations. Assume 1 cm
		// wire spacing with wires at 0.5cm on either side of the beamline at
		// x,y = 0,0. Also assume odd number planes have wires in the vertical
		// direction and even numbered planes have wires in the horizontal direction.
		// Vertical wires start with wire 1 at x=-71.5 and wire 144 at x=+71.5
		// (the gas volume ends at x=+/-72.0). Horizontal wires start with wire 1
		// at y=-71.5 (i.e. closest to the ground) and wire 144 at y=+71.5
		// (i.e. closest to the sky).
		int wire = 0;
		if(layer%2){
			// Vertical wires
			wire = (int)floor(pos_mwpc.x() + 73.0);
		}else{
			// Horizontal wires
			wire = (int)floor(pos_mwpc.y() + 73.0);
		}
		
		if(wire < 1 || wire > 144) return false; // wire number out of range
		
		// Check if there's already a hit close in time to this one
		// Time at pre, post and mid step points
		double t_mid = 0.5*(sp_pre->GetGlobalTime() + sp_post->GetGlobalTime()) / ns;
		CPPHitFMWPC *hit = NULL;
		for(unsigned int i=0; i<hits.size(); i++){
			
			if( hits[i]->layer != layer ) continue;
			if( hits[i]->wire  != wire  ) continue;
			if (fabs(hits[i]->t - t_mid) < TWO_HIT_RESOL){
				hit = hits[i];
				break;
			}
		}
		
		// If not close to another hit (in time) then make a new hit
		if(!hit){
			hit = new CPPHitFMWPC(layer, wire);
			hit->t = t_mid;
			hits.push_back(hit);
		}
		
		if(hit){
			hit->dE += dEsum;
			hit->dx += dx;
			if( t_mid < hit->t) hit->t = t_mid;
		}


//int N=touchable->GetHistoryDepth();
//for(int i=0; i<N; i++){
//G4ThreeVector pos = (touchable->GetHistory()->GetTransform(i).TransformPoint(sp_pre->GetPosition()) - LAB_OFFSET)/cm;
//_DBG_<<i<<" "<<touchable->GetVolume(i)->GetName()<<" copy:"<< touchable->GetVolume(i)->GetCopyNo()<<"  "<<pos<<endl;
////_DBG_<<"  phi: " << touchable->GetVolume(3)->GetObjectRotationValue().xx()<<endl;
//}

		return false;
	}

	//------------------------------
	// EndOfEvent
	void EndOfEvent(G4HCofThisEvent* hitCollection)
	{
		// Add fmwpchits
		if(!hits.empty()){

			// Create hits collection 
			CPPHitFMWPCCollection *aHC = new CPPHitFMWPCCollection("FMWPC", "fmwpchits");
			
			// Add all hits above threshold
			int Nhits = 0;
			for(uint32_t i=0; i<hits.size(); i++){	
			
				// Here we drop hits that have too little energy.
				CPPHitFMWPC *hit = hits[i];
				bool keep_hit = (hit->dE*1.0E6 > THRESH_KEV);

				if(keep_hit){
					aHC->insert(hit); // add hit to collection
					Nhits++;
				}else{
					delete hit;       // reject hit
				}
			}

			// Add hit collection to the event
			G4int HCID = GetCollectionID(0); // get ID of fmwpchits
			hitCollection->AddHitsCollection(HCID, aHC);
		}
	}
	
private:
	vector<CPPHitFMWPC*> hits;

	G4double TWO_HIT_RESOL;
	G4double THRESH_KEV;
};




#endif // __CPPSENSITIVEDETECTORFMWPC_

