#include "DetectorClass.h" 
#include "CoincidenceClass.h"

DetectorClass::DetectorClass(UserInput *u){

    userInp = u;

}

DetectorClass::~DetectorClass(){
    
    if (__OutFile__) {
        __OutFile__->Write();
        __OutFile__->Close();
        delete __OutFile__;
        __OutFile__ = nullptr;
    }

}

void DetectorClass::Inizialization(){

    //Here we define all the general variables needed for this class
    __Processing__ = userInp->Get_Processing();
    __Number_Sample__ = userInp->Get_Number_Samples();
    __Sample__ = userInp->Get_Sample();
    __DetType__ = userInp->Get_Detectors();
    __NumberDet__ = __DetType__.size();
    __Number_Strips__ = userInp->Get_Number_Strips();
    __Calibration__ = userInp->Get_Calibration_Mode();
    __Discard_Strips__ = userInp->Get_Broken_Strips();
    __TOF_Offset__ = userInp->Get_TOF_Offset();
    __Fligt_Path__ = userInp->Get_Flight_Path();
    __PileUp_Correction__ = userInp->Get_Pile_Up();
    __RunList__ = userInp->Get_Run_List();

    userInp->Get_PS_Parameters(__Dedicated_PS__, __Parasitic_PS__, __PKUP_Flash__);

    if(__Processing__ != "FlightPath" && __Processing__ != "Calibration"){
        __OutFile__ = new TFile(userInp->Get_Output_File().c_str(), "RECREATE");
    }else if(__Processing__ == "FlightPath" && !userInp->Get_Histo_From_File()){
        __OutFile__ = new TFile(userInp->Get_Output_File().c_str(), "RECREATE");
    }else{
        std::cout << "Opening file to get the TOF or Energy histogram for Flight Path Calibration" << std::endl;
    }

    //We read or not the calibration file. Notice:
    //IF two detectors the first one has to be the single side.
    if(__Calibration__ && __NumberDet__ == 1 && __Processing__ != "Coincidence"){
        
        std::vector<std::string> filename = userInp->Get_Calibration_Parameter(); 
        __Cal_Parameters__ = Read_Calibration_Parameters(filename[0]);

    }else if(__Calibration__ && __NumberDet__ == 1 && __Processing__ == "Coincidence"){

        std::vector<std::string> filename = userInp->Get_Calibration_Parameter(); 
        __Cal_Parameters_DSSSD__ = Read_Calibration_Parameters(filename[0]);

    }else if(__Calibration__ && __NumberDet__ == 2 && __Processing__ == "Coincidence"){

        std::vector<std::string> filename = userInp->Get_Calibration_Parameter(); 
        __Cal_Parameters_SSD__ = Read_Calibration_Parameters(filename[0]);
        __Cal_Parameters_DSSSD__ = Read_Calibration_Parameters(filename[1]);

    }

    //We rename the side depending on the number of detectors
    //Two detectors only define for coincidence
    if(__Number_Strips__ <= 16){
        __Side__.push_back("Front");
    }else if(__Number_Strips__ > 16 && __NumberDet__ == 1){
        __Side__.push_back("Front");
        __Side__.push_back("Back");
    }else if(__NumberDet__ == 2){
        __Side__.push_back("Front");
        __Side__.push_back("Back");
        __Side__.push_back("Forward");
    }

    //Predefinition of Dedicated and Parasitic. No more than this two and that's why they are hardcode
    __pulse_type__ = {"Dedicated", "Parasitic"};

    __PulseIntensity__ = {{"Dedicated", 0}, {"Parasitic", 0}};
    __Bunches__        = {{"Dedicated", 0}, {"Parasitic", 0}};

    //Definition of the different cuts to be applied
    __Amplitude_Cut__  = userInp->Get_Amplitude_Cut();
    __Energy_Cut__     = userInp->Get_Energy_Cut();
    __NewCut__         = userInp->Get_New_Cut();

    if(__Processing__ == "Stability"){
        __RunFiles__ = Read_RunFile(__RunList__[0]);
    }

    //Initialization of the variables for the histograms
    HistogramInizialization();

    //Here we specified which histograms have to be initialize
    if(__Processing__ == "Calibration"){
        Histogram_Def_1D_Calibration(__Sample__, __DetType__[0]);
    }else if(__Processing__ == "DeadTime"){
        Histogram_Def_DeadTime(__DetType__[0]);
    }else if(__Processing__ == "Coincidence"){
        Histogram_Def_Coincidence_Front_Back(__Sample__[0], __DetType__[0]);
        if(__NumberDet__ == 2){
            Histogram_Def_Coincidence_Forward_Backward(__Sample__[0], __DetType__[0]);
        }
    }else if(__Processing__ == "DataProcessing"){
        Histogram_Def_2D(__Sample__[0], __DetType__[0]);
        Histogram_Def_1D(__Sample__[0], __DetType__[0]);
    }else if(__Processing__ == "FlightPath" && !userInp->Get_Histo_From_File()){
        Histogram_Def_2D(__Sample__[0], __DetType__[0]);
        Histogram_Def_1D(__Sample__[0], __DetType__[0]);
    }else if(__Processing__ == "Stability"){
        Histogram_Def_Stability(__Sample__[0], __DetType__[0]);
    }else{
        std::cout << "No Histogram generated. Histogram reading from file" << std::endl;
    }

    if(__NewCut__ && __Processing__ == "DataProcessing"){
        NewCutLineDraw();
    }else if(__NewCut__ && (__Processing__ == "FlightPath" && !userInp->Get_Histo_From_File())){
        NewCutLineDraw();
    }

}

void DetectorClass::HistogramInizialization(){

    if(__Processing__ == "Calibration"){

        __amp_histo_CAL__         = new TH1F**[2];
        __cal_histo_CAL__         = new TH1F**[2];
        for(int i = 0; i < 2; i++){
            __amp_histo_CAL__[i]      = new TH1F*[__Number_Strips__];
            __cal_histo_CAL__[i]      = new TH1F*[__Number_Strips__];    
        }

    }else if(__Processing__ == "DeadTime"){

        __Thr_SILI__        = new TH2F*[__Number_Strips__];
        __Thr_SILI_En__     = new TH2F*[__Number_Strips__];
        __Thr_Time_Diff__   = new TH1F*[__Number_Strips__];
        __Thr_Energy_Dep__  = new TH1F*[__Number_Strips__];
        __Dead_Time__       = new TH1F*[__Number_Strips__];

    }else if(__Processing__ == "Stability"){

        __Strip_Run_Ded__  = new TH1D*[__Number_Strips__];
        __Strip_PKUP_Ded__ = new TH1D*[__Number_Strips__];
        __Strip_Run_Par__  = new TH1D*[__Number_Strips__];
        __Strip_PKUP_Par__ = new TH1D*[__Number_Strips__];

    }else if(__Processing__ == "Coincidence"){

        __TOF_Ded_Coincidence__ = new TH2D*[__Side__.size()];
        __TOF_Par_Coincidence__ = new TH2D*[__Side__.size()];

        __Energy_Ded_Coincidence__ = new TH2D*[__Side__.size()];
        __Energy_Par_Coincidence__ = new TH2D*[__Side__.size()];

        __TOF_coincidence_Ded_1D__ = new TH1D*[__Side__.size()];
        __TOF_coincidence_Par_1D__ = new TH1D*[__Side__.size()];

        __Energy_coincidence_Ded_1D__ = new TH1D*[__Side__.size()];
        __Energy_coincidence_Par_1D__ = new TH1D*[__Side__.size()];

        __amp_histo_coincidence__         = new TH1F**[__NumberDet__];
        if(__Calibration__) __cal_histo_coincidence__         = new TH1F**[__NumberDet__];
        for(int i = 0; i < __NumberDet__; i++){
            __amp_histo_coincidence__[i]      = new TH1F*[__Number_Strips__];
            if(__Calibration__) __cal_histo_coincidence__[i]      = new TH1F*[__Number_Strips__];    
        }

    }else if(__Processing__ == "DataProcessing" || (__Processing__ == "FlightPath" && !userInp->Get_Histo_From_File())){
        __TOF_Ded__ = new TH2D*[__Side__.size()];
        __TOF_Par__ = new TH2D*[__Side__.size()];

        __Energy_Ded__ = new TH2D*[__Side__.size()];
        __Energy_Par__ = new TH2D*[__Side__.size()];

        __TOF_Ded_1D__ = new TH1D*[__Side__.size()];
        __TOF_Par_1D__ = new TH1D*[__Side__.size()];

        __Energy_Ded_1D__ = new TH1D*[__Side__.size()];
        __Energy_Par_1D__ = new TH1D*[__Side__.size()];

        __TOF_Ded_1D_PileUp__ = new TH1D*[__Side__.size()];
        __TOF_Par_1D_PileUp__ = new TH1D*[__Side__.size()];

        __Energy_Ded_1D_PileUp__ = new TH1D*[__Side__.size()];
        __Energy_Par_1D_PileUp__ = new TH1D*[__Side__.size()];

        __Map_Ded_1D__  = new TH1D*[__Side__.size()]; 
        __Map_Par_1D__  = new TH1D*[__Side__.size()];

        __amp_histo__                      = new TH1F*[__Number_Strips__];
        if(__Calibration__) __cal_histo__  = new TH1F*[__Number_Strips__];
        if(__Calibration__) __amp_sum__    = new TH1F*[__Energy_Cut__.size() - 1];
        if(__Calibration__) __cal_sum__    = new TH1F*[__Energy_Cut__.size() - 1];

        __tof_histo__             = new TH1F**[__pulse_type__.size()];
        __energy_histo__          = new TH1F**[__pulse_type__.size()];
        for(int i = 0; i < __pulse_type__.size(); i++){
            __tof_histo__[i]      = new TH1F*[__Number_Strips__];
            __energy_histo__[i]   = new TH1F*[__Number_Strips__];    
        }
    }

    if(__Processing__ == "Calibration"){

        __amp_ch__  = {0.0, 20000.0, 20000};
        __amp_cal__ = {0.0, 30.0, 3000};
    
        __amp_ch_bin__  = Histo_Bins(__amp_ch__[0], __amp_ch__[1], __amp_ch__[2], false);
        __amp_cal_bin__ = Histo_Bins(__amp_cal__[0], __amp_cal__[1], __amp_cal__[2], false);

    }else if(__Processing__ == "DeadTime"){

        __Thr_Energy_Reb__ = {0.0, 10.0, 2000};
        __Thr_Energy_Main__ = {0.0, 10.0, 2000};
        __Thr_Time__ = {0.0, 10000.0, 500};

        __Thr_Energy_Reb_bin__ = Histo_Bins(__Thr_Energy_Reb__[0], __Thr_Energy_Reb__[1], __Thr_Energy_Reb__[2], false);
        __Thr_Energy_Main_bin__ = Histo_Bins(__Thr_Energy_Main__[0], __Thr_Energy_Main__[1], __Thr_Energy_Main__[2], false);
        __Thr_Time_bin__ = Histo_Bins(__Thr_Time__[0], __Thr_Time__[1], __Thr_Time__[2], false);
    
    }else if(__Processing__ == "Stability"){
    

        int first_bunch = std::stoi(__RunFiles__.front());
        int last_bunch  = std::stoi(__RunFiles__.back());
        __bunch__       = {first_bunch - 5, last_bunch + 5, last_bunch - first_bunch + 10};
        __bunch_bin__   = Histo_Bins(__bunch__[0], __bunch__[1], __bunch__[2], false);

    }else{

        __amp_ch__  = {0.0, 30000.0, 12000};
        __amp_cal__ = {0.0, 30.0, 3000};
        __tof__     = {1.0, 1.0e9, 4500};
        __energy__  = {1.0e-3, 1.0e9, 6000};
    
        __amp_ch_bin__  = Histo_Bins(__amp_ch__[0], __amp_ch__[1], __amp_ch__[2], false);
        __amp_cal_bin__ = Histo_Bins(__amp_cal__[0], __amp_cal__[1], __amp_cal__[2], false);
        __tof_bin__     = Histo_Bins(__tof__[0], __tof__[1], __tof__[2], true);
        __energy_bin__  = Histo_Bins(__energy__[0], __energy__[1], __energy__[2], true);

    }

}

std::vector<std::string> DetectorClass::Read_RunFile(std::string Runlist){

    std::string buffer;
    std::ifstream ffinp(Runlist.c_str());
    std::vector<std::string> RunFiles;

    if(ffinp.fail()){
        std::cout << "Poblem getting config file: " << Runlist << std::endl;
        exit(-1);
    }else std::cout << "Opening config file: " << Runlist << std::endl;

    while(ffinp >> buffer){
        RunFiles.push_back(buffer);
    }

    return RunFiles;
}

std::vector<std::vector<float>> DetectorClass::Read_Calibration_Parameters(std::string filename){
    float a0, a1;
    std::ifstream ffinp(filename.c_str());
    if (ffinp.fail()) {
        std::cout << "Poblem getting config file: " << filename.c_str() << std::endl;
        exit(-1);
    } else
        std::cout << "Opening config file: " << filename.c_str() << std::endl;

    std::vector<std::vector<float>> parameters;
    while(ffinp >> a0 >> a1){
        parameters.push_back({a0, a1});
    }

    return parameters;
}

void DetectorClass::FillingHistogramProcessing(float amp, double tof, float PulseIntensity, int detn, int Bunch, double pkuptof){

    int side = 0;
    if(detn <= 16){
        side = 0;
    }else if(detn > 16){
        side = 1;
    }

    double lowcut, highcut, upcut;

    if(__NewCut__){
        lowcut  = __Energy_Cut_Line__[0]->GetBinContent(__Energy_Cut_Line__[0]->GetXaxis()->FindBin(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__)));
        highcut = __Energy_Cut_Line__[1]->GetBinContent(__Energy_Cut_Line__[1]->GetXaxis()->FindBin(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__)));
    }else{
        lowcut  = __Amplitude_Cut__[0];
        highcut = __Amplitude_Cut__[1];
    }

    if(__PileUp_Correction__){
        if(__NewCut__){
            upcut  = __Energy_Cut_Line__[2]->GetBinContent(__Energy_Cut_Line__[2]->GetXaxis()->FindBin(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__)));
        }else{
            upcut = __Amplitude_Cut__[2];
        }
    }

    double cutvalue;

    if(__Calibration__){
        cutvalue = __Cal_Parameters__[detn - 1][0] + amp * __Cal_Parameters__[detn - 1][1];
    }else{
        cutvalue = amp;
    }

    if(pkuptof == 0 && PulseIntensity > __Dedicated_PS__[0]){
        pkuptof = __PKUP_Flash__[0];
    }else if(pkuptof == 0 && PulseIntensity > __Parasitic_PS__[0] && PulseIntensity < __Parasitic_PS__[1]){
        pkuptof = __PKUP_Flash__[1];
    }

    bool discard = false;
    for(int k = 0; k < __Discard_Strips__.size(); k++){
        if(detn == __Discard_Strips__[k]){
            discard = true;
        }
    }

    if(PulseIntensity > __Dedicated_PS__[0] && (tof - pkuptof + __TOF_Offset__) > 0){

        if(!discard){
            if(__Calibration__){
                __TOF_Ded__[side]->Fill(tof - pkuptof + __TOF_Offset__, __Cal_Parameters__[detn - 1][0] + amp * __Cal_Parameters__[detn - 1][1]);
                __Energy_Ded__[side]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__), __Cal_Parameters__[detn - 1][0] + amp * __Cal_Parameters__[detn - 1][1]);
            }else{
                __TOF_Ded__[side]->Fill(tof - pkuptof + __TOF_Offset__, amp);
                __Energy_Ded__[side]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__), amp);
            }
        }

        if(cutvalue > lowcut && cutvalue < highcut){

            if(!discard){
                __TOF_Ded_1D__[side]->Fill(tof - pkuptof + __TOF_Offset__);
                __Energy_Ded_1D__[side]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__));

                if(side == 0){
                    __Map_Ded_1D__[side]->Fill(detn - 1);
                }else if(side == 1){
                    __Map_Ded_1D__[side]->Fill(detn - 17);
                }
                
            }

            __tof_histo__[0][detn - 1]->Fill(tof - pkuptof + __TOF_Offset__);
            __energy_histo__[0][detn - 1]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__));

        }

        if(__PileUp_Correction__){
            if(cutvalue >= highcut && cutvalue <= upcut){
                if(!discard){
                    __TOF_Ded_1D_PileUp__[side]->Fill(tof - pkuptof + __TOF_Offset__);
                    __Energy_Ded_1D_PileUp__[side]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__));
                }
            }
        }

        if(_Aux_ != Bunch){
            __PulseIntensity__.at("Dedicated") += PulseIntensity;
            __Bunches__.at("Dedicated") += 1;
            _Aux_ = Bunch;
        }

    }else if(PulseIntensity > __Parasitic_PS__[0] && PulseIntensity < __Parasitic_PS__[1]  && (tof - pkuptof + __TOF_Offset__) > 0){
        
        if(!discard){
            if(__Calibration__){
                __TOF_Par__[side]->Fill(tof - pkuptof + __TOF_Offset__, __Cal_Parameters__[detn - 1][0] + amp * __Cal_Parameters__[detn - 1][1], 1);
                __Energy_Par__[side]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__), __Cal_Parameters__[detn - 1][0] + amp * __Cal_Parameters__[detn - 1][1], 1);
            }else{
                __TOF_Par__[side]->Fill(tof - pkuptof + __TOF_Offset__, amp, 1);
                __Energy_Par__[side]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__), amp, 1);
            }
        }

        
        if(cutvalue > lowcut && cutvalue < highcut){

            if(!discard){
                __TOF_Par_1D__[side]->Fill(tof - pkuptof + __TOF_Offset__, 1);
                __Energy_Par_1D__[side]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__), 1);

                if(side == 0){
                    __Map_Par_1D__[side]->Fill(detn - 1);
                }else if(side == 1){
                    __Map_Par_1D__[side]->Fill(detn - 17);
                }
            }

            __tof_histo__[1][detn - 1]->Fill(tof - pkuptof + __TOF_Offset__);
            __energy_histo__[1][detn - 1]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__));

        }

        if(__PileUp_Correction__){
            if(cutvalue >= highcut && cutvalue <= upcut){
                if(!discard){
                    __TOF_Par_1D_PileUp__[side]->Fill(tof - pkuptof + __TOF_Offset__);
                    __Energy_Par_1D_PileUp__[side]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__));
                }
            }
        }

        if(_Aux_ != Bunch){
            __PulseIntensity__.at("Parasitic") += PulseIntensity;
            __Bunches__.at("Parasitic") += 1;
            _Aux_ = Bunch;
        }
    }

    if(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__) > __Energy_Cut__[0] && TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__) <= __Energy_Cut__[1]){
        if(PulseIntensity > 1e12){
            __amp_histo__[detn - 1]->Fill(amp);
            if(__Calibration__) __cal_histo__[detn - 1]->Fill(__Cal_Parameters__[detn - 1][0] + amp * __Cal_Parameters__[detn - 1][1]);
        }
    }

    if(__Calibration__ && detn <= 16){
        for(int i = 0; i < __Energy_Cut__.size() - 1; i++){
            if(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__) > __Energy_Cut__[i] && TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__) <= __Energy_Cut__[i + 1]){
                __amp_sum__[i]->Fill(amp);
                __cal_sum__[i]->Fill(__Cal_Parameters__[detn - 1][0] + amp * __Cal_Parameters__[detn - 1][1]);
                break;
            }
        }
    }

}

void DetectorClass::StabilityProcessing(float amp, double tof, float PulseIntensity, int detn, int Run, int Bunch, double pkuptof, double pkuparea, bool end){

    int side = 0;
    if(detn <= 16){
        side = 0;
    }else if(detn > 16){
        side = 1;
    }

    if(_Aux_RunBunch_){
        _Aux_Run_Ded_ = Run;
        _Aux_Run_Par_ = Run;
        _Aux_RunBunch_ = false;
    }

    double lowcut, highcut;

    lowcut  = __Amplitude_Cut__[0];
    highcut = __Amplitude_Cut__[1];

    double cutvalue;

    if(__Calibration__){
        cutvalue = __Cal_Parameters__[detn - 1][0] + amp * __Cal_Parameters__[detn - 1][1];
    }else{
        cutvalue = amp;
    }

    if(pkuptof == 0 && PulseIntensity > __Dedicated_PS__[0]){
        pkuptof = __PKUP_Flash__[0];
    }else if(pkuptof == 0 && PulseIntensity > __Parasitic_PS__[0] && PulseIntensity < __Parasitic_PS__[1]){
        pkuptof = __PKUP_Flash__[1];
    }

    double counts_Ded = 0; 

    if(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__) > 1e-3 && TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__) <= 100){
        if(cutvalue > lowcut && cutvalue < highcut){
            if(PulseIntensity > __Dedicated_PS__[0]){

                if(_Aux_Ded_ != Bunch){
                    __PulseIntensity__.at("Dedicated") += PulseIntensity;
                    __Bunches__.at("Dedicated") += 1;
                    _Aux_Ded_ = Bunch;
                }

                if(_Aux_Run_Ded_ != Run){
                    _Aux_Run_Ded_ = Run;
                    for(int i = 0; i < __Number_Strips__; i++){
                        if(__Bunches__.at("Dedicated") != 0){
                            int bin = __Strip_PKUP_Ded__[i]->GetXaxis()->FindBin(Run - 1);
                            __Strip_PKUP_Ded__[i]->SetBinContent(bin, __Strip_PKUP_Ded__[i]->GetBinContent(bin) / __Bunches__.at("Dedicated"));
                            __Strip_Run_Ded__[i]->SetBinContent(bin, __Strip_Run_Ded__[i]->GetBinContent(bin) / __Bunches__.at("Dedicated"));
                        }
                    }
                    __Bunches__.at("Dedicated") = 0;
                }

                __Strip_Run_Ded__[detn - 1]->Fill(Run, 1.0);
                __Strip_PKUP_Ded__[detn - 1]->Fill(Run, 1.0 / pkuparea);

            }else if(PulseIntensity > __Parasitic_PS__[0] && PulseIntensity < __Parasitic_PS__[1]){

                if(_Aux_Par_ != Bunch){
                    __PulseIntensity__.at("Parasitic") += PulseIntensity;
                    __Bunches__.at("Parasitic") += 1;
                    _Aux_Par_ = Bunch;
                }

                if(_Aux_Run_Par_ != Run){
                    _Aux_Run_Par_ = Run;
                    for(int i = 0; i < __Number_Strips__; i++){
                        if(__Bunches__.at("Parasitic") != 0){
                            int bin = __Strip_PKUP_Par__[i]->GetXaxis()->FindBin(Run - 1);
                            __Strip_PKUP_Par__[i]->SetBinContent(bin, __Strip_PKUP_Par__[i]->GetBinContent(bin) / __Bunches__.at("Parasitic"));
                            __Strip_Run_Par__[i]->SetBinContent(bin, __Strip_Run_Par__[i]->GetBinContent(bin) / __Bunches__.at("Parasitic"));
                        }
                    }
                    __Bunches__.at("Parasitic") = 0;
                }

                __Strip_Run_Par__[detn - 1]->Fill(Run, 1.0);
                __Strip_PKUP_Par__[detn - 1]->Fill(Run, 1.0 / pkuparea);
            }
        }
    }

    if(end){
        std::cout << __Bunches__.at("Dedicated") << "   " << Run << std::endl;
        for(int i = 0; i < __Number_Strips__; i++){
            if(__Bunches__.at("Dedicated") != 0){
                int bin = __Strip_PKUP_Ded__[i]->GetXaxis()->FindBin(Run);
                __Strip_PKUP_Ded__[i]->SetBinContent(bin, __Strip_PKUP_Ded__[i]->GetBinContent(bin) / __Bunches__.at("Dedicated"));
                __Strip_Run_Ded__[i]->SetBinContent(bin, __Strip_Run_Ded__[i]->GetBinContent(bin) / __Bunches__.at("Dedicated"));
            }
            
            if(__Bunches__.at("Parasitic") != 0){
                int bin = __Strip_PKUP_Par__[i]->GetXaxis()->FindBin(Run);
                __Strip_PKUP_Par__[i]->SetBinContent(bin, __Strip_PKUP_Par__[i]->GetBinContent(bin) / __Bunches__.at("Parasitic"));
                __Strip_Run_Par__[i]->SetBinContent(bin, __Strip_Run_Par__[i]->GetBinContent(bin) / __Bunches__.at("Parasitic"));
            }
        }
    }

}

void DetectorClass::NormHistogram(){

    for(int k = 0; k < __Side__.size(); k++){
        __TOF_Ded_1D__[k]->Scale(1.0 / __PulseIntensity__.at("Dedicated")); 
        __TOF_Par_1D__[k]->Scale(1.0 / __PulseIntensity__.at("Parasitic")); 

        __Energy_Ded_1D__[k]->Scale(1.0 / __PulseIntensity__.at("Dedicated"));
        __Energy_Par_1D__[k]->Scale(1.0 / __PulseIntensity__.at("Parasitic"));
    }

    if(__PileUp_Correction__){
        for(int k = 0; k < __Side__.size(); k++){
            __TOF_Ded_1D_PileUp__[k]->Scale(1.0 / __PulseIntensity__.at("Dedicated")); 
            __TOF_Par_1D_PileUp__[k]->Scale(1.0 / __PulseIntensity__.at("Parasitic")); 

            __Energy_Ded_1D_PileUp__[k]->Scale(1.0 / __PulseIntensity__.at("Dedicated"));
            __Energy_Par_1D_PileUp__[k]->Scale(1.0 / __PulseIntensity__.at("Parasitic"));
        }
    }

    for(int i = 0; i < __Number_Strips__; i++){
        for(int j = 0; j < __pulse_type__.size(); j++){
            if(j == 0){
                __tof_histo__[0][i]->Scale(1.0 / __PulseIntensity__.at("Dedicated"));
                __energy_histo__[0][i]->Scale(1.0 / __PulseIntensity__.at("Dedicated"));
            }else{
                __tof_histo__[1][i]->Scale(1.0 / __PulseIntensity__.at("Parasitic"));
                __energy_histo__[1][i]->Scale(1.0 / __PulseIntensity__.at("Parasitic"));
            }
        }
    }


}

void DetectorClass::PileUpCorrection(){

    TH1D**  __TOF_Ded_1D_PileUp_Corrected__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
    TH1D**  __TOF_Par_1D_PileUp_Corrected__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

    TH1D**  __Energy_Ded_1D_PileUp_Corrected__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
    TH1D**  __Energy_Par_1D_PileUp_Corrected__; //This histogram is used to get the number of counts per runnumber and bunch for stability checks.

    __TOF_Ded_1D_PileUp_Corrected__ = new TH1D*[__Side__.size()];
    __TOF_Par_1D_PileUp_Corrected__ = new TH1D*[__Side__.size()];
    __Energy_Ded_1D_PileUp_Corrected__ = new TH1D*[__Side__.size()];
    __Energy_Par_1D_PileUp_Corrected__ = new TH1D*[__Side__.size()];

    for(int k = 0; k < __Side__.size(); k++){

        __TOF_Ded_1D_PileUp_Corrected__[k] = (TH1D*)__TOF_Ded_1D__[k]->Clone(Form("Pile up corrected TOF spectra for detetector %s & sample %s. %s & Dedicated", __Side__[k].c_str(), __Sample__[0].c_str(), __DetType__[0].c_str()));
        __TOF_Par_1D_PileUp_Corrected__[k] = (TH1D*)__TOF_Par_1D__[k]->Clone(Form("Pile up corrected TOF spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[k].c_str(), __Sample__[0].c_str(), __DetType__[0].c_str()));
        __Energy_Ded_1D_PileUp_Corrected__[k] = (TH1D*)__Energy_Ded_1D__[k]->Clone(Form("Pile up corrected Energy spectra for detetector %s & sample %s. %s & Dedicated",  __Side__[k].c_str(), __Sample__[0].c_str(), __DetType__[0].c_str()));
        __Energy_Par_1D_PileUp_Corrected__[k] = (TH1D*)__Energy_Par_1D__[k]->Clone(Form("Pile up corrected Energy spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[k].c_str(), __Sample__[0].c_str(), __DetType__[0].c_str()));

        __TOF_Ded_1D_PileUp_Corrected__[k]->SetDirectory(__OutFile__);
        __TOF_Par_1D_PileUp_Corrected__[k]->SetDirectory(__OutFile__);
        __Energy_Ded_1D_PileUp_Corrected__[k]->SetDirectory(__OutFile__);
        __Energy_Par_1D_PileUp_Corrected__[k]->SetDirectory(__OutFile__);

        __TOF_Ded_1D_PileUp_Corrected__[k]->Add(__TOF_Ded_1D_PileUp__[k], 2);
        __TOF_Par_1D_PileUp_Corrected__[k]->Add(__TOF_Par_1D_PileUp__[k], 2);
        __Energy_Ded_1D_PileUp_Corrected__[k]->Add(__Energy_Ded_1D_PileUp__[k], 2);
        __Energy_Par_1D_PileUp_Corrected__[k]->Add(__Energy_Par_1D_PileUp__[k], 2);
        
    }

    TH1D**  __TOF_Ded_Par_Ratio_1D__Before_PileUp_Corrected__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
    TH1D**  __TOF_Ded_Par_Ratio_1D__After_PileUp_Corrected__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.

    TH1D**  __Energy_Ded_Par_Ratio_1D__Before_PileUp_Corrected__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.
    TH1D**  __Energy_Ded_Par_Ratio_1D__After_PileUp_Corrected__; //Bunch_runnum spectra for 2D plot. This is used to normalize for the number of bunches.

    __TOF_Ded_Par_Ratio_1D__Before_PileUp_Corrected__ = new TH1D*[__Side__.size() * 2];
    __TOF_Ded_Par_Ratio_1D__After_PileUp_Corrected__ = new TH1D*[__Side__.size() * 2];
    __Energy_Ded_Par_Ratio_1D__Before_PileUp_Corrected__ = new TH1D*[__Side__.size() * 2];
    __Energy_Ded_Par_Ratio_1D__After_PileUp_Corrected__ = new TH1D*[__Side__.size() * 2];

    for(int k = 0; k < __Side__.size(); k++){

        __TOF_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k] = (TH1D*)__TOF_Ded_1D__[k]->Clone(Form("TOF ratio between Dedicated & parasitic before PileUp correction detetector %s, sample %s & %s", __Side__[k].c_str(), __Sample__[0].c_str(), __DetType__[0].c_str()));
        __TOF_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k] = (TH1D*)__TOF_Ded_1D_PileUp_Corrected__[k]->Clone(Form("TOF ratio between Dedicated & parasitic after PileUp correction detetector %s, sample %s & %s", __Side__[k].c_str(), __Sample__[0].c_str(), __DetType__[0].c_str()));
        __Energy_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k] = (TH1D*)__Energy_Ded_1D__[k]->Clone(Form("Energy ratio between Dedicated & parasitic before PileUp correction detetector %s, sample %s & %s", __Side__[k].c_str(), __Sample__[0].c_str(), __DetType__[0].c_str()));
        __Energy_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k] = (TH1D*)__Energy_Ded_1D_PileUp_Corrected__[k]->Clone(Form("Energy ratio between Dedicated & parasitic after PileUp correction detetector %s, sample %s & %s", __Side__[k].c_str(), __Sample__[0].c_str(), __DetType__[0].c_str()));

        __TOF_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k + __Side__.size()] = (TH1D*)__TOF_Par_1D__[k]->Clone();
        __TOF_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k + __Side__.size()] = (TH1D*)__TOF_Par_1D_PileUp_Corrected__[k]->Clone();
        __Energy_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k + __Side__.size()] = (TH1D*)__Energy_Par_1D__[k]->Clone();
        __Energy_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k + __Side__.size()] = (TH1D*)__Energy_Par_1D_PileUp_Corrected__[k]->Clone();

        __TOF_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k]->SetDirectory(__OutFile__);
        __TOF_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k]->SetDirectory(__OutFile__);
        __Energy_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k]->SetDirectory(__OutFile__);
        __Energy_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k]->SetDirectory(__OutFile__);

        __TOF_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k + __Side__.size()]->SetDirectory(0);
        __TOF_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k + __Side__.size()]->SetDirectory(0);
        __Energy_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k + __Side__.size()]->SetDirectory(0);
        __Energy_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k + __Side__.size()]->SetDirectory(0);

        __TOF_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k]->Rebin(20);
        __TOF_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k]->Rebin(20);
        __Energy_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k]->Rebin(20);
        __Energy_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k]->Rebin(20);

        __TOF_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k + __Side__.size()]->Rebin(20);
        __TOF_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k + __Side__.size()]->Rebin(20);
        __Energy_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k + __Side__.size()]->Rebin(20);
        __Energy_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k + __Side__.size()]->Rebin(20);

        __TOF_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k]->Divide(__TOF_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k + __Side__.size()]);
        __TOF_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k]->Divide(__TOF_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k + __Side__.size()]);
        __Energy_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k]->Divide(__Energy_Ded_Par_Ratio_1D__Before_PileUp_Corrected__[k + __Side__.size()]);
        __Energy_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k]->Divide(__Energy_Ded_Par_Ratio_1D__After_PileUp_Corrected__[k + __Side__.size()]);
        
    }

}

void DetectorClass::CalibrationProcessing(float amp, double tof, float PulseIntensity, int detn, double pkuptof, int samplenumber){
    
    if(pkuptof == 0 && PulseIntensity > __Dedicated_PS__[0]){
        pkuptof = __PKUP_Flash__[0];
    }else if(pkuptof == 0 && PulseIntensity > __Parasitic_PS__[0] && PulseIntensity < __Parasitic_PS__[1]){
        pkuptof = __PKUP_Flash__[1];
    }

    if(PulseIntensity > 1e12){
        if(TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__) > __Energy_Cut__[0] && TOF_Energy_Convertion(__Fligt_Path__, tof - pkuptof + __TOF_Offset__) <= __Energy_Cut__[1]){
            __amp_histo_CAL__[samplenumber][detn - 1]->Fill(amp);
        }
    }

}

void DetectorClass::DeadTimeCorrection(float amp1, float amp2, double tof1, double tof2, int detn, double pkuptof){
    
    double TOF1 = TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__);
    double TOF2 = TOF_Energy_Convertion(__Fligt_Path__, tof2 - pkuptof + __TOF_Offset__);
    double energy1 = __Cal_Parameters__[detn - 1][0] + amp1 * __Cal_Parameters__[detn - 1][1];
    double energy2 = __Cal_Parameters__[detn - 1][0] + amp2 * __Cal_Parameters__[detn - 1][1];

    if(TOF1 < 10000 && TOF2 < 10000){

        __Dead_Time__[detn - 1]->Fill(tof2 - tof1);

        if(energy1 > 0.6){
            __Thr_Time_Diff__[detn - 1]->Fill(tof2 - tof1);
            __Thr_SILI__[detn- 1]->Fill(tof2 - tof1, energy1);
        }
        __Thr_SILI_En__[detn - 1]->Fill(energy1, energy2);

        if((tof2 - tof1) < 2000){
            __Thr_Energy_Dep__[detn- 1]->Fill(energy2);
        }

    }

}

void DetectorClass::CoincidenceProcessing(float amp1, float amp2, int detn1, int detn2, double tof1, double tof2, double pulseintensity, double pkuptof, bool forwardbackward){

    bool discard_Front = false;
    for(int k = 0; k < __Discard_Strips__.size(); k++){
        if(detn1 == __Discard_Strips__[k]){
            discard_Front = true;
        }
    }

    bool discard_Back = false;
    for(int k = 0; k < __Discard_Strips__.size(); k++){
        if(detn2 == __Discard_Strips__[k]){
            discard_Back = true;
        }
    }

    double lowcut, highcut;

    if(__NewCut__){
        lowcut  = __Amplitude_Cut__[0] + TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__)/1000;
        highcut = __Amplitude_Cut__[1] + TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__)/1000;
    }else{
        lowcut  = __Amplitude_Cut__[0];
        highcut = __Amplitude_Cut__[1];
    }

    if(pkuptof == 0 && pulseintensity > 7e12){
        pkuptof = 13272.6;
    }else if(pkuptof == 0 && pulseintensity > 1e12 && pulseintensity < 7e12){
        pkuptof = 12743.5;
    }

    double energycut_1 = TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__);
    double energycut_2 = TOF_Energy_Convertion(__Fligt_Path__, tof2 - pkuptof + __TOF_Offset__);
    if(!forwardbackward){

        if ((energycut_1 > __Energy_Cut__[0] && energycut_1 < __Energy_Cut__[1]) ||
        (energycut_2 > __Energy_Cut__[0] && energycut_2 < __Energy_Cut__[1])) 
            __Diff_Time_Front_Back__->Fill(tof1 - tof2);

        if(pulseintensity < 7e12){

            if(!discard_Back && !discard_Front){
                if(__Calibration__){
                    __TOF_Ded_Coincidence__[0]->Fill(tof1 - pkuptof + __TOF_Offset__, __Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);
                    __Energy_Ded_Coincidence__[0]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__), __Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);
                    __TOF_Ded_Coincidence__[1]->Fill(tof2 - pkuptof + __TOF_Offset__, __Cal_Parameters_DSSSD__[detn2 - 1][0] + amp2 * __Cal_Parameters_DSSSD__[detn2 - 1][1]);
                    __Energy_Ded_Coincidence__[1]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof2 - pkuptof + __TOF_Offset__), __Cal_Parameters_DSSSD__[detn2 - 1][0] + amp2 * __Cal_Parameters_DSSSD__[detn2 - 1][1]);
                }else{
                    __TOF_Ded_Coincidence__[0]->Fill(tof1 - pkuptof + __TOF_Offset__, amp1);
                    __Energy_Ded_Coincidence__[0]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__), amp1);
                    __TOF_Ded_Coincidence__[1]->Fill(tof2 - pkuptof + __TOF_Offset__, amp2);
                    __Energy_Ded_Coincidence__[1]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof2 - pkuptof + __TOF_Offset__), amp2);
                }
            }

            if ((energycut_1 > __Energy_Cut__[0] && energycut_1 < __Energy_Cut__[1]) || 
                (energycut_2 > __Energy_Cut__[0] && energycut_2 < __Energy_Cut__[1])){

                if(!discard_Back && !discard_Front){
                    __Energy_Front_Back_Par__->Fill(__Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1], __Cal_Parameters_DSSSD__[detn2 - 1][0] + amp2 * __Cal_Parameters_DSSSD__[detn2 - 1][1]);
                }

                __Coincidence_Par__->Fill(detn1 - 1, detn2 - 1  - (__Number_Strips__ - 16));
                __amp_histo_coincidence__[0][detn1 - 1]->Fill(amp1);
                __cal_histo_coincidence__[0][detn1 - 1]->Fill(__Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);
                __amp_histo_coincidence__[0][detn2 - 1]->Fill(amp2);
                __cal_histo_coincidence__[0][detn2 - 1]->Fill(__Cal_Parameters_DSSSD__[detn2 - 1][0] + amp2 * __Cal_Parameters_DSSSD__[detn2 - 1][1]);
            } 
            
            if(amp1 > lowcut && amp1 < highcut){
                __TOF_coincidence_Par_1D__[0]->Fill(tof1 - pkuptof + __TOF_Offset__);
                __Energy_coincidence_Par_1D__[0]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__));
                __TOF_coincidence_Par_1D__[1]->Fill(tof2 - pkuptof + __TOF_Offset__);
                __Energy_coincidence_Par_1D__[1]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof2 - pkuptof + __TOF_Offset__));
            }

        }else{

            if(!discard_Back && !discard_Front){
                if(__Calibration__){
                    __TOF_Par_Coincidence__[0]->Fill(tof1 - pkuptof + __TOF_Offset__, __Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);
                    __Energy_Par_Coincidence__[0]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__), __Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);
                    __TOF_Par_Coincidence__[1]->Fill(tof2 - pkuptof + __TOF_Offset__, __Cal_Parameters_DSSSD__[detn2 - 1][0] + amp2 * __Cal_Parameters_DSSSD__[detn2 - 1][1]);
                    __Energy_Par_Coincidence__[1]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof2 - pkuptof + __TOF_Offset__), __Cal_Parameters_DSSSD__[detn2 - 1][0] + amp2 * __Cal_Parameters_DSSSD__[detn2 - 1][1]);
                }else{
                    __TOF_Par_Coincidence__[0]->Fill(tof1 - pkuptof + __TOF_Offset__, amp1);
                    __Energy_Par_Coincidence__[0]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__), amp1);
                    __TOF_Par_Coincidence__[1]->Fill(tof2 - pkuptof + __TOF_Offset__, amp2);
                    __Energy_Par_Coincidence__[1]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof2 - pkuptof + __TOF_Offset__), amp2);
                }
            }

            if ((energycut_1 > __Energy_Cut__[0] && energycut_1 < __Energy_Cut__[1]) ||
            (energycut_2 > __Energy_Cut__[0] && energycut_2 < __Energy_Cut__[1])){
                if(!discard_Back && !discard_Front){
                    __Energy_Front_Back_Ded__->Fill(__Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1], __Cal_Parameters_DSSSD__[detn2 - 1][0] + amp2 * __Cal_Parameters_DSSSD__[detn2 - 1][1]);
                }
            
                __Coincidence_Ded__->Fill(detn1 - 1, detn2 - 1 - (__Number_Strips__ - 16));
                __amp_histo_coincidence__[0][detn1 - 1]->Fill(amp1);
                __cal_histo_coincidence__[0][detn1 - 1]->Fill(__Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);
                __amp_histo_coincidence__[0][detn2 - 1]->Fill(amp2);
                __cal_histo_coincidence__[0][detn2 - 1]->Fill(__Cal_Parameters_DSSSD__[detn2 - 1][0] + amp2 * __Cal_Parameters_DSSSD__[detn2 - 1][1]);

            }
            
            if(amp1 > lowcut && amp1 < highcut){
                __TOF_coincidence_Ded_1D__[0]->Fill(tof1 - pkuptof + __TOF_Offset__);
                __Energy_coincidence_Ded_1D__[0]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__));
                __TOF_coincidence_Ded_1D__[1]->Fill(tof2 - pkuptof + __TOF_Offset__);
                __Energy_coincidence_Ded_1D__[1]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof2 - pkuptof + __TOF_Offset__));
            }

        }
    
    }else{

        if ((energycut_1 > __Energy_Cut__[0] && energycut_1 < __Energy_Cut__[1]) || (energycut_2 > __Energy_Cut__[0] && energycut_2 < __Energy_Cut__[1])) 
            __Diff_Time_Forward_Backward__->Fill(tof1 - tof2);

        if(pulseintensity < 7e12){

            if(!discard_Back && !discard_Front){
                if(__Calibration__){
                    __TOF_Ded_Coincidence__[2]->Fill(tof1 - pkuptof + __TOF_Offset__, __Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);
                    __Energy_Ded_Coincidence__[2]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__), __Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);
                }else{
                    __TOF_Ded_Coincidence__[2]->Fill(tof1 - pkuptof + __TOF_Offset__, amp1);
                    __Energy_Ded_Coincidence__[2]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__), amp1);
                }
            }

            if ((energycut_1 > __Energy_Cut__[0] && energycut_1 < __Energy_Cut__[1]) || 
                (energycut_2 > __Energy_Cut__[0] && energycut_2 < __Energy_Cut__[1])){

                if(!discard_Back && !discard_Front){
                    __Energy_Forward_Backward_Par__->Fill(__Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1], __Cal_Parameters_DSSSD__[detn2 - 1][0] + amp2 * __Cal_Parameters_DSSSD__[detn2 - 1][1]);
                }

                __Coincidence_Par_Forward_Backward__->Fill(detn1 - 1, detn2 - 1);
                __amp_histo_coincidence__[1][detn1 - 1]->Fill(amp1);
                __cal_histo_coincidence__[1][detn1 - 1]->Fill(__Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);
            } 
            
            if(amp1 > lowcut && amp1 < highcut){
                __TOF_coincidence_Par_1D__[2]->Fill(tof1 - pkuptof + __TOF_Offset__);
                __Energy_coincidence_Par_1D__[2]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__));
            }

        }else{

            if(!discard_Back && !discard_Front){
                if(__Calibration__){
                    __TOF_Par_Coincidence__[2]->Fill(tof1 - pkuptof + __TOF_Offset__, __Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);
                    __Energy_Par_Coincidence__[2]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__), __Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);
                }else{
                    __TOF_Par_Coincidence__[2]->Fill(tof1 - pkuptof + __TOF_Offset__, amp1);
                    __Energy_Par_Coincidence__[2]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__), amp1);
                }
            }

            if ((energycut_1 > __Energy_Cut__[0] && energycut_1 < __Energy_Cut__[1]) || (energycut_2 > __Energy_Cut__[0] && energycut_2 < __Energy_Cut__[1])){
                if(!discard_Back && !discard_Front){
                    __Energy_Forward_Backward_Ded__->Fill(__Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1], __Cal_Parameters_DSSSD__[detn2 - 1][0] + amp2 * __Cal_Parameters_DSSSD__[detn2 - 1][1]);
                }
            
                __Coincidence_Ded_Forward_Backward__->Fill(detn1 - 1, detn2 - 1);
                __amp_histo_coincidence__[1][detn1 - 1]->Fill(amp1);
                __cal_histo_coincidence__[1][detn1 - 1]->Fill(__Cal_Parameters_DSSSD__[detn1 - 1][0] + amp1 * __Cal_Parameters_DSSSD__[detn1 - 1][1]);

            }
            
            if(amp1 > lowcut && amp1 < highcut){
                __TOF_coincidence_Ded_1D__[2]->Fill(tof1 - pkuptof + __TOF_Offset__);
                __Energy_coincidence_Ded_1D__[2]->Fill(TOF_Energy_Convertion(__Fligt_Path__, tof1 - pkuptof + __TOF_Offset__));
            }

        }
    
    }

}

std::vector<double> DetectorClass::Histo_Bins(float fminvalue, float fmaxvalue, int fnumbins, bool bool_log){
    // Input verification
    if (fminvalue >= fmaxvalue) {
        throw std::invalid_argument("The minimum value must be less than the maximum.");
    }
    
    // Vector to store the limits of the bins
    std::vector<double> bins;
    
    if(bool_log){
        if (fminvalue <= 0 || fmaxvalue <= 0) {
            throw std::invalid_argument("The minimum and maximum values must be greater than 0.");
        }
        // Definition of the logaritmic binnings
        double logMin = std::log10(fminvalue);
        double logMax = std::log10(fmaxvalue);
        
        // Uniform spacing on the logarithmic scale
        double step = (logMax - logMin) / fnumbins;

        for (int i = 0; i <= fnumbins; ++i) {
            bins.push_back(std::pow(10, logMin + i * step));
        }
    }else{
        if (fminvalue < 0 || fmaxvalue < 0) {
            throw std::invalid_argument("The minimum and maximum value must be greater than 0.");
        } 
        double step = (fmaxvalue - fminvalue) / fnumbins;

        for (int i = 0; i <= fnumbins; ++i) {
            bins.push_back(fminvalue*1.0 + i * 1.0 * step);
        }
    }

    return bins;
}

double DetectorClass::TOF_Energy_Convertion(double L0, double tTOF){
    return 0.5*939.565330e6/(0.299792458*0.299792458)*(L0/tTOF)*(L0/tTOF);
}

void DetectorClass::Histogram_Def_2D(std::string __sample__, std::string __SILI__){

    if(__Calibration__){
        for(int i = 0; i < __Side__.size(); i++){
            __TOF_Ded__[i] = new TH2D(Form("TOF vs Amp spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("TOF vs Amp spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __tof__[2], __tof_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
            __TOF_Ded__[i]->SetDirectory(__OutFile__);

            __TOF_Par__[i] = new TH2D(Form("TOF vs Amp spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("TOF vs Amp spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __tof__[2], __tof_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
            __TOF_Par__[i]->SetDirectory(__OutFile__);


            __Energy_Ded__[i] = new TH2D(Form("Energy vs Amp spectra for detetector %s & sample %s. %s & Dedicated",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        Form("Energy vs Amp spectra for detetector %s & sample %s. %s & Dedicated",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        __energy__[2], __energy_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
            __Energy_Ded__[i]->SetDirectory(__OutFile__);

            __Energy_Par__[i] = new TH2D(Form("Energy vs Amp spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        Form("Energy vs Amp spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        __energy__[2], __energy_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
            __Energy_Par__[i]->SetDirectory(__OutFile__);
        }
    }else{
        for(int i = 0; i < __Side__.size(); i++){
            __TOF_Ded__[i] = new TH2D(Form("TOF vs Amp spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("TOF vs Amp spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __tof__[2], __tof_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
            __TOF_Ded__[i]->SetDirectory(__OutFile__);

            __TOF_Par__[i] = new TH2D(Form("TOF vs Amp spectra for detetector %s & sample %s. %s & Parasitic", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("TOF vs Amp spectra for detetector %s & sample %s. %s & Parasitic", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __tof__[2], __tof_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
            __TOF_Par__[i]->SetDirectory(__OutFile__);


            __Energy_Ded__[i] = new TH2D(Form("Energy vs Amp spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        Form("Energy vs Amp spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        __energy__[2], __energy_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
            __Energy_Ded__[i]->SetDirectory(__OutFile__);

            __Energy_Par__[i] = new TH2D(Form("Energy vs Amp spectra for detetector %s & sample %s. %s & Parasitic", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        Form("Energy vs Amp spectra for detetector %s & sample %s. %s & Parasitic", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        __energy__[2], __energy_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
            __Energy_Par__[i]->SetDirectory(__OutFile__);
        }
    }

}

void DetectorClass::Histogram_Def_1D(std::string __sample__, std::string __SILI__){
   
    for (int i = 0; i < __Side__.size(); i++){
        __Map_Ded_1D__[i] = new TH1D(Form("Strip map for detetector %s & %s. Dedicated", __SILI__.c_str(), __Side__[i].c_str()), 
                                Form("Strip map for detetector %s & %s. Dedicated", __SILI__.c_str(), __Side__[i].c_str()), 
                                16, 0, 16);
        __Map_Ded_1D__[i]->SetDirectory(__OutFile__);

        __Map_Par_1D__[i] = new TH1D(Form("Strip map for detetector %s & %s. Parasitic", __SILI__.c_str(), __Side__[i].c_str()), 
                                Form("Strip map for detetector %s & %s. Parasitic", __SILI__.c_str(), __Side__[i].c_str()), 
                                16, 0, 16);
        __Map_Par_1D__[i]->SetDirectory(__OutFile__);
    }
    
    for(int i = 0; i < __Side__.size(); i++){
        __TOF_Ded_1D__[i] = new TH1D(Form("TOF spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                Form("TOF spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                __tof__[2], __tof_bin__.data());
        __TOF_Ded_1D__[i]->SetDirectory(__OutFile__);

        __TOF_Par_1D__[i] = new TH1D(Form("TOF spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                Form("TOF spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                __tof__[2], __tof_bin__.data());
        __TOF_Par_1D__[i]->SetDirectory(__OutFile__);


        __Energy_Ded_1D__[i] = new TH1D(Form("Energy spectra for detetector %s & sample %s. %s & Dedicated",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("Energy spectra for detetector %s & sample %s. %s & Dedicated",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __energy__[2], __energy_bin__.data());
        __Energy_Ded_1D__[i]->SetDirectory(__OutFile__);

        __Energy_Par_1D__[i] = new TH1D(Form("Energy spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("Energy spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __energy__[2], __energy_bin__.data());
        __Energy_Par_1D__[i]->SetDirectory(__OutFile__);
    }

    if(__PileUp_Correction__){
        for(int i = 0; i < __Side__.size(); i++){
            __TOF_Ded_1D_PileUp__[i] = new TH1D(Form("Pile up TOF spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("Pile up TOF spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __tof__[2], __tof_bin__.data());
            __TOF_Ded_1D_PileUp__[i]->SetDirectory(__OutFile__);

            __TOF_Par_1D_PileUp__[i] = new TH1D(Form("Pile up TOF spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("Pile up TOF spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __tof__[2], __tof_bin__.data());
            __TOF_Par_1D_PileUp__[i]->SetDirectory(__OutFile__);


            __Energy_Ded_1D_PileUp__[i] = new TH1D(Form("Pile up Energy spectra for detetector %s & sample %s. %s & Dedicated",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        Form("Pile up Energy spectra for detetector %s & sample %s. %s & Dedicated",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        __energy__[2], __energy_bin__.data());
            __Energy_Ded_1D_PileUp__[i]->SetDirectory(__OutFile__);

            __Energy_Par_1D_PileUp__[i] = new TH1D(Form("Pile up Energy spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        Form("Pile up Energy spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        __energy__[2], __energy_bin__.data());
            __Energy_Par_1D_PileUp__[i]->SetDirectory(__OutFile__);
        }

    }

    for(int i = 0; i < __Number_Strips__; i++){

        __amp_histo__[i] = new TH1F(Form("Amp spectra for detetector %i & sample %s. %s detector", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                    Form("Amp spectra for detetector %i & sample %s. %s detector", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                    __amp_ch__[2], __amp_ch_bin__.data());
        __amp_histo__[i]->SetDirectory(__OutFile__);

        if(__Calibration__){
            __cal_histo__[i] = new TH1F(Form("Amp calibrated energy spectra for detetector %i & sample %s. %s detector", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                        Form("Amp calibrated energy spectra for detetector %i & sample %s. %s detector", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                        __amp_cal__[2], __amp_cal_bin__.data());
            __cal_histo__[i]->SetDirectory(__OutFile__);
        }

        for(int j = 0; j < __pulse_type__.size(); j++){

            __tof_histo__[j][i] = new TH1F(Form("TOF spectra for detetector %i & sample %s. %s & %s", i+1, __sample__.c_str(), __SILI__.c_str(), __pulse_type__[j].c_str()), 
                                        Form("TOF vs Amp spectra for detetector %i & sample %s. %s & %s", i+1, __sample__.c_str(), __SILI__.c_str(), __pulse_type__[j].c_str()), 
                                        __tof__[2], __tof_bin__.data());
            __tof_histo__[j][i]->SetDirectory(__OutFile__);

            __energy_histo__[j][i] = new TH1F(Form("Energy spectra for detetector %i & sample %s. %s & %s", i+1, __sample__.c_str(), __SILI__.c_str(), __pulse_type__[j].c_str()), 
                                            Form("Energy spectra for detetector %i & sample %s. %s & %s", i+1, __sample__.c_str(), __SILI__.c_str(), __pulse_type__[j].c_str()), 
                                            __energy__[2], __energy_bin__.data());
            __energy_histo__[j][i]->SetDirectory(__OutFile__);
        }
    } 

    if(__Calibration__){
        for(int i = 0; i < __Energy_Cut__.size() - 1; i++){
            __amp_sum__[i] = new TH1F(Form("Amp spectra sample %s. %s detector. Between %f & %f.", __sample__.c_str(), __SILI__.c_str(), __Energy_Cut__[i], __Energy_Cut__[i+1]), 
                                    Form("Amp spectra for sample %s. %s detector. Between %f & %f.", __sample__.c_str(), __SILI__.c_str(), __Energy_Cut__[i], __Energy_Cut__[i+1]), 
                                    __amp_ch__[2], __amp_ch_bin__.data());
            __amp_sum__[i]->SetDirectory(__OutFile__);

            __cal_sum__[i] = new TH1F(Form("Amp calibrated energy spectra for sample %s. %s detector. Between %f & %f.", __sample__.c_str(), __SILI__.c_str(), __Energy_Cut__[i], __Energy_Cut__[i+1]), 
                                        Form("Amp calibrated energy spectra for sample %s. %s detector. Between %f & %f.", __sample__.c_str(), __SILI__.c_str(), __Energy_Cut__[i], __Energy_Cut__[i+1]), 
                                        __amp_cal__[2], __amp_cal_bin__.data());
            __cal_sum__[i]->SetDirectory(__OutFile__);
        }
    }

}

void DetectorClass::Histogram_Def_1D_Calibration(std::vector<std::string> __sample__, std::string __SILI__){

    for(int i = 0; i < __sample__.size(); i++){
            for(int j = 0; j < __Number_Strips__; j++){

            __amp_histo_CAL__[i][j] = new TH1F(Form("Amp spectra for detetector %i & sample %s. %s detector", j+1, __sample__[i].c_str(), __SILI__.c_str()), 
                                        Form("Amp spectra for detetector %i & sample %s. %s detector", j+1, __sample__[i].c_str(), __SILI__.c_str()), 
                                        __amp_ch__[2], __amp_ch_bin__.data());

            __cal_histo_CAL__[i][j] = new TH1F(Form("Amp calibrated energy spectra for detetector %i & sample %s. %s detector", j+1, __sample__[i].c_str(), __SILI__.c_str()), 
                                        Form("Amp calibrated energy spectra for detetector %i & sample %s. %s detector", j+1, __sample__[i].c_str(), __SILI__.c_str()), 
                                        __amp_cal__[2], __amp_cal_bin__.data());
        }

    }

}

void DetectorClass::Histogram_Def_Stability(std::string __sample__, std::string __SILI__){
    
    for(int i = 0; i < __Number_Strips__; i++){
        __Strip_Run_Ded__[i] = new TH1D(Form("Stability of strip %i per Run. Sample %s, detector %s & Dedicated", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                    Form("Stability of strip %i per Run. Sample %s, detector %s & Dedicated", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                    __bunch__[2], __bunch_bin__.data());

        std::cout << "Created: "
          << i << "  "
          << __Strip_Run_Ded__[i]
          << std::endl;

std::cout << "Name: "
          << __Strip_Run_Ded__[i]->GetName()
          << std::endl;

std::cout << "Bins: "
          << __Strip_Run_Ded__[i]->GetNbinsX()
          << std::endl;

        __Strip_Run_Ded__[i]->SetDirectory(__OutFile__);

        __Strip_PKUP_Ded__[i] = new TH1D(Form("Stability of strip %i compared with PKUP area. Sample %s, detector %s & Dedicated", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                     Form("Stability of strip %i compared with PKUP area. Sample %s, detector %s & Dedicated", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                    __bunch__[2], __bunch_bin__.data());
        __Strip_PKUP_Ded__[i]->SetDirectory(__OutFile__);

        __Strip_Run_Par__[i] = new TH1D(Form("Stability of strip %i per Run. Sample %s, detector %s & Parasitic", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                    Form("Stability of strip %i per Run. Sample %s, detector %s & Parasitic", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                    __bunch__[2], __bunch_bin__.data());
        __Strip_Run_Par__[i]->SetDirectory(__OutFile__);

        __Strip_PKUP_Par__[i] = new TH1D(Form("Stability of strip %i compared with PKUP area. Sample %s, detector %s & Parasitic", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                     Form("Stability of strip %i compared with PKUP area. Sample %s, detector %s & Parasitic", i+1, __sample__.c_str(), __SILI__.c_str()), 
                                    __bunch__[2], __bunch_bin__.data());
        __Strip_PKUP_Par__[i]->SetDirectory(__OutFile__);
    }

    __BCT_PKUP_Ded__ = new TH1D(Form("Stability for the proton beam. Sample %s, detector %s & Dedicated", __sample__.c_str(), __SILI__.c_str()), 
                                     Form("Stability for the proton beam. Sample %s, detector %s & Dedicated", __sample__.c_str(), __SILI__.c_str()), 
                                    __bunch__[2], __bunch_bin__.data());
    __BCT_PKUP_Ded__->SetDirectory(__OutFile__);

    __BCT_PKUP_Par__ = new TH1D(Form("Stability for the proton beam. Sample %s, detector %s & Parasitic", __sample__.c_str(), __SILI__.c_str()), 
                                     Form("Stability for the proton beam. Sample %s, detector %s & Parasitic", __sample__.c_str(), __SILI__.c_str()), 
                                    __bunch__[2], __bunch_bin__.data());
    __BCT_PKUP_Par__->SetDirectory(__OutFile__);

}

void DetectorClass::Histogram_Def_DeadTime(std::string __SILI__){

    for(int i = 0; i < __Number_Strips__; i++){

        __Thr_SILI__[i] = new TH2F(Form("Time Diff & Energy dep between two signals for %s detector %i", __SILI__.c_str(), i+1), 
                                   Form("Time Diff & Energy dep between two signals for %s detector %i", __SILI__.c_str(), i+1), 
                                   __Thr_Time__[2], __Thr_Time_bin__.data(),
                                   __Thr_Energy_Reb__[2], __Thr_Energy_Reb_bin__.data());

        __Thr_SILI__[i]->SetDirectory(__OutFile__);

        __Thr_SILI_En__[i] = new TH2F(Form("Energy dep from both signals between two signals for %s detector %i", __SILI__.c_str(), i+1), 
                                   Form("Energy dep from both signals between two signals for %s detector %i", __SILI__.c_str(), i+1), 
                                   __Thr_Energy_Reb__[2], __Thr_Energy_Reb_bin__.data(),
                                   __Thr_Energy_Main__[2], __Thr_Energy_Main_bin__.data());

        __Thr_SILI_En__[i]->SetDirectory(__OutFile__);

        __Thr_Energy_Dep__[i] = new TH1F(Form("Energy dep between two signals for %s detector %i", __SILI__.c_str(), i+1), 
                                             Form("Energy dep between two signals for %s detector %i", __SILI__.c_str(), i+1), 
                                             __Thr_Energy_Reb__[2], __Thr_Energy_Reb_bin__.data());

        __Thr_Energy_Dep__[i]->SetDirectory(__OutFile__);


        __Thr_Time_Diff__[i] = new TH1F(Form("Time Diff between two signals for %s detector %i", __SILI__.c_str(), i+1), 
                                            Form("Time Diff between two signals for %s detector %i", __SILI__.c_str(), i+1), 
                                            __Thr_Time__[2], __Thr_Time_bin__.data());

        __Thr_Time_Diff__[i]->SetDirectory(__OutFile__);

        __Dead_Time__[i] = new TH1F(Form("Dead Time corection for %s detector %i", __SILI__.c_str(), i+1), 
                                            Form("Dead Time corection for %s detector %i", __SILI__.c_str(), i+1), 
                                            __Thr_Time__[2], __Thr_Time_bin__.data());

        __Dead_Time__[i]->SetDirectory(__OutFile__);

    } 

}

void DetectorClass::Histogram_Def_Coincidence_Front_Back(std::string __sample__, std::string __SILI__){

    __Coincidence_Ded__ = new TH2F(Form("Coincidence map between detectors & sample %s. %s & Dedicated", __sample__.c_str(), __SILI__.c_str()), 
                                    Form("Coincidence map between detectors & sample %s. %s & Dedicated", __sample__.c_str(), __SILI__.c_str()), 
                                    __Number_Strips__, 0, __Number_Strips__, __Number_Strips__, 0, __Number_Strips__);
    __Coincidence_Ded__->SetDirectory(__OutFile__);

    __Coincidence_Par__ = new TH2F(Form("Coincidence map between detectors & sample %s. %s & Parasitic", __sample__.c_str(), __SILI__.c_str()), 
                                    Form("Coincidence map between detectors & sample %s. %s & Parasitic", __sample__.c_str(), __SILI__.c_str()), 
                                    __Number_Strips__, 0, __Number_Strips__, __Number_Strips__, 0, __Number_Strips__);
    __Coincidence_Par__->SetDirectory(__OutFile__);

    __Diff_Time_Front_Back__ = new TH1D("Time different between Front-Back detector", "Time different between Front-Back detector", 1000, -1000, 1000);
    __Diff_Time_Front_Back__->SetDirectory(__OutFile__);

    if(__Calibration__){
        for(int i = 0; i < __Side__.size(); i++){
            __TOF_Ded_Coincidence__[i] = new TH2D(Form("TOF vs Amp coincidence spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("TOF vs Amp coincidence spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __tof__[2], __tof_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
            __TOF_Ded_Coincidence__[i]->SetDirectory(__OutFile__);

            __TOF_Par_Coincidence__[i] = new TH2D(Form("TOF vs Amp coincidence spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("TOF vs Amp coincidence spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __tof__[2], __tof_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
            __TOF_Par_Coincidence__[i]->SetDirectory(__OutFile__);


            __Energy_Ded_Coincidence__[i] = new TH2D(Form("Energy vs Amp coincidence spectra for detetector %s & sample %s. %s & Dedicated",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        Form("Energy vs Amp coincidence spectra for detetector %s & sample %s. %s & Dedicated",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        __energy__[2], __energy_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
            __Energy_Ded_Coincidence__[i]->SetDirectory(__OutFile__);

            __Energy_Par_Coincidence__[i] = new TH2D(Form("Energy vs Amp coincidence spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        Form("Energy vs Amp coincidence spectra for detetector %s & sample %s. %s & Parasitic",  __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        __energy__[2], __energy_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
            __Energy_Par_Coincidence__[i]->SetDirectory(__OutFile__);
        }
        __Energy_Front_Back_Ded__ = new TH2F(Form("Energy for front and back detectors. Sample %s & Dedicated", __sample__.c_str()), 
                                             Form("Energy for front and back detectors. Sample %s & Dedicated", __sample__.c_str()), 
                                             __amp_cal__[2], __amp_cal_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
        __Energy_Front_Back_Ded__->SetDirectory(__OutFile__);

        __Energy_Front_Back_Par__ = new TH2F(Form("Energy for front and back detectors. Sample %s & Parasitic", __sample__.c_str()), 
                                             Form("Energy for front and back detectors. Sample %s & Parasitic", __sample__.c_str()), 
                                             __amp_cal__[2], __amp_cal_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
        __Energy_Front_Back_Par__->SetDirectory(__OutFile__);
    }else{
        for(int i = 0; i < __Side__.size(); i++){
            __TOF_Ded_Coincidence__[i] = new TH2D(Form("TOF vs Amp coincidence spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("TOF vs Amp coincidence spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __tof__[2], __tof_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
            __TOF_Ded_Coincidence__[i]->SetDirectory(__OutFile__);

            __TOF_Par_Coincidence__[i] = new TH2D(Form("TOF vs Amp coincidence spectra for detetector %s & sample %s. %s & Parasitic", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    Form("TOF vs Amp coincidence spectra for detetector %s & sample %s. %s & Parasitic", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                    __tof__[2], __tof_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
            __TOF_Par_Coincidence__[i]->SetDirectory(__OutFile__);


            __Energy_Ded_Coincidence__[i] = new TH2D(Form("Energy vs Amp coincidence spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        Form("Energy vs Amp coincidence spectra for detetector %s & sample %s. %s & Dedicated", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        __energy__[2], __energy_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
            __Energy_Ded_Coincidence__[i]->SetDirectory(__OutFile__);

            __Energy_Par_Coincidence__[i] = new TH2D(Form("Energy vs Amp coincidence spectra for detetector %s & sample %s. %s & Parasitic", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        Form("Energy vs Amp coincidence spectra for detetector %s & sample %s. %s & Parasitic", __Side__[i].c_str(), __sample__.c_str(), __SILI__.c_str()), 
                                        __energy__[2], __energy_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
            __Energy_Par_Coincidence__[i]->SetDirectory(__OutFile__);
        }
        __Energy_Front_Back_Ded__ = new TH2F(Form("Energy for front and back detectors. Sample %s & Dedicated", __sample__.c_str()), 
                                             Form("Energy for front and back detectors. Sample %s & Dedicated", __sample__.c_str()), 
                                             __amp_ch__[2], __amp_ch_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
        __Energy_Front_Back_Ded__->SetDirectory(__OutFile__);

        __Energy_Front_Back_Par__ = new TH2F(Form("Energy for front and back detectors. Sample %s & Parasitic", __sample__.c_str()), 
                                             Form("Energy for front and back detectors. Sample %s & Parasitic", __sample__.c_str()), 
                                             __amp_ch__[2], __amp_ch_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
        __Energy_Front_Back_Par__->SetDirectory(__OutFile__);
    }

    for(int i = 0; i < __Side__.size(); i++){
        __TOF_coincidence_Ded_1D__[i] = new TH1D(Form("TOF spectra for coincidence front-back. Sample %s, %s & Dedicated", __sample__.c_str(), __Side__[i].c_str()), 
                                                 Form("TOF spectra for coincidence front-back. Sample %s, %s & Dedicated", __sample__.c_str(), __Side__[i].c_str()), 
                                __tof__[2], __tof_bin__.data());
        __TOF_coincidence_Ded_1D__[i]->SetDirectory(__OutFile__);

        __TOF_coincidence_Par_1D__[i] = new TH1D(Form("TOF spectra for coincidence front-back. Sample %s, %s & Parasitic", __sample__.c_str(), __Side__[i].c_str()), 
                                Form("TOF spectra for coincidence front-back. Sample %s, %s & Parasitic", __sample__.c_str(), __Side__[i].c_str()), 
                                __tof__[2], __tof_bin__.data());
        __TOF_coincidence_Par_1D__[i]->SetDirectory(__OutFile__);


        __Energy_coincidence_Ded_1D__[i] = new TH1D(Form("Energy spectra for coincidence front-back. Sample %s, %s & Dedicated", __sample__.c_str(), __Side__[i].c_str()), 
                                    Form("Energy spectra for coincidence front-back. Sample %s, %s & Dedicated", __sample__.c_str(), __Side__[i].c_str()), 
                                    __energy__[2], __energy_bin__.data());
        __Energy_coincidence_Ded_1D__[i]->SetDirectory(__OutFile__);

        __Energy_coincidence_Par_1D__[i] = new TH1D(Form("Energy spectra for coincidence front-back. Sample %s, %s & Parasitic", __sample__.c_str(), __Side__[i].c_str()), 
                                    Form("Energy spectra for coincidence front-back. Sample %s, %s & Parasitic", __sample__.c_str(), __Side__[i].c_str()), 
                                    __energy__[2], __energy_bin__.data());
        __Energy_coincidence_Par_1D__[i]->SetDirectory(__OutFile__);
    }

    for(int j = 0; j < __Number_Strips__; j++){
        __amp_histo_coincidence__[0][j] = new TH1F(Form("Coincidence Amp spectra for sample %s. %i detector", __sample__.c_str(), j + 1), 
                                        Form("Coincidence Amp spectra for sample %s. %i detector", __sample__.c_str(), j + 1), 
                                        __amp_ch__[2], __amp_ch_bin__.data());
        __amp_histo_coincidence__[0][j]->SetDirectory(__OutFile__);

        if(__Calibration__){

            __cal_histo_coincidence__[0][j] = new TH1F(Form("Coincidence Amp calibrated energy spectra for sample %s. %i detector", __sample__.c_str(), j + 1), 
                                            Form("Coincidence Amp calibrated energy spectra for sample %s. %i detector", __sample__.c_str(), j + 1), 
                                            __amp_cal__[2], __amp_cal_bin__.data());
            __cal_histo_coincidence__[0][j]->SetDirectory(__OutFile__);

        }

    }

}

void DetectorClass::Histogram_Def_Coincidence_Forward_Backward(std::string __sample__, std::string __SILI__){

    __Coincidence_Ded_Forward_Backward__ = new TH2F(Form("Coincidence map between forward & backward detectors. Sample %s & Dedicated", __sample__.c_str()), 
                                   Form("Coincidence map between forward & backward detectors. Sample %s & Dedicated", __sample__.c_str()), 
                                    __Number_Strips__, 0, __Number_Strips__, __Number_Strips__, 0, __Number_Strips__);
    __Coincidence_Ded_Forward_Backward__->SetDirectory(__OutFile__);

    __Coincidence_Par_Forward_Backward__ = new TH2F(Form("Coincidence map between forward & backward detectors. Sample %s & Parasitic", __sample__.c_str()), 
                                    Form("Coincidence map between forward & backward detectors. Sample %s & Parasitic", __sample__.c_str()), 
                                    __Number_Strips__, 0, __Number_Strips__, __Number_Strips__, 0, __Number_Strips__);
    __Coincidence_Par_Forward_Backward__->SetDirectory(__OutFile__);

    __Diff_Time_Forward_Backward__ = new TH1D("Time different between Forward-Backward detector", "Time different between Forward-Backward detector", 1000, -1000, 1000);
    __Diff_Time_Forward_Backward__->SetDirectory(__OutFile__);

    if(__Calibration__){
        int i = __Side__.size() - 1;
     
        __Energy_Forward_Backward_Ded__ = new TH2F(Form("Energy for forward and backward detectors. Sample %s & Dedicated", __sample__.c_str()), 
                                             Form("Energy for forward and backward detectors. Sample %s & Dedicated", __sample__.c_str()), 
                                             __amp_cal__[2], __amp_cal_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
        __Energy_Forward_Backward_Ded__->SetDirectory(__OutFile__);

        __Energy_Forward_Backward_Par__ = new TH2F(Form("Energy for forward and backward detectors. Sample %s & Parasitic", __sample__.c_str()), 
                                             Form("Energy for forward and backward detectors. Sample %s & Parasitic", __sample__.c_str()), 
                                             __amp_cal__[2], __amp_cal_bin__.data(), __amp_cal__[2], __amp_cal_bin__.data());
        __Energy_Forward_Backward_Par__->SetDirectory(__OutFile__);

    }else{

        int i = __Side__.size() - 1;
        
        __Energy_Forward_Backward_Ded__ = new TH2F(Form("Energy for forward and backward detectors. Sample %s & Dedicated", __sample__.c_str()), 
                                             Form("Energy for forward and backward detectors. Sample %s & Dedicated", __sample__.c_str()), 
                                             __amp_ch__[2], __amp_ch_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
        __Energy_Forward_Backward_Ded__->SetDirectory(__OutFile__);

        __Energy_Forward_Backward_Par__ = new TH2F(Form("Energy for forward and backward detectors. Sample %s & Parasitic", __sample__.c_str()), 
                                             Form("Energy for forward and backward detectors. Sample %s & Parasitic", __sample__.c_str()), 
                                             __amp_ch__[2], __amp_ch_bin__.data(), __amp_ch__[2], __amp_ch_bin__.data());
        __Energy_Forward_Backward_Par__->SetDirectory(__OutFile__);

    }


    int i = __Side__.size() - 1;
    __TOF_coincidence_Ded_1D__[i] = new TH1D(Form("TOF spectra for coincidence forward-backward. Sample %s, %s & Dedicated", __sample__.c_str(), __Side__[i].c_str()), 
                                             Form("TOF spectra for coincidence forward-backward. Sample %s, %s & Dedicated", __sample__.c_str(), __Side__[i].c_str()), 
                                             __tof__[2], __tof_bin__.data());
    __TOF_coincidence_Ded_1D__[i]->SetDirectory(__OutFile__);

    __TOF_coincidence_Par_1D__[i] = new TH1D(Form("TOF spectra for coincidence forward-backward. Sample %s, %s & Parasitic", __sample__.c_str(), __Side__[i].c_str()), 
                                             Form("TOF spectra for coincidence forward-backward. Sample %s, %s & Parasitic", __sample__.c_str(), __Side__[i].c_str()), 
                                             __tof__[2], __tof_bin__.data());
    __TOF_coincidence_Par_1D__[i]->SetDirectory(__OutFile__);


    __Energy_coincidence_Ded_1D__[i] = new TH1D(Form("Energy spectra for coincidence forward-backward. Sample %s, %s & Dedicated", __sample__.c_str(), __Side__[i].c_str()), 
                                                Form("Energy spectra for coincidence forward-backward. Sample %s, %s & Dedicated", __sample__.c_str(), __Side__[i].c_str()), 
                                                __energy__[2], __energy_bin__.data());
    __Energy_coincidence_Ded_1D__[i]->SetDirectory(__OutFile__);

    __Energy_coincidence_Par_1D__[i] = new TH1D(Form("Energy spectra for coincidence forward-backward. Sample %s, %s & Parasitic", __sample__.c_str(), __Side__[i].c_str()), 
                                                Form("Energy spectra for coincidence forward-backward. Sample %s, %s & Parasitic", __sample__.c_str(), __Side__[i].c_str()), 
                                                __energy__[2], __energy_bin__.data());
    __Energy_coincidence_Par_1D__[i]->SetDirectory(__OutFile__);
    

    for(int j = 0; j < 16; j++){
        __amp_histo_coincidence__[1][j] = new TH1F(Form("Coincidence Amp spectra for sample %s. %i detector forward", __sample__.c_str(), j + 1), 
                                        Form("Coincidence Amp spectra for sample %s. %i detector forward", __sample__.c_str(), j + 1), 
                                        __amp_ch__[2], __amp_ch_bin__.data());
        __amp_histo_coincidence__[1][j]->SetDirectory(__OutFile__);

        if(__Calibration__){

            __cal_histo_coincidence__[1][j] = new TH1F(Form("Coincidence Amp calibrated energy spectra for sample %s. %i detector forward", __sample__.c_str(), j + 1), 
                                            Form("Coincidence Amp calibrated energy spectra for sample %s. %i detector forward", __sample__.c_str(), j + 1), 
                                            __amp_cal__[2], __amp_cal_bin__.data());
            __cal_histo_coincidence__[1][j]->SetDirectory(__OutFile__);

        }

    }

}

void DetectorClass::NewCutLineDraw(){

    if(__PileUp_Correction__){
        __Energy_Cut_Line__ = new TH1D*[3];
    }else{
        __Energy_Cut_Line__ = new TH1D*[2];
    }

    __Energy_Cut_Line__[0] = new TH1D("New Cut applied in Amplitude. Low Cut", "New Cut applied in Amplitude. Low Cut", 
                                         __energy__[2], __energy_bin__.data());
    __Energy_Cut_Line__[0]->SetDirectory(__OutFile__);

    __Energy_Cut_Line__[1] = new TH1D("New Cut applied in Amplitude. High Cut", "New Cut applied in Amplitude. High Cut", 
                                         __energy__[2], __energy_bin__.data());
    __Energy_Cut_Line__[1]->SetDirectory(__OutFile__);

    if(__PileUp_Correction__){
        __Energy_Cut_Line__[2] = new TH1D("New Cut applied in Amplitude for PileUp. Up Cut", "New Cut applied in Amplitude for PileUp. Up Cut", 
                                         __energy__[2], __energy_bin__.data());
        __Energy_Cut_Line__[2]->SetDirectory(__OutFile__);
    }

    double lowcut, highcut, upcut;
    lowcut = __Amplitude_Cut__[0];
    highcut = __Amplitude_Cut__[1];

    for (int i = 0; i < __energy__[2]; ++i) {

        __Energy_Cut_Line__[1]->SetBinContent(i + 1, highcut + pow(__energy_bin__[i] / 1e6, userInp->Get_Pow_Factor()));
        __Energy_Cut_Line__[0]->SetBinContent(i + 1, lowcut + pow(__energy_bin__[i] / 1e6, userInp->Get_Pow_Factor()));
    
    }

    if(__PileUp_Correction__){
        upcut = __Amplitude_Cut__[2];
        for (int i = 0; i < __energy__[2]; ++i) __Energy_Cut_Line__[2]->SetBinContent(i + 1, upcut + pow(__energy_bin__[i] / 1e6, userInp->Get_Pow_Factor()));
    }

}
