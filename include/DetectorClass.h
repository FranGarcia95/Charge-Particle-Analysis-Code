#ifndef DetectorClass_h
#define DetectorClass_h

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
#include "TH1F.h"

#include "UserInput.h" 

class DetectorClass{ 
    
    public:

        DetectorClass(UserInput *u);
        ~DetectorClass();

        void Inizialization();
        void FillingHistogramProcessing(float amp, double tof, float PulseIntensity, int detn, int Bunch, double pkuptof);
        void CalibrationProcessing(float amp, double tof, float PulseIntensity, int detn, double pkuptof, int samplenumber);
        void StabilityProcessing(float amp, double tof, float PulseIntensity, int detn, int Run, int Bunch, double pkuptof, double pkuparea, bool end);
        void DeadTimeCorrection(float amp1, float amp2, double tof1, double tof2, int detn, double pkuptof);
        void CoincidenceProcessing(float amp1, float amp2, int detn1, int detn2, double tof1, double tof2, double pulseintensity, double pkuptof, bool forwardbackward);        
        void NormHistogram();
        void PileUpCorrection();
        std::vector<std::string> Read_RunFile(std::string Runlist);
        inline TH1F*** Get_Amp_Histogram() { return __amp_histo_CAL__; };
        inline TH1D** Get_TOF_Histogram() { return __TOF_Ded_1D__; };
        inline TH1D** Get_Energy_Histogram() { return __Energy_Ded_1D__; };
        inline std::vector<std::string> Get_Samples() { return __Sample__; };
        inline std::vector<float> Get_Energy() { return __energy__; };
        inline std::vector<double> Get_Energy_Bin() { return __energy_bin__; };

    private:

        UserInput *userInp;
        //Function for the data processing
        void NewCutLineDraw();
        void Histogram_Def_DeadTime(std::string __SILI__);
        void Histogram_Def_2D(std::string __sample__, std::string __SILI__);
        void Histogram_Def_1D(std::string __sample__, std::string __SILI__);
        void Histogram_Def_Stability(std::string __sample__, std::string __SILI__);
        void Histogram_Def_Coincidence_Front_Back(std::string __sample__, std::string __SILI__);
        void Histogram_Def_1D_Calibration(std::vector<std::string> __sample__, std::string __SILI__);
        void Histogram_Def_Coincidence_Forward_Backward(std::string __sample__, std::string __SILI__);
        
        double TOF_Energy_Convertion(double L0, double tTOF);
        std::vector<double> Histo_Bins(float fminvalue, float fmaxvalue, int fnumbins, bool bool_log);
        void HistogramInizialization();
        std::vector<std::vector<float>> Read_Calibration_Parameters(std::string filename);

        //General variables for the histograms definition
        std::vector<float> __amp_ch__, __tof__, __energy__, __amp_cal__, __runnum__;
        std::vector<double> __amp_ch_bin__, __tof_bin__, __energy_bin__, __amp_cal_bin__, __bunch_bin__;
        std::vector<int> __bunch__;
        std::vector<std::string> __RunFiles__, __RunList__;
        std::vector<std::string> __Side__;
        double __TOF_Offset__, __Fligt_Path__;
        TFile* __OutFile__ = nullptr;
        bool __PileUp_Correction__;

        //General variables for the histograms definition for Dead Time
        std::vector<float> __Thr_Energy_Main__, __Thr_Energy_Reb__, __Thr_Time__;
        std::vector<double> __Thr_Energy_Main_bin__, __Thr_Energy_Reb_bin__, __Thr_Time_bin__;

        std::map<std::string, float> __PulseIntensity__;
        std::map<std::string, int>   __Bunches__;
        int _Aux_ = 0;
        int _Aux_Ded_ = 0, _Aux_Par_ = 0, _Aux_Run_Ded_ = 0, _Aux_Run_Par_ = 0;
        bool _Aux_RunBunch_ = true;
        std::vector<std::string> __pulse_type__;
        std::vector<float> __Amplitude_Cut__, __Energy_Cut__;
        std::vector<std::string> __Sample__;
        int __NumberDet__, __Number_Strips__, __Number_Sample__;
        std::string __Processing__;
        std::string __Output__;
        std::vector<std::string> __DetType__;
        bool __Calibration__, __NewCut__;
        std::vector<std::vector<float>> __Cal_Parameters__, __Cal_Parameters_DSSSD__, __Cal_Parameters_SSD__;
        std::vector<double> __Dedicated_PS__, __Parasitic_PS__, __PKUP_Flash__;
        std::vector<int> __Discard_Strips__;

        //Histogram definition for data processing
        TH2D**  __TOF_Ded__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
        TH2D**  __TOF_Par__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

        TH2D**  __Energy_Ded__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
        TH2D**  __Energy_Par__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

        TH1D**  __TOF_Ded_1D__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
        TH1D**  __TOF_Par_1D__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

        TH1D**  __Map_Ded_1D__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
        TH1D**  __Map_Par_1D__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

        TH1D**  __Energy_Ded_1D__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
        TH1D**  __Energy_Par_1D__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

        TH1D**  __TOF_Ded_1D_PileUp__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
        TH1D**  __TOF_Par_1D_PileUp__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

        TH1D**  __Energy_Ded_1D_PileUp__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
        TH1D**  __Energy_Par_1D_PileUp__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

        TH1D** __Strip_Run_Ded__;
        TH1D** __Strip_PKUP_Ded__;
        TH1D** __Strip_Run_Par__;
        TH1D** __Strip_PKUP_Par__;
        TH1D* __BCT_PKUP_Ded__;
        TH1D* __BCT_PKUP_Par__;

        TH1F** __amp_histo__; //amp histogram to see the deposited energy bunch by bunch
        TH1F** __cal_histo__; //Deposited energy specra calibrated spectra of 1D.
        TH1F** __amp_sum__; //amp histogram to see the deposited energy bunch by bunch
        TH1F** __cal_sum__; //Deposited energy specra calibrated spectra of 1D.
        TH1F*** __tof_histo__; //tof spectra of 1D.
        TH1F*** __energy_histo__; //energy spectra of 1D.

        //Histogram definition for calibration processing
        TH1F*** __amp_histo_CAL__; //amp histogram to see the deposited energy bunch by bunch
        TH1F*** __cal_histo_CAL__; //Deposited energy specra calibrated spectra of 1D.

        //Histogram definition for coincidence processing
        TH1F*** __amp_histo_coincidence__; //amp histogram to see the deposited energy bunch by bunch
        TH1F*** __cal_histo_coincidence__; //Deposited energy specra calibrated spectra of 1D.

        TH2D**  __TOF_Ded_Coincidence__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
        TH2D**  __TOF_Par_Coincidence__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

        TH2D**  __Energy_Ded_Coincidence__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
        TH2D**  __Energy_Par_Coincidence__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

        TH2F*   __Coincidence_Ded__;
        TH2F*   __Coincidence_Par__;
        TH2F*   __Energy_Front_Back_Ded__;
        TH2F*   __Energy_Front_Back_Par__;
        TH1D*   __Diff_Time_Front_Back__;

        TH2F*   __Coincidence_Ded_Forward_Backward__;
        TH2F*   __Coincidence_Par_Forward_Backward__;
        TH2F*   __Energy_Forward_Backward_Ded__;
        TH2F*   __Energy_Forward_Backward_Par__;
        TH1D*   __Diff_Time_Forward_Backward__;

        TH1D**  __TOF_coincidence_Ded_1D__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
        TH1D**  __TOF_coincidence_Par_1D__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

        TH1D**  __Energy_coincidence_Ded_1D__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
        TH1D**  __Energy_coincidence_Par_1D__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

        //Histogram definition for Dead Time Correction
        TH2F** __Thr_SILI__;
        TH2F** __Thr_SILI_En__;
        TH1F** __Thr_Time_Diff__;
        TH1F** __Thr_Energy_Dep__;
        TH1F** __Dead_Time__;

        //New Cut lines histogram
        TH1D** __Energy_Cut_Line__;

};
#endif