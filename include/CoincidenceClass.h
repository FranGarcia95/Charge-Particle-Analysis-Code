#ifndef CoincidenceClass_h
#define CoincidenceClass_h

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
#include <TChain.h> 

#include "DetectorClass.h"
#include "UserInput.h"

class CoincidenceClass{ 
    
    public:

        CoincidenceClass(DetectorClass* det, UserInput *u);
        ~CoincidenceClass();

        void Processing();
        void Inizialization();
        void Hits(float amp, double tof, int detn, int bunch, double pulseintensity, std::string dettype, bool flag, double pkuptof);

    private:

        UserInput *userInp;
        DetectorClass *Detector;

        int __NumberDet__, __Number_Strips__;
        std::vector<std::string> __DetType__;

        double __PKUP_TOF__;

        //Coincidence Structure
        struct CoincidenceEvent  {
            float amp;
            double tof;
            int bunch;
            double pulseintensity;
        };

        bool Forward_Backward;

        std::map<int, std::vector<CoincidenceEvent>> DEED_Detector;
        std::map<int, std::vector<CoincidenceEvent>> EDET_Detector;

        std::map<int, std::vector<CoincidenceEvent>> Front_Detector;
        std::map<int, std::vector<CoincidenceEvent>> Back_Detector;
        std::map<int, std::vector<CoincidenceEvent>> Backward_Detector;

        void CoincidenceBackFront(const std::map<int, std::vector<CoincidenceEvent>>& Event_Front, 
                                  const std::map<int, std::vector<CoincidenceEvent>>& Event_Back);
        void CoincidenceDetectors(const std::map<int, std::vector<CoincidenceEvent>>& Event_Forward, 
                                  const std::map<int, std::vector<CoincidenceEvent>>& Event_Backward);
        void CoincidenceStrips(const std::map<int, std::vector<CoincidenceEvent>>& Event);

};
#endif