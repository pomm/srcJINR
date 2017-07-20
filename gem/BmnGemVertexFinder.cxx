#include "BmnGemVertexFinder.h"
#include "BmnMath.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TFitResult.h"
#include "vector"

static Float_t workTime = 0.0;

using namespace std;
using namespace TMath;

BmnGemVertexFinder::BmnGemVertexFinder() : fEventNo(0) {

    fGemHitsArray = NULL;
    fGemTracksArray = NULL;
    fKalman = NULL;
    fNTracks = 0;
    fIsField = kTRUE;
    fField = NULL;
    fHitsBranchName = "BmnGemStripHit";
    fTracksBranchName = "BmnGemTrack";
    fVertexBranchName = "BmnVertex";
}

BmnGemVertexFinder::~BmnGemVertexFinder() {
}

InitStatus BmnGemVertexFinder::Init() {

    if (fVerbose) cout << "=========================== Vertex finder init started ====================" << endl;

    //Get ROOT Manager
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) {
        Fatal("Init", "FairRootManager is not instantiated");
    }

    fGemHitsArray = (TClonesArray*) ioman->GetObject(fHitsBranchName); //in
    fGemTracksArray = (TClonesArray*) ioman->GetObject(fTracksBranchName); //in
    fVertexArray = new TClonesArray("CbmVertex", 100); //out
    ioman->Register(fVertexBranchName, "GEM", fVertexArray, kTRUE);

    fField = FairRunAna::Instance()->GetField();
    fDetector = new BmnGemStripStationSet_RunSpring2017(BmnGemStripConfiguration::RunSpring2017);

    if (fVerbose) cout << "=========================== Vertex finder init finished ===================" << endl;
}

void BmnGemVertexFinder::Exec(Option_t* opt) {
    clock_t tStart = clock();

    if (fVerbose) cout << "======================== Vertex finder exec started  ======================" << endl;
    if (fVerbose) cout << "Event number: " << fEventNo++ << endl;

    fVertexArray->Delete();

    fNTracks = fGemTracksArray->GetEntriesFast();

    if (fNTracks > 1) FindVertexByVirtualPlanes();

    if (fVerbose) cout << "\n======================== Vertex finder exec finished ======================" << endl;

    clock_t tFinish = clock();
    workTime += ((Float_t) (tFinish - tStart)) / CLOCKS_PER_SEC;
}

Float_t BmnGemVertexFinder::FindVZByVirtualPlanes(Float_t z_0, Float_t range) {

    fKalman = new BmnKalmanFilter_tmp();

    const Int_t nPlanes = 5;
    Float_t minZ = 0;

    while (range >= 0.1) {
        Float_t zMax = z_0 + range;
        Float_t zMin = z_0 - range;
        Float_t zStep = (zMax - zMin) / nPlanes;

        vector<TVector3> PlaneHits[nPlanes];
        Float_t zPlane[nPlanes];
        Float_t dist[nPlanes];

        for (Int_t iPlane = 0; iPlane < nPlanes; ++iPlane) {
            zPlane[iPlane] = zMax - iPlane * zStep;
            dist[iPlane] = 0.0;
        }

        TString propagationType = (fIsField) ? "field" : "line";

        for (Int_t iTr = 0; iTr < fNTracks; ++iTr) {
            BmnGemTrack* track = (BmnGemTrack*) fGemTracksArray->At(iTr);
            if (track->GetFlag() == kBMNBAD) continue;
            FairTrackParam par0 = *(track->GetParamFirst());

            for (Int_t iPlane = 0; iPlane < nPlanes; ++iPlane) {
                fKalman->TGeoTrackPropagate(&par0, zPlane[iPlane], 2212, NULL, NULL, propagationType);
                PlaneHits[iPlane].push_back(TVector3(par0.GetX(), par0.GetY(), par0.GetZ()));
            }
        }

        for (Int_t iPlane = 0; iPlane < nPlanes; ++iPlane) {
            Int_t nPairs = 0;
            for (Int_t i = 0; i < fNTracks; ++i) {
                TVector3 vI = PlaneHits[iPlane].at(i);
                for (Int_t j = i + 1; j < fNTracks; ++j) {
                    TVector3 vJ = PlaneHits[iPlane].at(j);
                    dist[iPlane] += Sqrt(Sqr(vI.X() - vJ.X()) + Sqr(vI.Y() - vJ.Y()));
                    nPairs++;
                }
            }
            dist[iPlane] /= nPairs;
        }

        Float_t minDist = FLT_MAX;
        minZ = FLT_MAX;
        for (Int_t iPlane = 0; iPlane < nPlanes; ++iPlane) {
            if (dist[iPlane] < minDist) {
                minZ = zPlane[iPlane];
                minDist = dist[iPlane];
            }
        }

        TGraph* vertex = new TGraph(nPlanes, zPlane, dist);
        vertex->Fit("pol2", "QF");
        TF1 *fit_func = vertex->GetFunction("pol2");
        Float_t b = fit_func->GetParameter(1);
        Float_t a = fit_func->GetParameter(2);
        z_0 = -b / (2 * a);
        range /= 2;
    }
    return (minZ);
}

void BmnGemVertexFinder::FindVertexByVirtualPlanes() {

    Float_t vz = FindVZByVirtualPlanes(fRoughVertex, 50.0);
    Float_t vx = 0.0;
    Float_t vy = 0.0;
    UInt_t nOk = 0;

    for (Int_t iTr = 0; iTr < fNTracks; ++iTr) {
        BmnGemTrack* track = (BmnGemTrack*) fGemTracksArray->At(iTr);
        if (track->GetFlag() == kBMNBAD) continue;
        FairTrackParam par0 = *(track->GetParamFirst());

        TString propagationType = (fIsField) ? "field" : "line";
        fKalman->TGeoTrackPropagate(&par0, vz, 2212, NULL, NULL, propagationType);
        vx += par0.GetX();
        vy += par0.GetY();
        track->SetB(Sqrt(par0.GetX() * par0.GetX() + par0.GetY() * par0.GetY())); //impact parameter
        nOk++;
    }

    vx /= nOk;
    vy /= nOk;

   if (Abs(vz - fRoughVertex) < 2.5) //2.5 - the best range from MC simulations and reconstruction (by A.Zelenow) 
       new((*fVertexArray)[fVertexArray->GetEntriesFast()]) CbmVertex("vertex", "vertex", vx, vy, vz, 0.0, 0, fNTracks, TMatrixFSym(3));
    delete fKalman;
}

void BmnGemVertexFinder::Finish() {
    ofstream outFile;
    outFile.open("QA/timing.txt", ofstream::app);
    outFile << "Vertex Finder Time: " << workTime;
    cout << "Work time of the GEM vertex finder: " << workTime << endl;
}