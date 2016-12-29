


#ifndef __CPPSENSITIVEDETECTORTOF_
#define __CPPSENSITIVEDETECTORTOF_

#include <mutex>
using namespace std;

#include <JANA/JApplication.h>
using namespace jana;

#include <EParticle.h>

#include "G4VSensitiveDetector.hh"

#include "JEventSource_CPPsim.h"
#include "CPPUserEventInformation.hh"
#include "CPPHitTOF.hh"
#include "CPPHitTOFTruthPoint.hh"

#undef TWO_HIT_RESOL // dodge #define in DFDCGeometry

// static int &RUN_NUMBER = JEventSource_CPPsim::RUN_NUMBER;

static once_flag tofparms_flag;

static G4double ATTEN_LENGTH          = 150.0; // cm;
static G4double C_EFFECTIVE           = 15.0;  // cm/ns;
static G4double LONG_BAR_LENGTH       = 252.0; // length of the bar (cm);
static G4double SHORT_BAR_LENGTH      = 120.0; // length of "half"-bars (cm);
static G4double HALF_LONG_BAR_LENGTH  = LONG_BAR_LENGTH/2.0;
static G4double HALF_SHORT_BAR_LENGTH = SHORT_BAR_LENGTH/2.0;
static G4double TWO_HIT_RESOL         = 25.0;  // separation time between two different hits (defined in DFDCGeometry.h);
static G4double THRESH_MEV            = 0.0;   // do not throw away any hits, one can do that later;

class CPPSensitiveDetectorTOF: public G4VSensitiveDetector
{
public:

	//------------------------------
	// Constructor
	CPPSensitiveDetectorTOF(const G4String& name):G4VSensitiveDetector(name){
		collectionName.insert("toftruthpoints"); // CPPHitTOFTruthPoint
		collectionName.insert("tofhits");        // CPPHitTOF

		// Only one thread will actually execute InitParams, but all
		// others will wait for it to finish.
		call_once(tofparms_flag, &CPPSensitiveDetectorTOF::InitParams, this);
	}

	//------------------------------
	// Destructor
	virtual ~CPPSensitiveDetectorTOF(){}

	//------------------------------
	// InitParams
	void InitParams(void)
	{
		// This should be called by only one thread to set the
		// static parameters used by all CPPSensitiveDetectorTOF
		// objects (G4 creates one per thread).

		// Try and get values from CCDB to override
		map<string, double> tof_parms;
		if(japp){
			JCalibration *calib = japp->GetJCalibration(RUN_NUMBER);
			if(calib) calib->Get("TOF/tof_parms", tof_parms);
		}
		if(tof_parms.empty()){
			G4cout << "Unable to get TOF/tof_parms from CCDB (run="<<RUN_NUMBER<<"). Using defaults" << endl;
		}else{
			ATTEN_LENGTH    = tof_parms["TOF_ATTEN_LENGTH" ];
			C_EFFECTIVE     = tof_parms["TOF_C_EFFECTIVE"      ];
			LONG_BAR_LENGTH = tof_parms["TOF_PADDLE_LENGTH"];
			TWO_HIT_RESOL   = tof_parms["TOF_TWO_HIT_RESOL"];
			THRESH_MEV      = tof_parms["TOF_THRESH_MEV"   ];
			G4cout << "TOF from CCDB (run " << RUN_NUMBER << "): " << endl;
			G4cout << "      ATTEN_LENGTH = " << ATTEN_LENGTH    << endl;
			G4cout << "       C_EFFECTIVE = " << C_EFFECTIVE     << endl;
			G4cout << "   LONG_BAR_LENGTH = " << LONG_BAR_LENGTH << endl;
			G4cout << "     TWO_HIT_RESOL = " << TWO_HIT_RESOL   << endl;
			G4cout << "        THRESH_MEV = " << THRESH_MEV      << endl;
		}

		HALF_LONG_BAR_LENGTH  = LONG_BAR_LENGTH/2.0;
		HALF_SHORT_BAR_LENGTH = SHORT_BAR_LENGTH/2.0;
	}

	//------------------------------
	// Initialize
	void Initialize(G4HCofThisEvent* hitCollection)
	{
		if(hitCollection == (void*)0x1) cout<< "Huh?"; // avoid compiler warning

		hitsMap.clear();
		hitsMapTruthPoint.clear();
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

		// Check if this track has been recorded in hitsMapTruthPoint and if not, add it
		int itrack = step->GetTrack()->GetTrackID();
		int pdgtype = step->GetTrack()->GetDefinition()->GetPDGEncoding();
		int ptype = EParticle::PDGtoPType(pdgtype);
		G4ThreeVector mom = sp_pre->GetMomentum() / GeV;
		G4ThreeVector pos = (sp_pre->GetPosition() - LAB_OFFSET) / cm;
		double E = sp_pre->GetTotalEnergy() / GeV;
		double t = sp_pre->GetGlobalTime() / ns;
		if(hitsMapTruthPoint.find(itrack) == hitsMapTruthPoint.end()){

			CPPHitTOFTruthPoint *tofpart = new CPPHitTOFTruthPoint(mom, pos);
			tofpart->E = E;
			tofpart->t = t;
			tofpart->primary = 0; // This is overwritten in CPPHDDMout where Nprimaries is available
			tofpart->ptype = ptype;
			tofpart->track = itrack;
			tofpart->itrack = itrack;

			hitsMapTruthPoint[itrack] = tofpart;
		}

		// Convert the position from world coordinates to local coordinates within the block
		G4ThreeVector localPos = touchable->GetHistory()->GetTopTransform().TransformPoint(sp_pre->GetPosition());

		G4ThreeVector dx = sp_post->GetPosition() + sp_pre->GetPosition();

		// HDDS divides TOF into 4 sections and fills them with repeating structures.
		// The conversion to GDML seems to loose the bar and plane values so we have
		// to derive them by other means.
		// The easiest way to tell which plane this is in is by looking at the
		// "xx" element of the rotation matrix of the "FTOF" volume (depth=3).
		// (plane=0 is rotated -90 degrees while plane=1 is not rotated)
		G4int plane = (touchable->GetVolume(3)->GetObjectRotationValue().xx()==1.0 ? 1:0);

//int N=touchable->GetHistoryDepth();
//for(int i=0; i<N; i++)
//_DBG_<<i<<" "<<touchable->GetVolume(i)->GetName()<<" copy:"<< touchable->GetVolume(i)->GetCopyNo()<<"  "<<pos<<endl;
//_DBG_<<"  phi: " << touchable->GetVolume(3)->GetObjectRotationValue().xx()<<endl;


		// The bar is derived from the copy number and the section the hit is in.
		G4int bar = touchable->GetCopyNumber(1)+1;

		// Get section by name of physical volume a few levels up. This has got to be terribly
		// inefficient and is susceptible to changes to HDDS or even the GDML conversion!
		// For the TOF, the sections are defined where "FTOT" and "FTOB" are the sections with
		// full-length bars and "FTON" and "FTOS" are the half-length bars. In the original
		// hitFTOTF.c code in hdgeant, they called "column" and is defined below. We set the
		// variable "column" here in order to more closely mimic hdgeant to make it easier to
		// compare.
		//
		// FTOC = Full length, full width
		// FTOH = Half length, full width
		// FTOX = Full length, half width


		// Note: column=0 means paddle read out on both ends,
		//       column=1 means single-ended readout to north end
		//       column=2 means single-ended readout to south end
		string section = touchable->GetVolume(2)->GetName();
		int column = -1;
		      if(section == "FTOT_1"){  // Top full length, full width
			column = 0;
			bar += 26;
		}else if(section == "FTOB_1"){  // Bot full length, full width
			column = 0;
			bar += 1;
		}else if(section == "FTOY_1"){  // Top full length, half width
			column = 0;
			bar += 24;
		}else if(section == "FTOZ_1"){  // Bot full length, half width
			column = 0;
			bar += 20;
		}else if(section == "FTON_1"){  // North half length
			column = 1;
			bar += 22;
		}else if(section == "FTOS_1"){  // South half length
			column = 2;
			bar += 22;
		}

		double dist = localPos.x() / cm;

		G4double dxnorth = HALF_LONG_BAR_LENGTH - dist;
		G4double dxsouth = HALF_LONG_BAR_LENGTH + dist;

		G4double tnorth = (column == 2) ? 0 : t + dxnorth/C_EFFECTIVE;
		G4double tsouth = (column == 1) ? 0 : t + dxsouth/C_EFFECTIVE;

		// calculate energy seen by PM for this track step using attenuation factor
		G4double dEnorth = (column == 2) ? 0 : dEsum * exp(-dxnorth/ATTEN_LENGTH);
		G4double dEsouth = (column == 1) ? 0 : dEsum * exp(-dxsouth/ATTEN_LENGTH);

		// Make index based on bar and plane values and get reference to hits
		// vector for that block.
		int northidx = 1 + 10*bar + 1000*plane;
		int southidx = 2 + 10*bar + 1000*plane;
		vector<CPPHitTOF*> &northhits = hitsMap[northidx];
		vector<CPPHitTOF*> &southhits = hitsMap[southidx];

		// Add hits to each end
		AddHit(northhits, tnorth, dEnorth, bar, plane, CPPHitTOF::kNorth, ptype, itrack, mom, pos, E, dist);
		AddHit(southhits, tsouth, dEsouth, bar, plane, CPPHitTOF::kSouth, ptype, itrack, mom, pos, E, dist);

		return false;
	}


	//------------------------------
	// AddHit
	void AddHit(vector<CPPHitTOF*> &hits, G4double t, G4double dE, G4int bar, G4int plane, G4int end, int ptype, int itrack, G4ThreeVector &mom, G4ThreeVector &pos, double E, double dist)
	{
		/// Add a hit to the given hits vector

		// Don't bother if dE is zero (will happen for half-sized bars)
		if(dE == 0.0) return;

		// Check if there's already a hit close in time to this one
		CPPHitTOF *hit = NULL;
		for(unsigned int i=0; i<hits.size(); i++){
			if (fabs(hits[i]->t - t) < TWO_HIT_RESOL){
				hit = hits[i];
				break;
			}
		}

		// If not close to another hit (in time) then make a new hit
		if(!hit){
			hit = new CPPHitTOF(bar, plane, end);
			hits.push_back(hit);
		}

		// Add energy and calculate energy weighted time average
		G4double t_weighted = hit->t*hit->dE + t*dE;
		hit->dE += dE;
		hit->t  = t_weighted/hit->dE;

		// Add hit to tofExtras
		CPPHitTOF::tofExtra_t extra;
		extra.ptype  = ptype;
		extra.itrack = itrack;
		extra.px     = mom.x();
		extra.py     = mom.y();
		extra.pz     = mom.z();
		extra.x      = pos.x();
		extra.y      = pos.y();
		extra.z      = pos.z();
		extra.E      = E;
		extra.dist   = dist;
		hit->tofExtras.push_back(extra);
	}

	//------------------------------
	// EndOfEvent
	void EndOfEvent(G4HCofThisEvent* hitCollection)
	{
		// Add toftruthpoints
		if(!hitsMapTruthPoint.empty()){

			// Create hits collection
			CPPHitTOFTruthPointCollection *aHC = new CPPHitTOFTruthPointCollection("TOF", "toftruthpoints");

			map<G4int, CPPHitTOFTruthPoint* >::iterator it;
			for(it=hitsMapTruthPoint.begin(); it!=hitsMapTruthPoint.end(); it++){
				CPPHitTOFTruthPoint *hit = it->second;
				aHC->insert(hit); // add hit to collection
			}

			// Add hit collection to the event
			G4int HCID = GetCollectionID(0); // get ID of toftruthpoints
			hitCollection->AddHitsCollection(HCID, aHC);
		}


		// Add tofhits
		if(!hitsMap.empty()){

			// Create hits collection
			CPPHitTOFCollection *aHC = new CPPHitTOFCollection("TOF", "tofhits");

			// Add selected hits to the hits collection
			map<G4int, vector<CPPHitTOF*> >::iterator iter;
			for(iter=hitsMap.begin(); iter!=hitsMap.end(); iter++){
				vector<CPPHitTOF*> &hits = iter->second;
				for(unsigned int j=0; j<hits.size(); j++) {

					// Here we drop hits that have too little energy.
					CPPHitTOF *hit = hits[j];
					bool keep_hit = (hit->dE*1.0E3 > THRESH_MEV);

					if(keep_hit){
						aHC->insert(hit); // add hit to collection
					}else{
						delete hit;       // reject hit
					}
				}
			}

			// Add hit collection to the event
			G4int HCID = GetCollectionID(1); // get ID of tofhits
			hitCollection->AddHitsCollection(HCID, aHC);
		}
	}

private:
	map<G4int, vector<CPPHitTOF*> > hitsMap;
	map<G4int, CPPHitTOFTruthPoint* > hitsMapTruthPoint; // key = trackid
};




#endif // __CPPSENSITIVEDETECTORTOF_
