#include<iostream>
#include<string>
#include<sstream>
#include<cstdlib>
#include<cstdio>
#include<cerrno>
#include"FileManager.hh"
#include"easiroc_config.hh"

// ---------------------------------------------------------------------------
// Slow control 
// ---------------------------------------------------------------------------
unsigned int slowdata[sizeByte_SC];

int PrepareSC(){
  std::ifstream file("./autoconfig/AutoConfig_SC.txt");
  if(!file.is_open()){
    return -1;
  }

  for(int i = 0; i<sizeByte_SC; ++i){
    slowdata[i] = 0;
  }

  FileManager FileMan(file);
  StringManager *SCName = FileMan.GetStringManager("SCName");
  
  int Loop_name     = SCName->GetVectorSize("ResisterName");
  int CurrentBufNum = sizeByte_SC -1;
  int IncrementNum  = 0;
  for(int i = 0; i<Loop_name; ++i){
    std::string name = SCName->GetString(i, "ResisterName");
    //    std::cerr << name.c_str() << std::endl;
    ParameterManager *resister = FileMan.GetParameterManager(name.c_str());
    int ArrayNum = resister->GetParameter(0, "ArrayNum");
    int BitNum   = resister->GetParameter(0, "BitNum");
    int BitOrder = resister->GetParameter(0, "BitOrder");
    for(int ch = 0; ch<ArrayNum; ++ch){
      std::ostringstream word;
      word << "A" << ch;
      std::string Channel = word.str();

      unsigned int ResVal = resister->GetParameter(0, Channel.c_str());
      if(BitOrder == 0){
	for(int bit = 0; bit<BitNum; ++bit){
	  slowdata[CurrentBufNum] = (slowdata[CurrentBufNum] << 1)+ ((ResVal >> bit) & 1);
	  //	  std::cerr << "Current val " << buf[CurrentBufNum] << std::endl;

	  ++IncrementNum;
	  if(IncrementNum == 32){
	    --CurrentBufNum;
	    IncrementNum = 0;
	  }
	}
      }else{
	for(int bit = 0; bit<BitNum; ++bit){
	  slowdata[CurrentBufNum] = (slowdata[CurrentBufNum] << 1) + ((ResVal >> (BitNum - bit - 1)) & 1);
	  
	  //	  std::cerr << "Current val " << buf[CurrentBufNum] << std::endl;

	  ++IncrementNum;
	  if(IncrementNum == 32){
	    --CurrentBufNum;
	    IncrementNum = 0;
	  }
	}
      }
      //      std::cerr << "CurrentBufNum " << CurrentBufNum << std::endl;
    }
  }

  //  buf[0] = buf[0] >> 1;

//   int TotalBit = 0;
  
//   for(int bit = 0; bit<8; ++bit){
//     if(((slowdata[0] >> bit) & 1)){
// 	std::cerr << "!" << std::flush;
//       }else{
// 	std::cerr << "." << std::flush;
//       }
//     ++TotalBit;
//   }

//   for(int i = 1; i<15; ++i){
//     for(int bit = 0; bit<32; ++bit){
//       if(((slowdata[i] >> bit) & 1)){
// 	std::cerr << "!" << std::flush;
//       }else{
// 	std::cerr << "." << std::flush;
//       }
//       ++TotalBit;
//     }
//   }

//   std::cerr << TotalBit << std::endl;

  return 0;
}

// ---------------------------------------------------------------------------
// Probe 
// ---------------------------------------------------------------------------
unsigned int Probedata[sizeByte_PSC];

int PreparePSC(){
  enum Asign{fs_all, ssh_16, ssh_0, pa_16, pa_0, sizeAsign};
  enum DataName{fs, ssh_hg, ssh_lg, pa_hg, pa_lg, sizeDataName};
  static DataName CurrentProbe = fs;
  static unsigned int CurrentCh = 0;
  static unsigned int ProbeBuffer = 0;
  bool         flag_rst  = false;
  bool flag_input = true;
  for(int i = 0; i<sizeByte_PSC; ++i){
    Probedata[i]   = 0;
  }

  std::cerr << "Probe select" << std::endl;
  std::cerr << " 1. OUT PA HG" << std::endl;
  std::cerr << " 2. OUT PA LG" << std::endl;
  std::cerr << " 3. OUT SSh HG" << std::endl;
  std::cerr << " 4. OUT SSh LG " << std::endl;
  std::cerr << " 5. OUT FS" << std::endl;
  std::cerr << " 6. Next" << std::endl;
  std::cerr << " 7. Previous" << std::endl;  
  std::cerr << " 8. Reset" << std::endl;
  std::cerr << "input # ====> " << std::flush;
  char buf[10];
  fgets(buf, sizeof(buf),stdin);
  char num;
  sscanf(buf,"%c",&num);
  
  switch(num){
  case '1':
    CurrentProbe = pa_hg;
    break;
      
  case '2':
    CurrentProbe = pa_lg;
    break;
      
  case '3':
    CurrentProbe = ssh_hg;
    break;

  case '4':
    CurrentProbe = ssh_lg;
    break;

  case '5':
    CurrentProbe = fs;
    break;

  case '6':
    
    break;

  case '7':
    
    break;

  case '8':
    flag_rst     = true;
    CurrentCh    = 0;
    ProbeBuffer  = 0;
    CurrentProbe = fs;
    break;

  default:
    break;
  };

  if(flag_rst){
    return 0;
  }

  if(flag_input){
    std::cerr << "Input channel number ====> " << std::flush;
    std::cin >> CurrentCh;
    if(CurrentCh > 31){
      std::cerr << "Index is too big" << std::endl;
      CurrentCh = 31;
    }
    if(CurrentCh < 0){
      std::cerr << "Index is too small" << std::endl;
      CurrentCh = 0;
    }
    ProbeBuffer = 1 << (31 - CurrentCh);
  }

  unsigned int buffer_16 = ProbeBuffer*ProbeBuffer;
  unsigned int buffer_0  = (ProbeBuffer >> 16)*(ProbeBuffer >> 16);
  if(CurrentProbe == ssh_hg || CurrentProbe == pa_hg){
    buffer_16 = buffer_16 << 1;
    buffer_0  = buffer_0 << 1;
  }
  if(CurrentProbe == fs){
    Probedata[fs_all] = ProbeBuffer;
  }else if(CurrentProbe == ssh_hg || CurrentProbe == ssh_lg){
    if(CurrentCh > 15){
      Probedata[ssh_16] = buffer_16;
    }else{
      Probedata[ssh_0]  = buffer_0;
    }
  }else if(CurrentProbe == pa_hg || CurrentProbe == pa_lg){
    if(CurrentCh > 15){
      Probedata[pa_16] = buffer_16;
    }else{
      Probedata[pa_0]  = buffer_0;
    }
  }else{
    std::cerr << "bug desu" << std::endl;
  }

  for(int shift = 0; shift<32; ++shift){
    if(((Probedata[fs_all]) >> shift) & 1){
      std::cerr << "!" << std::flush;
    }else{
      std::cerr << "." << std::flush;
    }
  }
  std::cerr << std::endl;

  for(int index = ssh_16; index <= ssh_0; ++index){
    for(int shift = 0; shift<32; ++shift){
      if(((Probedata[index]) >> shift) & 1){
	std::cerr << "!" << std::flush;
      }else{
	std::cerr << "." << std::flush;
      }
    }
  }
  std::cerr << std::endl;

  for(int index = pa_16; index <= pa_0; ++index){
    for(int shift = 0; shift<32; ++shift){
      if(((Probedata[index]) >> shift) & 1){
	std::cerr << "!" << std::flush;
      }else{
	std::cerr << "." << std::flush;
      }
    }
  }
  std::cerr << std::endl;

  return 0;
}

// ---------------------------------------------------------------------------
// Read SC
// ---------------------------------------------------------------------------
unsigned int ReadSCdata[sizeByte_RSC];

int PrepareReadSC(){
  std::ifstream file("./autoconfig/AutoConfig_ReadSC.txt");
  if(!file.is_open()){
    return -1;
  }

  for(int i = 0; i<sizeByte_RSC; ++i){
    ReadSCdata[i] = 0;
  }

  FileManager FileMan(file);
  StringManager *RSCName = FileMan.GetStringManager("ReadSCName");
  
  int CurrentBufNum = sizeByte_RSC -1;
  int IncrementNum  = 0;

  std::string name = RSCName->GetString(0, "ReadResisterName");
  std::cerr << name << std::endl;
  ParameterManager *resister = FileMan.GetParameterManager(name.c_str());
  int ArrayNum = resister->GetParameter(0, "ArrayNum");
  int BitNum   = resister->GetParameter(0, "BitNum");
  int BitOrder = resister->GetParameter(0, "BitOrder");
  for(int ch = 0; ch<ArrayNum; ++ch){
    std::ostringstream word;
    word << "A" << ch;
    std::string Channel = word.str();

    unsigned int ResVal = resister->GetParameter(0, Channel.c_str());
    if(BitOrder == 0){
      for(int bit = 0; bit<BitNum; ++bit){
	ReadSCdata[CurrentBufNum] = (ReadSCdata[CurrentBufNum] << 1)+ ((ResVal >> bit) & 1);
	++IncrementNum;
	if(IncrementNum == 32){
	  --CurrentBufNum;
	  IncrementNum = 0;
	}
      }
    }else{
      for(int bit = 0; bit<BitNum; ++bit){
	ReadSCdata[CurrentBufNum] = (ReadSCdata[CurrentBufNum] << 1) + ((ResVal >> (BitNum - bit - 1)) & 1);
	++IncrementNum;
	if(IncrementNum == 32){
	  --CurrentBufNum;
	  IncrementNum = 0;
	}
      }
    }
  }

  bool Block2bit = false;

  std::cerr << "Ch 32 <<               >> Ch 0" << std::endl;

  for(int shift = 0; shift<32; ++shift){
    if(((ReadSCdata[0]) >> shift) & 1){
      std::cerr << "!" << std::flush;
      if(Block2bit){
	std::cerr << std::endl;
	std::cerr << "### Error Read SC ###" << std::endl;
	exit(-1);
      }else{
	Block2bit = true;
      }
    }else{
      std::cerr << "." << std::flush;
    }
  }

  std::cerr << std::endl;
  std::cerr << std::endl;

  return 0;
}

// ---------------------------------------------------------------------------
// Debug FPGA
// ---------------------------------------------------------------------------
unsigned int Pindata[sizeByte_PIN];

int PrepareFPGA(){
  std::ifstream file("./autoconfig/AutoConfig_FPGA.txt");
  if(!file.is_open()){
    return -1;
  }

  for(int i = 0; i<sizeByte_RSC; ++i){
    Pindata[i] = 0;
  }

  FileManager FileMan(file);
  StringManager *PinName = FileMan.GetStringManager("PinName");
  
  int Loop_Word     = PinName->GetVectorSize("PinName");
  int CurrentBufNum = sizeByte_PIN  -1;
  int IncrementNum  = 0;

  for(int i = 0; i<Loop_Word; ++i){
    std::string name = PinName->GetString(i, "PinName");
    ParameterManager *resister = FileMan.GetParameterManager(name.c_str());
    for(int bitNum = 7; bitNum>-1; --bitNum){
      std::ostringstream word;
      word << "bit" << bitNum;
      std::string bitName = word.str();
      int bit = resister->GetParameter(0, bitName.c_str());
      if(bit){
	Pindata[CurrentBufNum] = (Pindata[CurrentBufNum] << 1) + 1;
      }else{
	Pindata[CurrentBufNum] = (Pindata[CurrentBufNum] << 1) + 0;
      }
      ++IncrementNum;
      if(IncrementNum == 32){
	--CurrentBufNum;
	IncrementNum = 0;
      }
    }
  }

//   std::cerr << "bit0 <<  >> bit7" << std::endl;
   
//   for(int i = 0; i<8; ++i){
//     if((Pindata[0] >> i) & 1){
//       std::cerr << "!" << std::flush;
//     }else{
//       std::cerr << "." << std::flush;
//     }
//   }

//   for(int i = 0; i<32; ++i){
//     if(0 == i%8){
//       std::cerr << "   word_" << i/8 << std::endl;
//     }

//     if((Pindata[1] >> i) & 1){
//       std::cerr << "!" << std::flush;
//     }else{
//       std::cerr << "." << std::flush;
//     }
//   }
//   std::cerr << "   word_5" << std::endl;
  return 0;
}

void Debug(){
//   std::ifstream file("./tmp/SCName.txt");
  
//   StringManager SCName(file);
//   std::ofstream AutoSC("./autoconfig/AutoConfig_SC.txt",std::ios::app);
  
//   int Loop_name     = SCName.GetVectorSize("ResisterName");
//   for(int i = 0; i<Loop_name; ++i){
//     std::cerr << SCName.GetString(i, "ResisterName") << std::endl;
//     AutoSC << "NameString " << SCName.GetString(i, "ResisterName") << std::endl;
//     AutoSC << "./setup/" << SCName.GetString(i, "ResisterName") << ".txt" << std::endl;
//     AutoSC << std::endl;
//   }
  
  return;
}
