#ifndef FIR_CALIBRATION_H
#define FIR_CALIBRATION_H

#include <iostream>

#include "apvdaq.hh"
#include "apvdaq_param.hh"

class FirCalib
{
private:
  double fir_adc[APVDAQNUMB][APVCHIPNUMB][35];
  double firconst[APVDAQNUMB][APVCHIPNUMB][FirNumOfParam];
  FirCalib();

public:
  ~FirCalib(){};
  static FirCalib &get_instance(void);
  void StartCalibration();
  bool TickSearch(unsigned int module);
  bool ReadFirData();
  void CalFirParam();

  void AvgFirData();
  void RecordFirParam();
  void ReadFirParam();
  void WriteFirParamToFpga();



  void ClearData();
};

#endif

