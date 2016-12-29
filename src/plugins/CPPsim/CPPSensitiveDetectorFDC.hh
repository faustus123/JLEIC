

#ifndef _CPPSENSITIVEDETECTORFDC_
#define _CPPSENSITIVEDETECTORFDC_

#include <map>
using namespace std;

#include "G4VSensitiveDetector.hh"

#include <CLHEP/Random/RandGauss.h>
#include <CLHEP/Random/RandPoisson.h>

#include <EParticle.h>

#include "JEventSource_CPPsim.h"
#include "CPPHitFDCTruthPoint.hh"
#include "CPPHitFDCAnode.hh"
#include "CPPHitFDCCathode.hh"
#include "CPPMagneticField.hh"

static int &MODE_ELOSS = JEventSource_CPPsim::MODE_ELOSS;

// //---------------------------------------------------
// // From FDC/DFDCGeometry.h in GlueX sim-recon
// //
// #define FDC_NUM_LAYERS			24
// #define FDC_NUM_PLANES                  72
// #define FDC_ACTIVE_RADIUS		48.5
//
// //----- These were cut from HDGeant/hitFDC.c -----
// #define DRIFT_SPEED           .0055
// #define CATHODE_ROT_ANGLE     1.309  // 75 degrees
// //#define CATHODE_ROT_ANGLE     0.7854 // 45 degrees
// #define WIRE_DEAD_ZONE_RADIUS 3.2
// #define ANODE_CATHODE_SPACING 0.5
// #define TWO_HIT_RESOL         250.0
// #define WIRES_PER_PLANE       96
// #define WIRE_SPACING          1.0
// #define U_OF_WIRE_ZERO        (-((WIRES_PER_PLANE-1)*WIRE_SPACING)/2)
// #define STRIPS_PER_PLANE      192
// #define STRIP_SPACING         0.5007
// #define U_OF_STRIP_ZERO		   (-((STRIPS_PER_PLANE-1)*STRIP_SPACING)/2)
// #define STRIP_ZERO_OFFSET     -95.5
// #define STRIP_GAP             0.1
// #define MAX_HITS             100
// //#define K2                  1.15
// #define STRIP_NODES           3
// #define THRESH_KEV           1.
// #define THRESH_STRIPS        5.   /* mV */
// #define ELECTRON_CHARGE 1.6022e-4 /* fC */
// //---------------------------------------------------


extern "C" {
void hitForwardDC (float xin[4], float xout[4],
                   float pin[5], float pout[5], float dEsum,
                   int track, int stack, int history, int ipart);
}


class CPPSensitiveDetectorFDC: public G4VSensitiveDetector
{
public:
	CPPSensitiveDetectorFDC(const G4String& name):G4VSensitiveDetector(name){
		collectionName.insert("fdctruthpoints");  // CPPHitFDCTruthPoint
		collectionName.insert("fdcanodehits");    // CPPHitFDCAnode
		collectionName.insert("fdccathodehits");  // CPPHitFDCCathode

		// These values obtained from ccdb_2014-01-22.sqlite with "jcalibread FDC/fdc_parms"

		ACTIVE_AREA_OUTER_RADIUS  = 48.5;
		ANODE_CATHODE_SPACING     = 0.5;
		CATHODE_SIGMA             = 150.0;
		DIFFUSION_COEFF           = 1.1e-6;
		DRIFT_SPEED               = .0055;
//		ELECTRON_CHARGE           = 1.6022e-4;
		HIT_DROP_FRACTION         = 0.0;
		K2                        = 1.15;
//		MAX_HITS                  = 100;
		PED_NOISE                 = 0.2;
		STRIPS_PER_PLANE          = 192;
		STRIP_DEAD_ZONE_RADIUS    = 5.0;
		STRIP_GAP                 = 0.1;
		STRIP_NODES               = 3;
		STRIP_SPACING             = 0.5;
		TDRIFT_SIGMA              = 1.0e-9;
		THRESHOLD_FACTOR          = 4.0;
		THRESH_KEV                = 1.;
		THRESH_STRIPS             = 0.001;
		TIME_WINDOW               = 1000.0;
		TWO_HIT_RESOL             = 25.;
		WIRES_PER_PLANE           = 96;
		WIRE_DEAD_ZONE_RADIUS     = 3.5;
		WIRE_SPACING              = 1.;

		// copied from hitFDC.c in hdgeant source
//		ELECTRON_CHARGE =1.6022e-4; /* fC */
		GAS_GAIN = 8e4;
		CATHODE_ROT_ANGLE = 1.309; // 75 degrees

		U_OF_WIRE_ZERO     = (-((WIRES_PER_PLANE-1.)*WIRE_SPACING)/2);
		U_OF_STRIP_ZERO    = (-((STRIPS_PER_PLANE-1.)*STRIP_SPACING)/2);

		wire_dead_zone_radius[0] = 3.0;
		wire_dead_zone_radius[1] = 3.0;
		wire_dead_zone_radius[2] = 3.9;
		wire_dead_zone_radius[3] = 3.9;

		strip_dead_zone_radius =  1.3;
	}

	virtual ~CPPSensitiveDetectorFDC(){}


	//------------------------------
	// Initialize
	void Initialize(G4HCofThisEvent* hitCollection)
	{
		if(hitCollection == (void*)0x1) cout<< "Huh?"; // avoid compiler warning

		hitsMapTruthPoint.clear();
		hitsMapAnode.clear();
		hitsMapCathode.clear();
	}

	//------------------------------
	// ProcessHits
	G4bool ProcessHits(G4Step* step, G4TouchableHistory* history)
	{
		// Physical volume hierarchy (for package 4):
		//
		// HALL_PV
		//   LASS_PV
		//     FDC_PV
		//       FDP4_PV
		//         FDM4_PV
		//           FDC4_PV     <-- wire/cathode plane rotations done here
		//             FDA4_PV   <-- active volume (step is in this volume)
		//             FDX4_PV   <-- wire material (inactive)
		//
		//  This method is called when stepping through the FDA1-FDA4 volumes.
		// (The Physical Volume names are FDA1_PV - FDA4_PV.)
		// If accessing a value through "touchable" such as touchable->GetVolume(N)
		// then the depth "N" is the number of ancestors to go back. e.g.
		// touchable->GetVolume(0) will be the FDA4_PV volume and
		// touchable->GetVolume(4) will be the FDC_PV volume.
		//
		// Conversely, methods of touchable->GetHistory() use the depth
		// or number of decendants starting from the volume the track was generated
		// in (at least that's how I think this works). If the particle is generated
		// in the LASS volume (which it tends to be) then e.g.
		// touchable->GetHistory()->GetTransform(4) will reference the transform
		// of the FDC4_PV volume (LASS_PV is depth 0). In those cases though it
		// is probably better to index it relative to current volume which we
		// know to be and FDA volume. e.g. depth = touchable->GetHistoryDepth()
		// touchable->GetHistory()->GetTransform(depth-2).
		//

		if(history == (void*)0x1) cout<< "Huh?"; // avoid compiler warning

		G4double dEsum = step->GetTotalEnergyDeposit() / GeV;

		if (dEsum==0. && MODE_ELOSS!=0) return false;

		// Get pre-step and post-step points in global coordinates, Geant4 units
		G4StepPoint *sp_pre = step->GetPreStepPoint();
		G4StepPoint *sp_post = step->GetPostStepPoint();
		const G4TouchableHandle &touchable = sp_pre->GetTouchableHandle();
		G4ThreeVector posGlobalpre  = sp_pre->GetPosition()  - LAB_OFFSET;
		G4ThreeVector posGlobalpost = sp_post->GetPosition() - LAB_OFFSET;

		// Get pre-step and post-step points in local coordinates and Geant4 units (mm)
		// We use the coordinates in the FDC1_PV, FDC2_PV, ... volume as the
		// local coordinate system. This has the chamber rotations relative to
		// the global, but also defines the wire plane to be at z=0.0.
		G4int depth = touchable->GetHistoryDepth();
		const G4AffineTransform &TransformGlobalToLocal = touchable->GetHistory()->GetTransform(depth-1);
		const G4AffineTransform &TransformLocalToGlobal = TransformGlobalToLocal.Inverse();
		G4ThreeVector posLocalpre  = TransformGlobalToLocal.TransformPoint(posGlobalpre  + LAB_OFFSET);
		G4ThreeVector posLocalpost = TransformGlobalToLocal.TransformPoint(posGlobalpost + LAB_OFFSET);

//printf("%s:%d ----------------------------------------\n", __FILE__,__LINE__);
//_DBG_<<"posGlobalpre : " << posGlobalpre  /cm << " phi=" << atan2(posGlobalpre.y(), posGlobalpre.x())*57.3 << endl;
//_DBG_<<"posLocalpre : " << posLocalpre  /cm << endl;
//_DBG_<<"posLocalpost: " << posLocalpost /cm << endl;

		// FDC numbering is done different for HDDM than for most everywhere
		// else in that is uses a module number (1-8) and "layer" within the
		// module (1-3). Usually we talk about package (1-4) and chamber (1-6).
		// The geometry defined in HDDS does not use repeating structures so
		// the value returned by GetCopyNumber(i) is always zero and of no use.
		// HDDS does use a number 1-4 in the naming of the sensitive gas volumes
		// corresponding to the 4 packages. Use this to determine package number.
		int package = 0;
		string volname = touchable->GetVolume(0)->GetName();  // FDA1_PV, FDA2_PV, ...
		switch(volname[3]){
			case '1' : package = 1; break;
			case '2' : package = 2; break;
			case '3' : package = 3; break;
			case '4' : package = 4; break;
		}

		// If mid-point of step is in dead zone for wire then ignore hit
		G4ThreeVector posLocalmid = 0.5*(posLocalpre + posLocalpost);
		double r = posLocalmid.perp() / cm;
		if (r < wire_dead_zone_radius[package - 1]) return false;

		// We use the z location of the 2nd level up in the history to determine
		// the chamber number (1-6) within the package. This is the "FDM4_PV" volume
		// and has a spacing of 2.1254cm.
		double tmp_z = touchable->GetHistory()->GetTransform(depth-2).TransformPoint(posGlobalpre + LAB_OFFSET).z();
		G4double chamber_location = floor(tmp_z/cm/2.1254) + 4.0; // 1.0-6.0
		G4int chamber = (int)chamber_location; // 1-6
		if(chamber<1 || chamber>6){
			_DBG_ << "Bad chamber number " << chamber << " (should be 1-6)" << endl;
			return false;
		}

		// Time at pre, post and mid step points
		double t_pre  = sp_pre->GetGlobalTime() / ns;
		double t_post = sp_post->GetGlobalTime() / ns;
		double t_mid = 0.5*(t_pre + t_post);

		// Other indexes
		int glayer = 3*6*(package-1) + 3*(chamber-1) + 2;  // 1-72  (2,5,8,11,14,17,20,23, ...)
		int layer = 1 + ((glayer-1)/3%3); // 1-3
		int module = (glayer-1)/9 + 1;     // 1-8
		int pdgtype = step->GetTrack()->GetDefinition()->GetPDGEncoding();
		int ptype = EParticle::PDGtoPType(pdgtype);
		int itrack = step->GetTrack()->GetTrackID();

		// Find wires closest to pre and post step points
		int wire1 = ceil((posLocalpre.x()/cm  - U_OF_WIRE_ZERO)/WIRE_SPACING +0.5);
		int wire2 = ceil((posLocalpost.x()/cm - U_OF_WIRE_ZERO)/WIRE_SPACING +0.5);
		if( (wire1 > WIRES_PER_PLANE) && (wire2 > WIRES_PER_PLANE) ) return false;
		if( (wire1 < 1) && (wire2 < 1)) return false;
		if(wire1 > wire2){
			int tmp = wire1;
			wire1 = wire2;
			wire2 = tmp;
		}

		// Force wire values to be in range
		if (wire1 > WIRES_PER_PLANE) wire1 = WIRES_PER_PLANE;
		if (wire2 > WIRES_PER_PLANE) wire2 = WIRES_PER_PLANE;
		if (wire1 < 1) wire1 = 1;
		if (wire2 < 1) wire2 = 1;

		// POCA (Point Of Closest Approach) to each wire is calculated in
		// x'/z plane where x' is direction perpendicular wires. This is
		// done in 2D in the local coordinate system with a little vector
		// algebra.
		//
		// dpos       - unit vector pointing from pre-step point to post-step point in local coordinates x'/z plane
		// delta_pos  - distance between pre-step and post-step in x'/z plane
		// w          - vector pointing from pre-step to wire position in x'/z plane
		// alpha      - distance from pre-step point to POCA in x'/z plane
		//
		G4ThreeVector Dpos = posLocalpost - posLocalpre; // 3D vector pointing from pre-step to post-step
		G4TwoVector dpos(Dpos.x(), Dpos.z());            // 2D vector of same in x'/z plane
		double delta_pos = dpos.mag(); // Geant4 units (i.e. mm)
		dpos *= 1.0/delta_pos;
		double zwire = 0.0;  // wire plane always at z=0.0 in the "local" coordinate system

		// OK, here is a little monkey shines. The code in hdgeant used the following
		// method to define its equivalent of cos_B_wires, the cosine of the angle between
		// the wire and the B-field in the wire plane. This code is left over from a long
		// time ago when wire planes had only 3 angles (0, +60, -60) as opposed to 6
		// (180, 120, 60, 0, -60, -120) as is defined in the modern geometry. The reconstruction
		// is consistent with how hdgeant produced the data and if we don't do it this
		// way, then the Lorentz effect is way off and the reconstruction doesn't work right.
		// Thus, some of our first attempt at doing this is commented out and the hdgeant
		// code adopted (tweaked slightly) to get it to work consistently with the reconstruction.

		double wire_theta=1.0472*(float)((layer%3)-1);
		double wire_dir[2];
		wire_dir[0]=sin(wire_theta);
		wire_dir[1]=cos(wire_theta);

		// Get magnetic field at midpoint of step
		G4ThreeVector posGlobalmid = TransformLocalToGlobal.TransformPoint(posLocalmid);
		G4ThreeVector B;
	    gCPPMagneticField->GetFieldValue(posGlobalmid, B); // B-field in global coordinates
		B *= 1.0/tesla; // convert to Tesla (GEANT 4 seems to use MegaTesla as the natural units??!!)
		double Br = B.perp();
		double Bz = B.z();
		double phi_wires = Br>0.0 ? (acos((B.x()*wire_dir[0]+B.y()*wire_dir[1])/Br)):0.0;
		double cos_B_wires = cos(phi_wires);
//		double phi_wires = posLocalpre.deltaPhi(posGlobalpre); // phi angle of wires in global coordinates
//		B.rotateZ(phi_wires); // rotate B-field into local coordinate system
//		double cos_B_wires = B.y()/Br;

//printf("%s:%d phiB_global: %f  layer=%d wire_theta=%f\n",__FILE__,__LINE__, atan2(B.y(), B.x()), layer, wire_theta*57.3);
//printf("%s:%d Br: %f Bz: %f cos_B_wires: %f phi= %f\n",__FILE__,__LINE__, Br, B.z(),cos_B_wires, phi_wires*57.3);

		// Check if this track has been recorded in this chamber in hitsMapTruthPoint and if not, add it
		G4int truthidx = layer + 10*module + 100*itrack;
		if(hitsMapTruthPoint.find(truthidx) == hitsMapTruthPoint.end()){

			double E = sp_pre->GetTotalEnergy() / GeV;
			G4ThreeVector mom = sp_pre->GetMomentum() / GeV;
			G4ThreeVector pos = (sp_pre->GetPosition() - LAB_OFFSET) / cm;

			double dr = (posLocalpost - posLocalpre).mag() / cm;
			double dEdx = 0.0;
			if( dr > 1e-3 ) dEdx = dEsum/dr;

			double x = 0.5*(posLocalpost + posLocalpre).x();
			int wire = ceil((x - U_OF_WIRE_ZERO)/WIRE_SPACING +0.5);
			double xwire = U_OF_WIRE_ZERO + (wire-1)*WIRE_SPACING;
			double delta_x = (posLocalpost - posLocalpre).x();
			double delta_z = (posLocalpost - posLocalpre).z();
			double alpha = atan2(delta_x, delta_z);
			double u[2];
			u[0] = posLocalpre.z();
			u[1] = posLocalpre.x() - xwire;
			double dradius = fabs( u[1]*cos(alpha)-u[0]*sin(alpha) ) / cm;

			CPPHitFDCTruthPoint *fdcpart = new CPPHitFDCTruthPoint(module, layer, mom, pos);
			fdcpart->E = E;
			fdcpart->dEdx = dEdx;
			fdcpart->dradius = dradius;
			fdcpart->t = t_pre;
			fdcpart->primary = 0; // This is overwritten in CPPHDDMout where Nprimaries is available
			fdcpart->ptype = ptype;
			fdcpart->track = itrack;
			fdcpart->itrack = itrack;

			hitsMapTruthPoint[truthidx] = fdcpart;
		}


//		// Copy the entrance and exit x-values into x1 and x2 making sure x1<=x2
//		float x1 = pre.x()  / cm;
//		float x2 = post.x() / cm;
//		float y1 = pre.y()  / cm;
//		float y2 = post.y() / cm;
//		float z1 = pre.y()  / cm;
//		float z2 = post.y() / cm;
//		if(x2 < x1){
//			float tmp = x1;
//			x1 = x2;
//			x2 = tmp;
//
//			tmp = y1;
//			y1 = y2;
//			y2 = tmp;
//
//			tmp = z1;
//			z1 = z2;
//			z2 = tmp;
//		}

//
//		// The energy of the step needs to be distributed to all wires hit.
//		// We do this by assuming the fraction of energy deposited in a cell
//		// is equal to the fraction of the total pathlength in the cell.
//		// more specifically, the total pathlength in the local x-direction.
//		// In principle, the step can span several wires. The end wires are the
//		// ones for which the step spans only a portion of the cell. First,
//		// calculate these fractions.
//		float xwire1 = U_OF_WIRE_ZERO + (wire1-1)*WIRE_SPACING;
//		float xwire2 = U_OF_WIRE_ZERO + (wire2-1)*WIRE_SPACING;
//		float xwire1_inner = xwire1+0.5*WIRE_SPACING;
//		float xwire2_inner = xwire2-0.5*WIRE_SPACING;
//
//		float frac1 = fabs(x1 - xwire1_inner)/WIRE_SPACING; // fraction of cell step crossed
//		float frac2 = fabs(x2 - xwire2_inner)/WIRE_SPACING; // fraction of cell step crossed
//
//		// Total distance traversed by the step in the local x-direction
//		// in units of cells.
//		float total_dist_cells = frac1 + frac2 + (float)(wire2-wire1-1);

//		// Calculate slope and intercept of trajectory projection onto local x/y plane.
//		// We need this to calculate the y-coordinate of the trajectory near each wire
//		// in order to get the B-field there which is needed for Lorentz corrections.
//		float xy_slope = (y2-y1)/(x2-x1);
//		float xy_inter = y1 - xy_slope*x1;
//		float xz_slope = (z2-z1)/(x2-x1);
//		float xz_inter = z1 - xz_slope*x1;
//		if(x1==x2){
//			xy_slope = 0.0;
//			xy_inter = (y1+y2)/2.0;
//			xz_slope = 0.0;
//			xz_inter = (z1+z2)/2.0;
//		}

 		// Loop over all wires hit by this track segment, distributing
		// energy evenly based on path length through the cell.
		for(int wire=wire1; wire<=wire2; wire++){

//			float frac = 1.0; // fraction of this cell the step crossed
//			if(wire  == wire1) frac = frac1;  // if this is the first wire, fraction of cell crossed
//			if(wire  == wire2) frac = frac2;  // if this is the last wire, fraction of cell crossed
//			if(wire1 == wire2) frac = total_dist_cells; // If first and last wires are same, make this the whole step.

			// Get location of wire
			double xwire = U_OF_WIRE_ZERO + (wire-1)*WIRE_SPACING;

			// Calculate POCA to wire in x'/z plane
			G4TwoVector posLocalpre2D(posLocalpre.x()/cm, posLocalpre.z()/cm);
			G4TwoVector w(G4TwoVector(xwire, zwire) - posLocalpre2D);  // cm
			double alpha = w.dot(dpos);    // cm
			G4TwoVector poca = dpos*alpha; // cm
			double d = poca.x() - w.x();   // cm

			// Get POCA point along wire
			double poca_along_wire = ( Dpos.y() * alpha/Dpos.mag() + posLocalpre.y()) / cm;

			// useful combinations of dx and dz
			double poca_z = poca.y() + posLocalpre.z()/cm; // cm  "y"-coordinate of poca 2D vector is z-coordinate
			double dx=d;
			double dx2=dx*dx;
			double dx4=dx2*dx2;
			double dz2=poca_z*poca_z;
			double dz4=dz2*dz2;

			// Next compute the avalanche position along wire.
			// Correct avalanche position with deflection along wire due to
			// Lorentz force.
//printf("%s:%d     >>> poca_along_wire(pre)=%f   poca_z=%f\n", __FILE__,__LINE__, poca_along_wire, poca_z);
			poca_along_wire += ( -0.125*Bz*(1.-0.048*Br) )*dx
			  + (-0.18-0.0129*(Bz))*(Br*cos_B_wires)*poca_z
			  + ( -0.000176 )*dx*dx2/(dz2+0.001);

			// Add transverse diffusion
			double rndno[2];
			rndno[0] = CLHEP::RandGauss::shoot();
			rndno[1] = CLHEP::RandGauss::shoot();
			poca_along_wire += (( 0.01 )*pow(dx2+dz2,0.125)+( 0.0061 )*dx2)*rndno[0];
//printf("%s:%d     <<< poca_along_wire(post)=%f dx=%f dz2=%f\n", __FILE__,__LINE__,poca_along_wire,dx,dz2);

			// Do not use this cluster if the Lorentz force would deflect
			// the electrons outside the active region of the detector
			double poca_r = sqrt(pow(xwire,2.0) + pow(poca_along_wire, 2.0)); // radius of hit in cm
			if( poca_r > ACTIVE_AREA_OUTER_RADIUS ) continue;

			// Model the drift time and longitudinal diffusion as a function of
			// position of the cluster within the cell
			double tdrift_unsmeared=1086.0*(1.+0.039*B.mag())*dx2+( 1068.0 )*dz2
			  +dx4*(( -2.675 )/(dz2+0.001)+( 2.4e4 )*dz2);
			double dt=(( 39.44   )*dx4/(0.5-dz2)+( 56.0  )*dz4/(0.5-dx2)
			  + ( 0.01566 )*dx4/(dz4+0.002)/(0.251-dx2))*rndno[1];

			// Minimum drift time for docas near wire (very crude approximation)
			double v_max=0.08; // guess for now based on Garfield, near wire
			double dradius=sqrt(dx2+dz2);
			double tmin=dradius/v_max;
			double tdrift_smeared=tdrift_unsmeared+dt;
			if( tdrift_smeared < tmin ) tdrift_smeared=tmin;

			// Avalanche time
			double tdrift = t_mid + tdrift_smeared;

			// Skip cluster if the time would go beyond readout window
			if ( tdrift > TIME_WINDOW ) continue;

			// Get existing hits (if any) for this wire and either merge this one
			// with an existing one or create a new one. It is worth noting that
			// this follows how it was done in hdgeant (GEANT3). The geometry is
			// defined such that there is a thin plane of inactive material representing
			// the wires (FDX1, FDX2, ...) This causes GEANT to take two steps across
			// the cell: one up to the wire plane and one from the wire plane to the
			// other side. Thus, for a normal track, this method gets called twice
			// for a single wire. The hits from these two calls should always be within
			// the TWO_HIT_RESOL value so that they get merged.
			G4int idx = 1000*glayer + wire;
			vector<CPPHitFDCAnode*> &hits = hitsMapAnode[idx];
			CPPHitFDCAnode *hit = NULL;
			for(uint32_t i=0; i<hits.size(); i++){
				if( fabs(hits[i]->t - tdrift) < TWO_HIT_RESOL){
					hit = hits[i];
					break;
				}
			}
			if(!hit){
				hit = new CPPHitFDCAnode(layer, module, wire, itrack, ptype);
				hits.push_back(hit);
			}
			if(tdrift < hit->t){
				hit->t = tdrift;
				hit->t_unsmeared = tdrift_unsmeared;
				hit->d = d;
			}
			hit->dE += dEsum; // FIXME! This needs to be the fraction of energy in this wire's cell

			// Add in cathode strip hits corresponding to this wire hit
			// n.b. this will routinely get called twice for each track crossing a cell for
			// the same reason described above. The two hits on each strip should very often
			// get merged though.
			AddFDCCathodeHits(xwire, poca_along_wire, tdrift, dEsum, itrack, ptype, chamber, module, layer);

		} // Loop over wire

		return false;
	}


	//------------------------------
	// AddFDCCathodeHits
	bool AddFDCCathodeHits(double xwire, double avalanche_y, double tdrift,
		       double dE, int itrack, int ptype, int chamber, int module, int layer){

		// Anode charge
		double q_anode;
		int n_t;

		// Average number of secondary ion pairs for 40/60 Ar/CO2 mixture
		double n_s_per_p=1.89;

		// Find the number of primary ion pairs:
		// The total number of ion pairs depends on the energy deposition
		// and the effective average energy to produce a pair, w_eff.
		// On average for each primary ion pair produced there are n_s_per_p
		// secondary ion pairs produced.

		//Average energy needed to produce an ion pair for 50/50 mixture
		double w_eff=30.2e-9; // GeV
		// Average number of primary ion pairs
		double n_p_mean = dE/w_eff/(1.+n_s_per_p);
		int n_p = CLHEP::RandPoisson::shoot(n_p_mean); // number of primary ion pairs (was gpoiss_(&n_p_mean,&n_p,&one) in hdgeant)

		// Total number of ion pairs.  On average for each primary ion
		// pair produced there are n_s secondary ion pairs produced.  The
		// probability distribution is a compound poisson distribution
		// that requires generating two Poisson variables.
		double n_s_mean = ((double)n_p)*n_s_per_p;
		int n_s = CLHEP::RandPoisson::shoot(n_s_mean);  // in GEANT3 this was gpoiss_(&n_s_mean,&n_s,&one);
		n_t = n_s + n_p;
		q_anode=((double)n_t)*GAS_GAIN*ELECTRON_CHARGE;

		// Mock-up of cathode strip charge distribution
		int plane, node;
		for (plane=1; plane<4; plane+=2){
			double theta = (plane == 1)? M_PI-CATHODE_ROT_ANGLE : CATHODE_ROT_ANGLE;
			double cathode_u = -xwire*cos(theta) - avalanche_y*sin(theta);
			int strip1 = ceil((cathode_u-U_OF_STRIP_ZERO)/STRIP_SPACING +0.5);
			double cathode_u1 = (strip1-1)*STRIP_SPACING + U_OF_STRIP_ZERO;
			double delta = cathode_u-cathode_u1;
			double half_gap=ANODE_CATHODE_SPACING;
			for (node=-STRIP_NODES; node<=STRIP_NODES; node++){
				// Induce charge on the strips according to the Mathieson
				// function tuned to results from FDC prototype
				double lambda1=(((double)node-0.5)*STRIP_SPACING+STRIP_GAP/2. -delta)/half_gap;
				double lambda2=(((double)node+0.5)*STRIP_SPACING-STRIP_GAP/2. -delta)/half_gap;
				double factor=0.25*M_PI*K2;
				double q = 0.25*q_anode*(tanh(factor*lambda2)-tanh(factor*lambda1));

				int strip = strip1+node;

				// Throw away hits on strips falling within a certain dead-zone radius
				double strip_outer_u=cathode_u1 + (STRIP_SPACING+STRIP_GAP/2.)*(int)node;
				double cathode_v=-xwire*sin(theta)+avalanche_y*cos(theta);
				double check_radius=sqrt(strip_outer_u*strip_outer_u + cathode_v*cathode_v);

				if ((strip > 0) && (check_radius>strip_dead_zone_radius) && (strip <= STRIPS_PER_PLANE)){

					G4int idx = 100000*module + 10000*plane + 1000*layer + strip;
					vector<CPPHitFDCCathode*> &hits = hitsMapCathode[idx];
					CPPHitFDCCathode *hit = NULL;
					for(uint32_t i=0; i<hits.size(); i++){
						if( fabs(hits[i]->t - tdrift) < TWO_HIT_RESOL){
							hit = hits[i];
							break;
						}
					}
					if(!hit){
						hit = new CPPHitFDCCathode(layer, module, plane, strip, itrack, ptype);
						hits.push_back(hit);
					}
					if(tdrift < hit->t){
						hit->t = tdrift;
					}
					hit->q += q;
				}
			} // loop over cathode strips

		} // loop over cathode views

		return false;
	}

	//------------------------------
	// EndOfEvent
	void EndOfEvent(G4HCofThisEvent* hitCollection)
	{
		// -------- TRUTH POINTS --------
		if(!hitsMapTruthPoint.empty()){

			// Create hits collection
			CPPHitFDCTruthPointCollection *aHC = new CPPHitFDCTruthPointCollection("FDC", "fdctruthpoints");

			map<G4int, CPPHitFDCTruthPoint* >::iterator it;
			for(it=hitsMapTruthPoint.begin(); it!=hitsMapTruthPoint.end(); it++){
				CPPHitFDCTruthPoint *hit = it->second;
				aHC->insert(hit); // add hit to collection
			}

			// Add hit collection to the event
			G4int HCID = GetCollectionID(0); // get ID of FDCtruthpoints
			hitCollection->AddHitsCollection(HCID, aHC);
		}

		// -------- ANODE HITS --------
		if(!hitsMapAnode.empty()){

			// Create hits collection
			CPPHitFDCAnodeCollection *aHC = new CPPHitFDCAnodeCollection("FDCAnode", "fdcanodehits");

			// Add selected hits to the hits collection
			map<G4int, vector<CPPHitFDCAnode*> >::iterator iter;
			for(iter=hitsMapAnode.begin(); iter!=hitsMapAnode.end(); iter++){
				vector<CPPHitFDCAnode*> &hits = iter->second;
				for(unsigned int j=0; j<hits.size(); j++) {

					// Here we have an opportunity to discard the hit if we wanted to.
					// (but we don't)
					aHC->insert(hits[j]); // add hit to collection
				}
			}

			// Add hit collection to the event
			G4int HCID = GetCollectionID(1); // get first ID of this sensitive detector
			hitCollection->AddHitsCollection(HCID, aHC);
		}

		// -------- CATHODE HITS --------
		if(!hitsMapCathode.empty()){

			// Create hits collection
			CPPHitFDCCathodeCollection *aHC = new CPPHitFDCCathodeCollection("FDCCathode", "fdccathodehits");

			// Add selected hits to the hits collection
			map<G4int, vector<CPPHitFDCCathode*> >::iterator iter;
			for(iter=hitsMapCathode.begin(); iter!=hitsMapCathode.end(); iter++){
				vector<CPPHitFDCCathode*> &hits = iter->second;
				for(unsigned int j=0; j<hits.size(); j++) {

					// Here we have an opportunity to discard the hit if we wanted to.
					// (but we don't)
					aHC->insert(hits[j]); // add hit to collection
				}
			}

			// Add hit collection to the event
			G4int HCID = GetCollectionID(2); // get first ID of this sensitive detector
			hitCollection->AddHitsCollection(HCID, aHC);
		}
	}

private:
	map<G4int, CPPHitFDCTruthPoint* > hitsMapTruthPoint;
	map<G4int, vector<CPPHitFDCAnode*> > hitsMapAnode;
	map<G4int, vector<CPPHitFDCCathode*> > hitsMapCathode;

	G4double ACTIVE_AREA_OUTER_RADIUS;
	G4double ANODE_CATHODE_SPACING;
	G4double CATHODE_SIGMA;
	G4double DIFFUSION_COEFF;
	G4double DRIFT_SPEED;
	G4double ELECTRON_CHARGE;
	G4double HIT_DROP_FRACTION;
	G4double K2;
	// G4int    MAX_HITS;
	G4double PED_NOISE;
	G4double STRIPS_PER_PLANE;
	G4double STRIP_DEAD_ZONE_RADIUS;
	G4double STRIP_GAP;
	G4double STRIP_NODES;
	G4double STRIP_SPACING;
	G4double TDRIFT_SIGMA;
	G4double THRESHOLD_FACTOR;
	G4double THRESH_KEV;
	G4double THRESH_STRIPS;
	G4double TIME_WINDOW;
	G4double TWO_HIT_RESOL;
	G4int    WIRES_PER_PLANE;
	G4double WIRE_DEAD_ZONE_RADIUS;
	G4double WIRE_SPACING;

//	G4double ELECTRON_CHARGE;
	G4double GAS_GAIN;
	G4double CATHODE_ROT_ANGLE;

	G4double U_OF_WIRE_ZERO;
	G4double U_OF_STRIP_ZERO;

	G4double wire_dead_zone_radius[4];
	G4double strip_dead_zone_radius;

};


#endif // _CPPSENSITIVEDETECTORFDC_
