// -------------------------------------------------------------------------
// -----                      CbmPsdPoint source file                 -----
// -----                   Created 28/07/04  by V. Friese              -----
// -------------------------------------------------------------------------


#include "CbmPsdPoint.h"

#include <iostream>
using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmPsdPoint::CbmPsdPoint() 
  : FairMCPoint(), 
    fModuleID(0)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmPsdPoint::CbmPsdPoint(Int_t trackID, Int_t detID, TVector3 pos, 
			   TVector3 mom, Double_t tof, Double_t length, 
			   Double_t eLoss)
  : FairMCPoint(trackID, detID, pos, mom, tof, length, eLoss), 
    fModuleID(0)
{
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmPsdPoint::~CbmPsdPoint() { }
// -------------------------------------------------------------------------



// -----   Public method Print   -------------------------------------------
void CbmPsdPoint::Print(const Option_t* opt) const {
  cout << "-I- CbmPsdPoint: ECAL point for track " << fTrackID 
       << " in detector " << fDetectorID << endl;
  cout << "    Position (" << fX << ", " << fY << ", " << fZ
       << ") cm" << endl;
  cout << "    Momentum (" << fPx << ", " << fPy << ", " << fPz
       << ") GeV" << endl;
  cout << "    Time " << fTime << " ns,  Length " << fLength 
       << " cm,  Energy loss " << fELoss*1.0e06 << " keV" << endl;
}
// -------------------------------------------------------------------------


ClassImp(CbmPsdPoint)
