#ifndef UserInput_h
#define UserInput_h

#include <iostream>
#include <fstream>
#include <sstream>
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

class UserInput{ 
    
    public:

        UserInput(char *configName);
        ~UserInput();

        inline std::string Get_Cross_Section() { return __Cross_Section__; }
        inline std::string Get_Counting_Rate_TC() { return __Counting_Rate_TC__; }
        inline std::string Get_Evaluated_Flux() { return __Evaluated_Flux__; }
        inline int Get_Number_Detectors() { return __Number_Detectors__; }
        inline std::vector<std::string> Get_Root_Files() { return __RootFiles__; }
        inline std::vector<std::string> Get_Run_List() { return __Runlist__; }
        inline std::vector<std::string> Get_Calibration_Parameter() { return __Parameter_Calibration_File__; }
        inline bool Get_Calibration_Mode() {return __Calibration__; }
        inline std::vector<int> Get_Broken_Strips() { return __Discard_Strips__; }
        inline std::vector<int> Get_Broken_Strips_SSD() { return __Discard_Strips_SSD__; }
        inline int Get_Number_Strips() { return __NumberStrips__; }
        inline int Get_Width_Fit__() { return __Width_Fit__; }
        inline std::string Get_Processing() { return __Processing__; }
        inline std::string Get_PS_mode() { return __PS_mode__; }
        inline int Get_Number_Samples() { return __Number_Sample__; }
        inline std::vector<std::string> Get_Sample() { return __Sample__; }
        inline std::vector<std::string> Get_Detectors() { return __Detector__; }
        inline std::vector<float> Get_Amplitude_Cut() { return __Amplitude_Cut__; }
        inline std::vector<float> Get_Amplitude_Cut_SSD() { return __Amplitude_Cut_SSD__; }
        inline std::vector<float> Get_Energy_Cut() { return __Energy_Cut__; }
        inline std::string Get_Output_File() { return __Output_File__; }
        inline bool Get_New_Cut() { return __NewCut__; }
        inline bool Get_Pile_Up() { return __PileUp_Correction__; }
        inline bool Get_Apply_Pile_Up() { return __Apply_PileUp__; }
        inline double Get_Flight_Path() { return __Fligt_Path__; }
        inline double Get_TOF_Offset() { return __TOF_offset__; }
        inline double Get_Shift_Factor() { return __Shift_Factor__; }
        inline double Get_Maximum_Time_Diff__() { return __Maximum_Time_Diff__; }
        inline double Get_Minimum_Energy__() { return __Minimum_Energy__; }
        inline float Get_Pow_Factor() { return __Pow_Factor__; }
        inline bool Get_TC_FlightPath() { return __TC__; }
        inline bool Get_Histo_From_File() { return __Readhistofromfile__; }
        inline std::string Get_Histo_File() { return __File_Name__; }
        inline int  Get_Number_Fits() { return __NumberFits__; }
        inline std::vector<bool> Get_Peak_Be_Fitted__() { return __Peaks_Be_Fitted__; }
        inline int Get_NumberPeak_Calibration() { return __NumberPeaks__; }
        inline std::vector<float> Get_Simulated_Energy() { return __Simulated_Energy__; }
        inline std::vector<float> Get_Simulated_Energy_Error() { return __Simulated_Energy_Error__; }
        inline std::vector<std::vector<int>> Get_Calibration_Range() { return __Cal_Range__; }
        void Get_FlightPath_Calibration_Range(std::vector<float> &Sim_Lower, std::vector<float> &Sim_Upper,
                                              std::vector<float> &Exp_Lower, std::vector<float> &Exp_Upper);
        void Get_FlightPath_Experimental_Parameters(std::vector<double> &Thermal_Point, std::vector<double> &First_Dip, 
                                                    std::vector<double> &Second_Dip, std::vector<double> &Third_Dip, 
                                                    std::vector<double> &Resonance);
        void Get_Strips_order(std::vector<int> &Order_Strip_Front, std::vector<int> &Order_Strip_Back);
        void Get_FlightPath_Simulation_Parameters(std::vector<double> &Thermal_Point, std::vector<double> &First_Dip, 
                                                  std::vector<double> &Second_Dip, std::vector<double> &Third_Dip, 
                                                  std::vector<double> &Resonance);
        void Get_PS_Parameters(std::vector<double> &DedicatedPS, std::vector<double> &ParasiticPS, 
                               std::vector<double> &PKUPTOF);
        void Get_Coincidence_Amp_Window(float &coincidence_amp_low_window, float &coincidence_amp_high_window);
        void Get_Coincidence_Time_Window(float &coincidence_time_low_window, float &coincidence_time_high_window);
        void Get_Coincidence_Time_Window_Forward_Backward(float &coincidence_time_low_window_2det, float &coincidence_time_high_window_2det);
        void Get_Coincidence_Amp_Window_Forward_Backward(float &coincidence_amp_low_window_2det, float &coincidence_amp_high_window_2det);
    
    private:

        void ReadingFile(char *finpName);
        void Read_InfoPeakFile(std::string file);
        void Read_Flight_Path_Parameters(std::string file);

        //General definitions
        std::vector<std::string> __Sample__, __Detector__;
        std::vector<std::string> __Runlist__, __RootFiles__, __Parameter_Calibration_File__;
        std::string __Processing__;
        double __TOF_offset__, __Fligt_Path__, __Shift_Factor__, __Minimum_Energy__, __Maximum_Time_Diff__;
        std::vector<int> __Discard_Strips__, __Discard_Strips_SSD__;
        int __NumberStrips__, __Number_Detectors__ = 1, __Number_Sample__ = 1, __Number_Cuts__, __Number_Strips_Discarded__;
        int __Default_Amp_Cut__, __Number_Strips_Discarded_SSD__;
        bool __Calibration__ = false, __PileUp_Correction__ = false, __Apply_PileUp__ = false;

        //Parameters for the Amplitude to Energy Calibration
        std::string __CalibrationPeakInfoFile__, __Calibration_Parameters__, __Evaluated_Flux__, __PS_mode__;

        std::vector<std::vector<int>> __Cal_Range__;
        std::vector<int> __Order_Strip_Front__, __Order_Strip_Back__, __Order_Strip_SSD__;
        std::vector<float> __Simulated_Energy__, __Amplitude_Cut__, __Energy_Cut__, __Simulated_Energy_Error__, __Amplitude_Cut_SSD__;
        int __NumberPeaks__, __Width_Fit__, __Number_Strip_Front__, __Number_Strip_Back__, __Number_Strip_SSD__;
        float __Pow_Factor__, __Coincidence_Time_Low_Window__,__Coincidence_Time_High_Window__;
        float __Coincidence_Amp_Low_Window__, __Coincidence_Amp_High_Window__;
        float __Coincidence_Time_Low_Window_2Det__,__Coincidence_Time_High_Window_2Det__;
        float __Coincidence_Amp_Low_Window_2Det__, __Coincidence_Amp_High_Window_2Det__;

        //Flight Path parameters
        bool __TC__, __Readhistofromfile__, __NewCut__;
        int __NumberFits__;
        std::string __File_Name__, __Output_File__, __Counting_Rate_TC__, __Cross_Section__;
        std::vector<bool> __Peaks_Be_Fitted__;
        std::vector<double> __Dedicated_PS__, __Parasitic_PS__, __PKUP_Flash__;
        std::vector<float> __Exp_Upper_Limit__, __Exp_Lower_Limit__, __Sim_Upper_Limit__ ,__Sim_Lower_Limit__;
        std::vector<double> __Par_Exp_Thermal__, __Par_Exp_Dip1__, __Par_Exp_Dip2__, __Par_Exp_Dip3__, __Par_Exp_Res__;
        std::vector<double> __Par_Sim_Thermal__, __Par_Sim_Dip1__, __Par_Sim_Dip2__, __Par_Sim_Dip3__, __Par_Sim_Res__;


};
#endif