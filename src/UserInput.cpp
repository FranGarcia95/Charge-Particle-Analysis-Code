#include "UserInput.h" 

UserInput::UserInput(char *configName){

    std::cout << "Creating the input manager " << std::endl;
    ReadingFile(configName);

}

UserInput::~UserInput(){}

void UserInput::ReadingFile(char *finpName){

    std::string buffer;
    std::ifstream finp(finpName);
    if (finp.fail()) {
        std::cout << "Poblem getting config file: " << finpName << std::endl;
        exit(-1);
    } else std::cout << "Opening config file: " << finpName << std::endl;

    //General Definition
    while (finp >> buffer) {
        if(buffer.find("#") == 0) continue;

        if(buffer == "STOP") break;

        else if(buffer == "ProcessingMode")
            finp >> __Processing__;
        else if(buffer == "NumberSamples")
            finp >> __Number_Sample__;
        else if(buffer == "Samples"){
            for(int i = 0; i < __Number_Sample__; i++){
                finp >> buffer;
                __Sample__.push_back(buffer);
            }
        }
        else if(buffer == "NumberDetectors")
            finp >> __Number_Detectors__;
        else if(buffer == "Detectors"){
            for(int i = 0; i < __Number_Detectors__; i++){
                finp >> buffer;
                __Detector__.push_back(buffer);
            }
        }
        else if(buffer == "NumberStrips")
            finp >> __NumberStrips__;
        else if(buffer == "TOFoffset[ns]")
            finp >> __TOF_offset__;
        else if(buffer == "Flightpath[m]")
            finp >> __Fligt_Path__;
        else if(buffer == "ShiftFactor")
            finp >> __Shift_Factor__;
        else if(buffer == "DedicatedPS"){
            for(int i = 0; i < 2; i++){
                finp >> buffer;
                __Dedicated_PS__.push_back(stod(buffer));
            }
        }
        else if(buffer == "ParasiticPS"){
            for(int i = 0; i < 2; i++){
                finp >> buffer;
                __Parasitic_PS__.push_back(stod(buffer));
            }
        }
        else if(buffer == "PKUPTOF"){
            for(int i = 0; i < 2; i++){
                finp >> buffer;
                __PKUP_Flash__.push_back(stod(buffer));
            }
        }
        else if(buffer == "Calibration"){
            finp >> __Calibration__;
            if(__Calibration__){
                finp >> buffer;
                if(buffer == "ParameterCalFile"){
                    for(int i = 0; i < __Number_Sample__; i++){
                        finp >> buffer;
                        __Parameter_Calibration_File__.push_back(buffer);
                    }
                }else{
                    std::cerr << "No Calibration parameter file provided" << std::endl;
                    exit(-1);
                }
            }
        }
        else if(buffer == "DefaultAmpCut")
            finp >> __Default_Amp_Cut__;
        else if(buffer == "AmplitudeRangeCut"){
            std::cout << "Number of cuts" << __Default_Amp_Cut__ << std::endl;
            for(int i = 0; i < __Default_Amp_Cut__; i++){
                finp >> buffer;
                __Amplitude_Cut__.push_back(stof(buffer));
                std::cout << __Amplitude_Cut__[i] << std::endl;
            }
        }
        else if(buffer == "Constant/Curve"){
            if(__Calibration__){
                finp >> __NewCut__;
            }else if(!__Calibration__){
                __NewCut__ = false;
                std::cout << "Reading the option for the cut applied to the TOF spectra." <<
                             "If calibration option false, the cut is constant by default." << std::endl;
            }
        }else if(buffer == "PowFactor")
            finp >> __Pow_Factor__;
        else if(buffer == "PileUpCorrection")
            finp >> __PileUp_Correction__;
        else if(buffer == "NumberEnergyRanges"){
            finp >> __Number_Cuts__;
            if(__Number_Cuts__ < 1){
                std::cerr << "At least one energy range needed for projections" << std::endl;
                exit(-1);
            }else if(__Number_Cuts__ > 2){
                std::cerr << "If more energy ranges are wanted, the calibration parameters must be provided" << std:: endl;
            }
        }
        else if(buffer == "EnergyRangeCut"){
            for(int i = 0; i <= __Number_Cuts__; i++){
                finp >> buffer;
                __Energy_Cut__.push_back(stof(buffer));   
            }
        }
        else if(buffer == "NumberStripsDiscard")
            finp >> __Number_Strips_Discarded__;
        else if(buffer == "StripsDiscarded"){
            for(int i = 0; i < __Number_Strips_Discarded__; i++){
                finp >> buffer;
                __Discard_Strips__.push_back(stoi(buffer));
            }
        }
        else if(buffer == "RootFiles"){
            std::cout << __Number_Sample__ << std::endl;
            for(int i = 0; i < __Number_Sample__; i++){
                finp >> buffer;
                std::cout << buffer << std::endl;
                __RootFiles__.push_back(buffer);
            }
        }
        else if(buffer == "RunList"){
            for(int i = 0; i < __Number_Sample__; i++){
                finp >> buffer;
                __Runlist__.push_back(buffer);
            }
        }
        else if(buffer == "OutputFile")
            finp >> __Output_File__;
        
        //From here only calibration parameters
        else if(buffer == "NumberPeaks")
            finp >> __NumberPeaks__;
        else if(buffer == "SimulatedEnergy"){
            for(int i = 0; i < __NumberPeaks__; i++){
                finp >> buffer;
                __Simulated_Energy__.push_back(stof(buffer));
            }
        }
        else if(buffer == "SimulatedEnergyError"){
            for(int i = 0; i < __NumberPeaks__; i++){
                finp >> buffer;
                __Simulated_Energy_Error__.push_back(stof(buffer));
            }
        }
        else if(buffer == "ParameterCalFile")
            finp >> __CalibrationPeakInfoFile__; 
        else if(buffer == "WidthFitting")
            finp >> __Width_Fit__;

        //From here only Flight Path parameters
        else if(buffer == "Fitmode/TCmode")
            finp >> __TC__;
        else if(buffer == "ApplyPileUp")
            finp >> __Apply_PileUp__;
        else if(buffer == "Process/ReadFromfile")
            finp >> __Readhistofromfile__;
        else if(buffer == "InputRootFile")
            finp >> __File_Name__;
        else if(buffer == "EvaluatedFlux")
            finp >> __Evaluated_Flux__;
        else if(buffer == "CountingRateTC")
            finp >> __Counting_Rate_TC__;
        else if(buffer == "CrossSection")
            finp >> __Cross_Section__;
        else if(buffer == "FlightPathParameters")
            finp >> __Calibration_Parameters__;
    }

    //Read the parameters for energy Calibration
    if(__Processing__ == "Calibration") Read_InfoPeakFile(__CalibrationPeakInfoFile__);

    //Read the parameters for Flight Path calibration
    if(__Processing__ == "FlightPath") Read_Flight_Path_Parameters(__Calibration_Parameters__);

}

void UserInput::Read_InfoPeakFile(std::string file){

    std::string fline, buffer;
    std::ifstream ffinp(file.c_str());

    __Cal_Range__.resize(__NumberPeaks__);

    int columnIndex = 0;

    if (ffinp.fail()) {
        std::cout << "Poblem getting config file: " << file << std::endl;
        exit(-1);
    } else std::cout << "Opening config file: " << file << std::endl;

    while(getline(ffinp, fline)){
        std::istringstream ss(fline);
        columnIndex = 0;
        while(ss >> buffer){
            __Cal_Range__[columnIndex].push_back(std::stoi(buffer));
            columnIndex++;
        }  
    }

}

void UserInput::Read_Flight_Path_Parameters(std::string file){
    std::string buffer;
    std::ifstream finp(file.c_str());

    if (finp.fail()) {
        std::cout << "Poblem getting flight path calibration parameters file: " << file << std::endl;
        exit(-1);
    } else std::cout << "Opening flight path calibration parameters file: " << file << std::endl;

    while (finp >> buffer) {
        if (buffer.find("#") == 0)
            continue;
        else if(buffer == "Number_Peak_Fitted")
            finp >> __NumberFits__;
        else if(buffer == "Exp_Lower_Limit")
            for(int i = 0; i < __NumberFits__; i++){
                finp >> buffer;
                __Exp_Lower_Limit__.push_back(stof(buffer));
            }
        else if(buffer == "Exp_Upper_Limit")
            for(int i = 0; i < __NumberFits__; i++){
                finp >> buffer;
                __Exp_Upper_Limit__.push_back(stof(buffer));
            }
        else if(buffer == "Sim_Lower_Limit")
            for(int i = 0; i < __NumberFits__; i++){
                finp >> buffer;
                __Sim_Lower_Limit__.push_back(stof(buffer));
            }
        else if(buffer == "Sim_Upper_Limit")
            for(int i = 0; i < __NumberFits__; i++){
                finp >> buffer;
                __Sim_Upper_Limit__.push_back(stof(buffer));
            }
        else if(buffer == "Exp_Thermal"){
            finp >> buffer;
            if(buffer == "0"){
                __Peaks_Be_Fitted__.push_back(false);
            }else if(buffer == "4"){
                __Peaks_Be_Fitted__.push_back(true);
                std::cout << "No experimental thermal point used" << std::endl;
                for(int i = 0; i < 4; i++){
                    finp >> buffer;
                    __Par_Exp_Thermal__.push_back(stod(buffer)); 
                }

            }else{
                std::cerr << "Parameters for the thermal point wrong" << std::endl;
                exit(-1);
            }
        }
        else if(buffer == "Exp_First_Dip"){
            finp >> buffer;
            if(buffer == "0"){
                __Peaks_Be_Fitted__.push_back(false);
                std::cout << "No experimental first dip point used" << std::endl;
            }else if(buffer == "5"){
                __Peaks_Be_Fitted__.push_back(true);
                for(int i = 0; i < 5; i++){
                    finp >> buffer;
                    __Par_Exp_Dip1__.push_back(stod(buffer)); 
                }

            }else{
                std::cerr << "Parameters for the first dip wrong" << std::endl;
                exit(-1);
            }
        }
        else if(buffer == "Exp_Second_Dip"){
            finp >> buffer;
            if(buffer == "0"){
                __Peaks_Be_Fitted__.push_back(false);
                std::cout << "No experimental second dip point used" << std::endl;
            }else if(buffer == "4"){
                __Peaks_Be_Fitted__.push_back(true);
                for(int i = 0; i < 4; i++){
                    finp >> buffer;
                    __Par_Exp_Dip2__.push_back(stod(buffer)); 
                }

            }else{
                std::cerr << "Parameters for the second dip wrong" << std::endl;
                exit(-1);
            }
        }
        else if(buffer == "Exp_Third_Dip"){
            finp >> buffer;
            if(buffer == "0"){
                __Peaks_Be_Fitted__.push_back(false);
                std::cout << "No experimental third dip point used" << std::endl;
            }else if(buffer == "4"){
                __Peaks_Be_Fitted__.push_back(true);
                for(int i = 0; i < 4; i++){
                    finp >> buffer;
                    __Par_Exp_Dip3__.push_back(stod(buffer)); 
                }

            }else{
                std::cerr << "Parameters for the third dip wrong" << std::endl;
                exit(-1);
            }
        }
        else if(buffer == "Exp_Resonance"){
            finp >> buffer;
            if(buffer == "0"){
                __Peaks_Be_Fitted__.push_back(false);
                std::cout << "No experimental resonance point used" << std::endl;
            }else if(buffer == "3"){
                __Peaks_Be_Fitted__.push_back(true);
                for(int i = 0; i < 3; i++){
                    finp >> buffer;
                    __Par_Exp_Res__.push_back(stod(buffer)); 
                }

            }else{
                std::cerr << "Parameters for the resonance wrong" << std::endl;
                exit(-1);
            }
        }
        else if(buffer == "Sim_Thermal"){
            finp >> buffer;
            if(buffer == "0" && !__Peaks_Be_Fitted__[0]){
                std::cout << "No simulated thermal point used" << std::endl;
            }else if(buffer == "4" && __Peaks_Be_Fitted__[0]){
                for(int i = 0; i < 4; i++){
                    finp >> buffer;
                    __Par_Sim_Thermal__.push_back(stod(buffer)); 
                }
            }else{
                std::cerr << "Parameters for the thermal point wrong" << std::endl;
                exit(-1);
            }
        }
        else if(buffer == "Sim_First_Dip"){
            finp >> buffer;
            if(buffer == "0" && !__Peaks_Be_Fitted__[1]){
                std::cout << "No simulated first dip point used" << std::endl;
            }else if(buffer == "5" && __Peaks_Be_Fitted__[1]){
                for(int i = 0; i < 5; i++){
                    finp >> buffer;
                    __Par_Sim_Dip1__.push_back(stod(buffer)); 
                }
            }else{
                std::cerr << "Parameters for the first dip wrong" << std::endl;
                exit(-1);
            }
        }
        else if(buffer == "Sim_Second_Dip"){
            finp >> buffer;
            if(buffer == "0" && !__Peaks_Be_Fitted__[2]){
                std::cout << "No simulated second dip point used" << std::endl;
            }else if(buffer == "4" && __Peaks_Be_Fitted__[2]){
                for(int i = 0; i < 4; i++){
                    finp >> buffer;
                    __Par_Sim_Dip2__.push_back(stod(buffer)); 
                }
            }else{
                std::cerr << "Parameters for the second dip wrong" << std::endl;
                exit(-1);
            }
        }
        else if(buffer == "Sim_Third_Dip"){
            finp >> buffer;
            if(buffer == "0" && !__Peaks_Be_Fitted__[3]){
                std::cout << "No simulated third dip point used" << std::endl;
            }else if(buffer == "4"  && __Peaks_Be_Fitted__[3]){
                for(int i = 0; i < 4; i++){
                    finp >> buffer;
                    __Par_Sim_Dip3__.push_back(stod(buffer)); 
                }

            }else{
                std::cerr << "Parameters for the third dip wrong" << std::endl;
                exit(-1);
            }
        }
        else if(buffer == "Sim_Resonance"){
            finp >> buffer;
            if(buffer == "0" && !__Peaks_Be_Fitted__[4]){
                std::cout << "No simulated resonance point used" << std::endl;
            }else if(buffer == "3" && __Peaks_Be_Fitted__[4]){
                for(int i = 0; i < 3; i++){
                    finp >> buffer;
                    __Par_Sim_Res__.push_back(stod(buffer)); 
                }

            }else{
                std::cerr << "Parameters for the resonance wrong" << std::endl;
                exit(-1);
            }
        }
    }

    finp.close();


}

void UserInput::Get_FlightPath_Calibration_Range(std::vector<float> &Sim_Lower, std::vector<float> &Sim_Upper,
                                           std::vector<float> &Exp_Lower, std::vector<float> &Exp_Upper){

    Sim_Lower = __Sim_Lower_Limit__;   
    Sim_Upper = __Sim_Upper_Limit__;
    Exp_Lower = __Exp_Lower_Limit__;
    Exp_Upper = __Exp_Upper_Limit__;                                
 
}

void UserInput::Get_FlightPath_Experimental_Parameters(std::vector<double> &Thermal_Point, std::vector<double> &First_Dip, 
                                                     std::vector<double> &Second_Dip, std::vector<double> &Third_Dip, 
                                                     std::vector<double> &Resonance){

    Thermal_Point =  __Par_Exp_Thermal__;
    First_Dip = __Par_Exp_Dip1__;
    Second_Dip = __Par_Exp_Dip2__;
    Third_Dip = __Par_Exp_Dip3__;
    Resonance = __Par_Exp_Res__;                        
 
}

void UserInput::Get_FlightPath_Simulation_Parameters(std::vector<double> &Thermal_Point, std::vector<double> &First_Dip, 
                                                     std::vector<double> &Second_Dip, std::vector<double> &Third_Dip, 
                                                     std::vector<double> &Resonance){
    
    Thermal_Point =  __Par_Sim_Thermal__;
    First_Dip = __Par_Sim_Dip1__;
    Second_Dip = __Par_Sim_Dip2__;
    Third_Dip = __Par_Sim_Dip3__;
    Resonance = __Par_Sim_Res__; 

}

void UserInput::Get_PS_Parameters(std::vector<double> &DedicatedPS, std::vector<double> &ParasiticPS, 
                                  std::vector<double> &PKUPTOF){

    DedicatedPS = __Dedicated_PS__;
    ParasiticPS = __Parasitic_PS__;
    PKUPTOF     = __PKUP_Flash__;                    
 
}

