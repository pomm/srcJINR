// -------------------------------------------------------------------------
// -----                        FairMCTracks source file                  -----
// -----                  Created 10/12/07  by M. Al-Turany            -----
// -------------------------------------------------------------------------
#include "FairMCTracks.h"

#include "FairEventManagerEditor.h"
#include "FairRootManager.h"
#include "FairLogger.h"

#include "TEveManager.h"    // for gEve
#include "TEvePathMark.h"
#include "TEveVector.h"
#include "TGeoTrack.h"
#include "TMathBase.h"
#include "TObjArray.h"

#include <string.h>
#include <iostream>
using namespace std;


// -----   Default constructor   -------------------------------------------
FairMCTracks::FairMCTracks()
  : FairTask("FairMCTracks", 0),
    fTrackList(NULL),
    fTrPr(NULL),
    fEventManager(NULL),
    fEveTrList(NULL),
    fTrList(NULL),
    MinEnergyLimit(-1.),
    MaxEnergyLimit(-1.),
    PEnergy(-1.)
{
}

// -----   Standard constructor   ------------------------------------------
FairMCTracks::FairMCTracks(const char* name, Int_t iVerbose)
  : FairTask(name, iVerbose),
    fTrackList(NULL),
    fTrPr(NULL),
    fEventManager(NULL),
    fEveTrList(new TObjArray(16)),
    fTrList(NULL),
    MinEnergyLimit(-1.),
    MaxEnergyLimit(-1.),
    PEnergy(-1.)
{
}

// -----   Destructor   ----------------------------------------------------
FairMCTracks::~FairMCTracks()
{
}

// -------------------------------------------------------------------------
InitStatus FairMCTracks::Init()
{
  LOG(DEBUG)<<"FairMCTracks::Init()"<<FairLogger::endl;

  FairRootManager* fManager = FairRootManager::Instance();
  fEventManager = FairEventManager::Instance();
  LOG(DEBUG1) <<  "FairMCTracks::Init() get instance of FairEventManager " << FairLogger::endl;

  fTrackList = (TClonesArray*) fManager->GetObject("GeoTracks");
  if (fTrackList == 0)
  {
    LOG(ERROR)<<"FairMCTracks::Init() branch "<<GetName()<<" not found! Task will be deactivated"<<FairLogger::endl;
    SetActive(kFALSE);
  }
  LOG(DEBUG1)<<"FairMCTracks::Init() get track list "<<fTrackList<<FairLogger::endl;

  MinEnergyLimit = fEventManager->GetEvtMinEnergy();
  MaxEnergyLimit = fEventManager->GetEvtMaxEnergy();
  PEnergy = 0;

  if (IsActive()) return kSUCCESS;
  else return kERROR;
}

// -------------------------------------------------------------------------
void FairMCTracks::Exec(Option_t* /*option*/)
{
  if (!IsActive())
      return;
  LOG(DEBUG1)<<"FairMCTracks::Exec"<<FairLogger::endl;

  Reset();

  TGeoTrack* tr;
  const Double_t* point;
  for (Int_t i = 0; i < fTrackList->GetEntriesFast(); i++)
  {
    LOG(DEBUG3)<<"FairMCTracks::Exec "<<i<<FairLogger::endl;
    tr = (TGeoTrack*) fTrackList->At(i);

    TParticle* P = (TParticle*) tr->GetParticle();
    PEnergy = P->Energy();
    MinEnergyLimit = TMath::Min(PEnergy, MinEnergyLimit);
    MaxEnergyLimit = TMath::Max(PEnergy, MaxEnergyLimit);
    LOG(DEBUG3)<<"MinEnergyLimit "<<MinEnergyLimit<<" MaxEnergyLimit "<<MaxEnergyLimit<<FairLogger::endl;

    if ((fEventManager->IsPriOnly() && (P->GetMother(0) > -1)))
        continue;
    if ((fEventManager->GetCurrentPDG() != 0) && (fEventManager->GetCurrentPDG() != tr->GetPDG()))
        continue;
    LOG(DEBUG3)<<"PEnergy "<<PEnergy<<" Min "<<fEventManager->GetMinEnergy()<<" Max "<<fEventManager->GetMaxEnergy()<<FairLogger::endl;
    if ((PEnergy < fEventManager->GetMinEnergy()) || (PEnergy > fEventManager->GetMaxEnergy()))
        continue;

    fTrList = GetTrGroup(P);
    TEveTrack* track = new TEveTrack(P, tr->GetPDG(), fTrPr);
    track->SetLineColor(fEventManager->Color(tr->GetPDG()));

    Int_t Np = tr->GetNpoints();
    for (Int_t n = 0; n < Np; n++)
    {
        point = tr->GetPoint(n);

        track->SetPoint(n, point[0], point[1], point[2]);
        TEveVector pos = TEveVector(point[0], point[1], point[2]);

        TEvePathMark* path = new TEvePathMark();
        path->fV = pos;
        path->fTime = point[3];
        if (n == 0)
        {
          TEveVector Mom = TEveVector(P->Px(), P->Py(), P->Pz());
          path->fP = Mom;
        }

        track->AddPathMark(*path);
        LOG(DEBUG4)<<"Path marker added "<<path<<FairLogger::endl;

        delete path;
    }

    fTrList->AddElement(track);
    LOG(DEBUG3)<<"Track added "<<track->GetName()<<FairLogger::endl;
  }

  //for (Int_t i = 0; i < fEveTrList->GetEntriesFast(); i++)
  //{
  //  TEveTrackList* TrListIn = (TEveTrackList*) fEveTrList->At(i);
  //  TrListIn->FindMomentumLimits(TrListIn, kFALSE);
  //}
  fEventManager->SetEvtMinEnergy(MinEnergyLimit);
  fEventManager->SetEvtMaxEnergy(MaxEnergyLimit);

  //gEve->Redraw3D(kFALSE);
}

// -------------------------------------------------------------------------
void FairMCTracks::SetParContainers()
{
}

// -------------------------------------------------------------------------
void FairMCTracks::Finish()
{
}

// -------------------------------------------------------------------------
void FairMCTracks::Reset()
{
  for (Int_t i = 0; i < fEveTrList->GetEntriesFast(); i++)
  {
    TEveTrackList* ele = (TEveTrackList*) fEveTrList->At(i);
    gEve->RemoveElement(ele, fEventManager->EveMCTracks);
  }
  fEveTrList->Clear();
}

TEveTrackList* FairMCTracks::GetTrGroup(TParticle* P)
{
  fTrList = 0;
  for (Int_t i = 0; i < fEveTrList->GetEntriesFast(); i++)
  {
    TEveTrackList* TrListIn = (TEveTrackList*) fEveTrList->At(i);
    if (strcmp(TrListIn->GetName(), P->GetName()) == 0)
    {
      fTrList = TrListIn;
      break;
    }
  }

  if (fTrList == 0)
  {
    fTrPr = new TEveTrackPropagator();
    fTrList = new TEveTrackList(P->GetName(), fTrPr);
    // set track color by particle PDG from FairEventManager
    fTrList->SetMainColor(fEventManager->Color(P->GetPdgCode()));
    fTrList->SetRnrLine(kTRUE);
    fEveTrList->Add(fTrList);

    if (fEventManager->EveMCTracks == NULL)
    {
        fEventManager->EveMCTracks = new TEveElementList("MC tracks");
        gEve->AddElement(fEventManager->EveMCTracks, fEventManager);
        fEventManager->EveMCTracks->SetRnrState(kFALSE);
        fEventManager->GetEventEditor()->fShowMCTracks->SetEnabled(kTRUE);
    }
    gEve->AddElement(fTrList, fEventManager->EveMCTracks);
  }

  return fTrList;
}

ClassImp(FairMCTracks)
