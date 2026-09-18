#ifndef DetectorCalibration_h
#define DetectorCalibration_h

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <map>

#include "TTree.h"
#include <TString.h>
#include "TSystem.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCutG.h"
#include <TF1.h>
#include <TGraphErrors.h>
#include <TCanvas.h>

#include "DetectorClass.h" 
#include "UserInput.h" 

class DetectorCalibration{ 
    
    public:

        DetectorCalibration(DetectorClass* det, UserInput *u);
        ~DetectorCalibration();
        void Inizialization();
        void Processing();

    private:

        UserInput *userInp;
        DetectorClass *Detector;

        std::vector<float> Fitting_Peak(float Mean, TH1F* histo, int width, TFile *outfile, int a, int b);
        void linear_calibration(std::vector<float> info_peak, std::vector<float> info_peak_err, int strip, TFile *outfile);

        TH1F*** __amp_histo__; //amp histogram to see the deposited energy bunch by bunch
        std::vector<std::vector<int>> __Cal_Range__;
        int __Number_Strips__;
        std::vector<std::string> __Sample__;
        int __Width_Fit__;

        std::vector<std::vector<float>> __Means_Info__;
        std::vector<std::vector<float>> __Energy_Par__;
        std::vector<float> __Peak_Energy__, __Peak_Energy_Error__;
        int __Number_Peaks__;

};
#endif
