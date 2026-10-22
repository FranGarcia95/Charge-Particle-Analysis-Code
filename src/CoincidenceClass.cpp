#include "DetectorClass.h" 
#include "CoincidenceClass.h" 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CoincidenceClass::CoincidenceClass(DetectorClass* det, UserInput *u){

    Detector = det;
    userInp = u;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CoincidenceClass::~CoincidenceClass(){}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CoincidenceClass::Inizialization(){

    __DetType__ = userInp->Get_Detectors();
    __NumberDet__ = userInp->Get_Number_Detectors();
    __Number_Strips__ = userInp->Get_Number_Strips();
    userInp->Get_Coincidence_Time_Window(__Coincidence_Low_Window_Front_Back__, __Coincidence_High_Window_Front_Back__);
    userInp->Get_Coincidence_Time_Window_Forward_Backward(__Coincidence_Low_Window_Forward_Backward__, __Coincidence_High_Window_Forward_Backward__);

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CoincidenceClass::Hits(float amp, double tof, int detn, int bunch, double pulseintensity, std::string dettype, bool flag, double pkuptof){

    CoincidenceEvent c;

    c.amp = amp;
    c.tof = tof;
    c.bunch = bunch;
    c.pulseintensity = pulseintensity;
    __PKUP_TOF__ = pkuptof;
    
    if(__Number_Strips__ == 16){
        if(dettype == "DEED"){
            DEED_Detector[detn].push_back(c);
        }else if(dettype == "EDET"){
            EDET_Detector[detn].push_back(c);
        }
    }else if(__Number_Strips__ == 32 && __NumberDet__ == 1){
        if(detn <= 16){
            Front_Detector[detn].push_back(c);
        }else if(detn > 16){
            Back_Detector[detn].push_back(c);
        }
    }else if(__Number_Strips__ == 32 && __NumberDet__ == 2){
        if(dettype == "DSSSD" && detn <= 16){
            Front_Detector[detn].push_back(c);
        }else if(dettype == "DSSSD" && detn > 16){
            Back_Detector[detn].push_back(c);
        }else if(dettype == "SSD"){
            DEED_Detector[detn].push_back(c);
        }
    }

    
    
    if(flag){

        Processing();

    }

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CoincidenceClass::Processing(){
              
    if(__Number_Strips__ == 16){

        CoincidenceStrips(DEED_Detector);
        DEED_Detector.clear();

    }else if(__Number_Strips__ == 32 && __NumberDet__ == 1){

        Forward_Backward = false;
        CoincidenceDetectors(Front_Detector, Back_Detector);
        Front_Detector.clear();
        Back_Detector.clear();

    }else if(__Number_Strips__ == 32 && __NumberDet__ == 2){

        Forward_Backward = false;
        CoincidenceDetectors(Front_Detector, Back_Detector);
        Front_Detector.clear();
        Back_Detector.clear();
        Forward_Backward = true;
        CoincidenceDetectors(DEED_Detector, Backward_Detector);
        DEED_Detector.clear();
        Backward_Detector.clear();

    }

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CoincidenceClass::CoincidenceDetectors(const std::map<int, std::vector<CoincidenceEvent>>& Event_Forward, 
                                            const std::map<int, std::vector<CoincidenceEvent>>& Event_Backward){

    for(const auto& detector_first : Event_Forward){
        int detID_1 = detector_first.first;

        const auto& hits_first = detector_first.second;

        for(const auto& hit_first : hits_first){
             
            float  amp_first = hit_first.amp;
            double tof_first = hit_first.tof;
            int    bunch     = hit_first.bunch;
            double pulseintensity = hit_first.pulseintensity;

            for(const auto& detector_second : Event_Backward){

                int detID_2 = detector_second.first;

                const auto& hits_second = detector_second.second;

                for(const auto& hit_second : hits_second){

                    float  amp_second = hit_second.amp;
                    double tof_second = hit_second.tof;
                    if(__NumberDet__ == 1){

                        if((tof_second - tof_first) >= __Coincidence_High_Window_Front_Back__){
                            break;
                        }else if((tof_second - tof_first) >= __Coincidence_Low_Window_Front_Back__ && 
                                 (tof_second - tof_first) <= __Coincidence_High_Window_Front_Back__){
                                
                            Detector->CoincidenceProcessing(amp_first, amp_second, detID_1, detID_2, tof_first, tof_second, pulseintensity, bunch, __PKUP_TOF__, false);
                           
                        }

                    }else if(__NumberDet__ == 2){

                        if(Forward_Backward){

                            if((tof_second - tof_first) >= __Coincidence_High_Window_Forward_Backward__){
                                break;
                            }else if((tof_second - tof_first) >= __Coincidence_Low_Window_Forward_Backward__ && 
                                     (tof_second - tof_first) <= __Coincidence_High_Window_Forward_Backward__){
                                Detector->CoincidenceProcessing(amp_first, amp_second, detID_1, detID_2, tof_first, tof_second, pulseintensity, bunch, __PKUP_TOF__, true);
                                
                            }

                        }else if(!Forward_Backward){

                            if((tof_second - tof_first) >= __Coincidence_High_Window_Front_Back__){
                                break;
                            }else if((tof_second - tof_first) >= __Coincidence_Low_Window_Front_Back__ && 
                                     (tof_second - tof_first) <= __Coincidence_High_Window_Front_Back__){

                                CoincidenceEvent c;

                                c.amp = amp_first;
                                c.tof = tof_first;
                                c.bunch = bunch;
                                c.pulseintensity = pulseintensity;

                                Backward_Detector[detID_1].push_back(c);
                                Detector->CoincidenceProcessing(amp_first, amp_second, detID_1, detID_2, tof_first, tof_second, pulseintensity, bunch, __PKUP_TOF__, false);
                            }
                        }
                    }
                    
                }
            }
        }
    }
}                      

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CoincidenceClass::CoincidenceStrips(const std::map<int, std::vector<CoincidenceEvent>>& Event){

    int coincidence_window = 2000;

    for(const auto& detector_first : Event){
        int detID_1 = detector_first.first;

        const auto& hits_first = detector_first.second;

        for(const auto& hit_first : hits_first){
             
            float  amp_first = hit_first.amp;
            double tof_first = hit_first.tof;
            int    bunch     = hit_first.bunch;
            double pulseintensity = hit_first.pulseintensity;

            for(const auto& detector_second : Event){

                int detID_2 = detector_second.first;

                if(detID_1 >= detID_2) continue;

                const auto& hits_second = detector_second.second;

                for(const auto& hit_second : hits_second){

                    float  amp_second = hit_second.amp;
                    double tof_second = hit_second.tof;

                    if(tof_second > (tof_first + coincidence_window)){

                        break;

                    }else if(std::abs(tof_second - tof_first) < coincidence_window){

                        Detector->CoincidenceProcessing(amp_first, amp_second, detID_1, detID_2, tof_first, tof_second, pulseintensity, bunch, __PKUP_TOF__, false);

                    }
                }
            }
        }
    }
}
