#ifndef DSSSDProcessing_h
#define DSSSDProcessing_h

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include "TTree.h"
#include <TString.h>
#include "TSystem.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCutG.h"
#include <TChain.h> 

#include "DetectorClass.h"
#include "UserInput.h"

class CoincidenceClass;

class DSSSDProcessing{ 
    
    public:

        DSSSDProcessing(DetectorClass* det, UserInput *u);
        ~DSSSDProcessing();
        void Inizialization();
        void SetCoincidenceClass(CoincidenceClass* coincidence);
        void Processing(std::string Processing);
        void Clear();

    private:

        //Function for the data processing
        UserInput *userInp;

        //void Super_Coincidence_Processing(std::string Processing, int flag);
        void Coincidence_Processing_Strips(std::string Processing, std::string Rootfiles);
        void Coincidence_Processing_FrontBack(std::string Processing, std::string Rootfiles);  
        void Stability_Check(std::string Rootfiles);
        void Coincidence_Processing_Forward_Backward(std::string Processing, std::string Rootfiles);  
        void Data_Processing(std::string Processing, std::string Rootfiles, int flag = 0);
        void DeadTime_Processing(std::string Rootfiles);
        TChain* Chain_Def(const char* ftree_name, std::string Rootfiles);
        TChain* Chain_SSD(const char* ftree_name, std::string Rootfiles);
        //Parameters read from the root files
        double tof, tflash;
        float area, amp, PulseIntensity, risetime;
        int detn, BunchNumber, RunNumber, PSpulse, event, segment;

        double tof_PKUP, tflash_PKUP;
        float amp_PKUP, PulseIntensity_PKUP, area_PKUP;
        int BunchNumber_PKUP, RunNumber_PKUP;

        double tof_SSD, tflash_SSD;
        float area_SSD, amp_SSD, PulseIntensity_SSD, risetime_SSD;
        int detn_SSD, BunchNumber_SSD, RunNumber_SSD, PSpulse_SSD, event_SSD, segment_SSD;

        TChain *DSSSD, *SSD, *SILI, *PKUP;
        DetectorClass *Detector;
        CoincidenceClass *CoincidenceAnalysis;

        std::vector<std::string> __RunFiles__, __RunList__, __Root_Files__, __pulse_type__;
        int __NumberDet__, __Number_Strips__;
        double __Shift_Factor__;
        std::string __Output__, __DetType__;
        TFile* __OutFile__;

        //TChain *PKUP, *DEED, *EDET;
};
#endif