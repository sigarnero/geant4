#include "TFile.h"
#include "TTree.h"
#include <set>
#include <iostream>
#include "TCanvas.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TH2F.h"
#include "TLegend.h"
#include "TBox.h"
#include "TLatex.h"

void check_multiplicity(const char* filename, int eventID) {

    TFile *f = TFile::Open(filename);
    TTree *tExit = (TTree*)f->Get("RadiatorExit");

    Int_t fEvent, fTrackID;
    Double_t fExitX, fExitY, fExitZ, fPathLengthAtExit;

    tExit->SetBranchAddress("fEvent", &fEvent);
    tExit->SetBranchAddress("fTrackID", &fTrackID);
    tExit->SetBranchAddress("fExitX", &fExitX);
    tExit->SetBranchAddress("fExitY", &fExitY);
    tExit->SetBranchAddress("fExitZ", &fExitZ);
    tExit->SetBranchAddress("fPathLengthAtExit", &fPathLengthAtExit);

    std::set<int> uniqueTracks;
    std::vector<int> selectedTracks;

    std::map<int, std::vector<double>> pathTracks;
    std::map<int, std::vector<double>> xTracks;
    std::map<int, std::vector<double>> yTracks;
    std::map<int, std::vector<double>> zTracks;


    Long64_t n = tExit->GetEntries();

    // std::vector<int> selectedTracks = {105, 109};
    for(Long64_t i = 0; i < n; i++){

        tExit->GetEntry(i);

        if(fEvent == eventID){

            // store first 5 track IDs only
            if(uniqueTracks.find(fTrackID) == uniqueTracks.end()){

                uniqueTracks.insert(fTrackID);

                if(selectedTracks.size() < 5){
                    selectedTracks.push_back(fTrackID);
                }
            }


            // save points only for selected tracks
            if(std::find(selectedTracks.begin(),
                         selectedTracks.end(),
                         fTrackID) != selectedTracks.end()) {

                pathTracks[fTrackID].push_back(fPathLengthAtExit);
                xTracks[fTrackID].push_back(fExitX);
                yTracks[fTrackID].push_back(fExitY);
                zTracks[fTrackID].push_back(fExitZ);
            }
        }
    }

    for(auto trackID : selectedTracks){
        std::cout << "TrackID " << trackID
              << " has "
              << pathTracks[trackID].size()
              << " exit points"
              << std::endl;
    }


    std::cout << "Event " << eventID
              << ": " << uniqueTracks.size()
              << " unique tracks" << std::endl;


    // ======================
    // Plot
    // ======================

    TCanvas *c = new TCanvas(
        "cExit",
        "First 5 photon trajectories",
        1000,
        750
    );

    c->SetGrid();


    int colors[5] = {
        kRed+1,
        kBlue+1,
        kGreen+2,
        kMagenta+1,
        kOrange+7
    };


    TLegend *legend = new TLegend(0.70,0.65,0.90,0.88);
    legend->SetTextSize(0.03);


    bool first = true;

    int counter = 0;

    for(auto trackID : selectedTracks){

        TGraph *gr = new TGraph(
            pathTracks[trackID].size(),
            pathTracks[trackID].data(),
            xTracks[trackID].data()
        );


        gr->SetMarkerStyle(20);
        gr->SetMarkerSize(1.5);
        gr->SetMarkerColor(colors[counter]);


        if(first){

            gr->SetTitle(
                Form("First 5 photon tracks - Event %d;"
                     "Path length at exit [mm];"
                     "Exit X [mm]",
                     eventID)
            );

            gr->Draw("AP");
            gr->GetXaxis()->SetLimits(0., 3000.);     // asse X
            gr->GetYaxis()->SetRangeUser(-120., 120.); // asse Y
            first=false;

        } else {

            gr->Draw("P SAME");

        }


        legend->AddEntry(
            gr,
            Form("Track ID %d", trackID),
            "p"
        );


        counter++;
    }


    legend->Draw();

    // c->SaveAs(Form("exit_first5_tracks_event_%d.png", eventID));

    int firstTrack = selectedTracks[1];

    TCanvas *cYZ = new TCanvas(
        "cYZ",
        "Photon impact positions",
        900,
        700
    );

    cYZ->SetGrid();
    TGraph *grYZ = new TGraph(
        yTracks[firstTrack].size(),
        zTracks[firstTrack].data(),
        yTracks[firstTrack].data()
    );

    grYZ->SetTitle(Form("Photon Track %d; ""z [mm]; ""y [mm]", firstTrack));

    grYZ->SetMarkerStyle(20);
    grYZ->SetMarkerSize(1.4);
    grYZ->SetMarkerColor(kBlue+1);

    grYZ->SetLineColor(kBlue+1);
    grYZ->SetLineWidth(2);

    TH1F *frame = cYZ->DrawFrame(140., -12.5, 160., 12.5);

    frame->SetTitle(Form("Photon Track %d;z [mm];y [mm]", firstTrack));

    TBox *detector = new TBox(142., -8., 158., 8.);

    detector->SetFillStyle(0);      // trasparente
    detector->SetLineColor(kRed+1);
    detector->SetLineWidth(2);

    detector->Draw("same");

    grYZ->Draw("LP SAME");;

    for(size_t i=0;i<zTracks[firstTrack].size();++i){ 

        TLatex *lab = new TLatex(
            zTracks[firstTrack][i],
            yTracks[firstTrack][i],
            Form("%zu",i)
        );

        lab->SetTextSize(0.025);
        lab->Draw();
    }   
}