//--------------------------------------------------------------------------------------------------------------------------------------
#ifndef __BmnLANDHitProducer_H
#define __BmnLANDHitProducer_H 1

#include <TClonesArray.h>
#include <FairTask.h>
#include <TVector3.h>
#include <math.h>
#include "BmnLANDGeometry.h"
#include "BmnLANDHit.h"
#include "BmnTrigDigit.h"
#include "BmnLANDDigit.h"

#define LAND_PLANE_N 6
#define LAND_BAR_N 20

class TRandom2;
class TEfficiency;
class TH1D;
class TH2D;
//class BmnTof1GeoUtils;
//--------------------------------------------------------------------------------------------------------------------------------------
class BmnLANDHitProducer : public FairTask 
{
protected:
	TClonesArray		*fLandDigits; // Digits.
	TClonesArray		*fLandHits;   // Hits.
	TClonesArray		*fLandCands;  // Neutron candidates.

	Bool_t			fUseMCData;
public:
	BmnLANDHitProducer(const char *name = "LAND HitProducer", Bool_t useMCdata = true, Int_t verbose = 1, Bool_t DoTest = false);
	virtual ~BmnLANDHitProducer();

	virtual InitStatus 	Init();
	virtual void 		Exec(Option_t* opt);
	virtual void 		Finish();

	bool SetVelMap(TString filename);

private:
	struct VScint {
	  VScint(): vscint(1) {}
	  Float_t vscint;
	};
	VScint m_vscint[LAND_PLANE_N][LAND_BAR_N];
	BmnLANDGeometry * fLANDGeometry;

ClassDef(BmnLANDHitProducer, 2);
};

#endif
//--------------------------------------------------------------------------------------------------------------------------------------

