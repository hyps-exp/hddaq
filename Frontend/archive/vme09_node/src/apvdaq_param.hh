//-------------------
//setparam.h
//ASANO,Hidemitsu
//
//ver1.0 2009 10 8
//
//last modified 2 9
//------------------------
#ifndef apvdaq_param_h
#define apvdaq_param_h

//Address 

#define MASTER    0x11000000
#define SLAVE1    0x12000000
#define SLAVE2    0x13000000
#define SLAVE3    0x14000000
#define SLAVE4    0x15000000
#define SLAVE5    0x16000000
#define SLAVE6    0x17000000
#define SLAVE7    0x18000000
#define SLAVE8    0x19000000
#define SLAVE9    0x1a000000
#define SLAVEA    0x1b000000
#define SLAVEB    0x1c000000



#define BROADCAST 0x80000000 

//#define SINGLE_PEAK_MODE
#define MULTI_PEAK_MODE
//#define CALIBRATION_MODE

#define SINGLE_SEQ_CELL 83

#define MULTI_SEQ_CELL2 36
#define MULTI_SEQ_CELL3 39
#define MULTI_SEQ_CELL4 42

// by E07-2017 05.29
// #define MULTI_SEQ_CELL2 37
// #define MULTI_SEQ_CELL3 40
// #define MULTI_SEQ_CELL4 43

//Oct 2015
// #define MULTI_SEQ_CELL2 50
// #define MULTI_SEQ_CELL3 53
// #define MULTI_SEQ_CELL4 56

// At Nagoya 20140117 -- Perfect Timing
//#define MULTI_SEQ_CELL1 75
//#define MULTI_SEQ_CELL2 76
//#define MULTI_SEQ_CELL3 79
//#define MULTI_SEQ_CELL4 82
//#define MULTI_SEQ_CELL4 84

// For LED test
////#define MULTI_SEQ_CELL1 75
//#define MULTI_SEQ_CELL2 84
//#define MULTI_SEQ_CELL3 87
////#define MULTI_SEQ_CELL4 84



//#define MULTI_SEQ_CELL2 108
//#define MULTI_SEQ_CELL3 111


//#define MULTI_SEQ_CELL1 40
//#define MULTI_SEQ_CELL2 43
//#define MULTI_SEQ_CELL3 46
//#define MULTI_SEQ_CELL4 49
//#define CALIBRATION_SEQ_CELL1 52
//#define CALIBRATION_SEQ_CELL2 55
//#define CALIBRATION_SEQ_CELL3 58
//#define CALIBRATION_SEQ_CELL4 61

//#define CALIBRATION_SEQ_CELL1 132
//#define CALIBRATION_SEQ_CELL2 79
//#define CALIBRATION_SEQ_CELL3 82
//#define CALIBRATION_SEQ_CELL4 141

// Correct Timing !! -- Maybe Latency 0x5f + 2 = 97
//#define CALIBRATION_SEQ_CELL2 98
//#define CALIBRATION_SEQ_CELL3 101
#define CALIBRATION_SEQ_CELL2 96
#define CALIBRATION_SEQ_CELL3 99


#define APV_CHANNELS  128
#define MAX_APV         3

#define ADC_TOTAL_WORDS 8192
#define FIFO_CELLS_TO_READ 1500
#define MAX_I2C_RETRIES  5

//unsigned short adc_rawdata1[MAX_APV][ADC_TOTAL_WORDS];



//apv_i2c_settings [register adrress][value]
const unsigned short apv_i2c_settings[16][2] = {	
#ifdef SINGLE_PEAK_MODE
	{MODE,0x3f}, 
	// Mode of Operation of Chip 
    // D[5]:Preamp Polaority 0:Non-Inverting 1:Inverting   :1
    // D[4]:Read-Out Frequency 0:20MHz 1:40MHz             :1
    // D[3]:Read-Out Mode 0:Deconvolution 1:Peak           :1
    // D[2]:Calibration Inhibit 0:OFF 1:ON                 :1
    // D[1]:Trigger Mode 0:3-sample 1:1-sample             :1
    // D[0]:Analogue Bias 0:OFF 1:ON                       :1
#endif

#ifdef MULTI_PEAK_MODE
	{MODE,0x3d},
	//{MODE,0x1d},
    // Mode of Operation of Chip 
    // D[5]:Preamp Polaority 0:Non-Inverting 1:Inverting   :1
    // D[4]:Read-Out Frequency 0:20MHz 1:40MHz             :1
    // D[3]:Read-Out Mode 0:Deconvolution 1:Peak           :1
    // D[2]:Calibration Inhibit 0:OFF 1:ON                 :1
    // D[1]:Trigger Mode 0:3-sample 1:1-sample             :0
    // D[0]:Analogue Bias 0:OFF 1:ON                       :1
#endif

#ifdef CALIBRATION_MODE
	{MODE,0x19},
	//{MODE,0x39},
	// Mode of Operation of Chip 
    // D[5]:Preamp Polaority 0:Non-Inverting 1:Inverting   :0
    // D[4]:Read-Out Frequency 0:20MHz 1:40MHz             :1
    // D[3]:Read-Out Mode 0:Deconvolution 1:Peak           :1
    // D[2]:Calibration Inhibit 0:OFF 1:ON                 :0
    // D[1]:Trigger Mode 0:3-sample 1:1-sample             :0
    // D[0]:Analogue Bias 0:OFF 1:ON                       :1
    
	//apvdaq.c で2 trigger cell を定義 なので今は6 sample
#endif	

	{LATENCY,0x5f}, // delay between Write and Trigger Pointer in the pipeline memory control.
	//{LATENCY,0x84}, // delay between Write and Trigger Pointer in the pipeline memory control.
				    // 8bit binary number (default:0x84 = 132 clockcycles )       
				    // 0-192 
	{IPRE,0x62},    // Preamp Input FET(Field effect transistor) 
	                // 0-1020 uA n*4uA default:0x62
	{IPCASC,0x34},  // Preamp Cascode Current Bias 
	                // 0-255uA  default 0x34
	{IPSF,0x22},    // Preamp Source Follower Current Bias
	                // 0-255uA default 0x22
	{ISHA,0x22},    // Shaper Input FET Current Bias
	                // 0-255uA dafault 0x22
	{ISSF,0x22},    // Shaper Source Follower Current Bias
	                // 0-255uA default 0x22
	{IPSP,0x37},    // APSP (Analogue Pulse Shape Processor) Current Bias
	                // 0-255uA default 0x37
	{IMUXIN,0x22},  // Multiplexer Input Current Bias
	                // 0-255uA default 0x22
	{VFP,0x1e},     // Preamp Feedback Voltage Bias
	                // -1.25 to 0.65V -1.25V + (7.5mV*n)  default 0x1e
	{VFS,0x3c},     // Shaper Feedback Voltage Bias
	                // -1.25 to 0.65V -1.25V + (7.5mV*n)  default 0x3c 

	{VPSP,0x1e},    // APSP Voltage Level Adjust
                    // -0.65 to 1.25V +1.25V - (7.5mV*n)  default 0x1e
	{MUXGAIN,0x04}, // Sets Gain of Multiplexer
	                // 8-bit pattern which defines which size resister to use in the input stage of the multiplexer.
					//                                    default 0x04  
	{ICAL,0x1a},    // Calibrate Edge Generator Current  Bias 
	                // 1 mip = 26 (0x1a) 

	{CSEL,0x00},    // Calibrate Delay Select
	//{CDRV,0x2}     // Calibrate Output Mask
	{CDRV,0x55}     // Calibrate Output Mask
	//{CDRV,0xdb}     // Calibrate Output Mask
};

//setting CNT register

#define ALLOWED_TRIGGERS 1 //when trigger mode is sequencer ,this is always 1

//#define SHIFT_REGISTER_DELAY 0

#define SHIFT_REGISTER_DELAY 75
#define FRAME_LENGTH 138
#define TICK_LENGTH 9 //30m 40MHz mode

#define ADC_OFFSET 0x64 //

//setting PHOS delay-chip 
#define MASTER_PHOS_ADCDELAY 7
#define SLAVE1_PHOS_ADCDELAY 8
#define SLAVE2_PHOS_ADCDELAY 7

//#define SLAVE3_PHOS_ADCDELAY 7 //original
#define SLAVE3_PHOS_ADCDELAY 6 
//#define SLAVE3_PHOS_ADCDELAY 5 fail
//#define SLAVE3_PHOS_ADCDELAY 8 better
//#define SLAVE3_PHOS_ADCDELAY 9 fail

#define SLAVE4_PHOS_ADCDELAY 5
#define SLAVE5_PHOS_ADCDELAY 7

#define SLAVE6_PHOS_ADCDELAY 8
#define SLAVE7_PHOS_ADCDELAY 10
//#define SLAVE8_PHOS_ADCDELAY 10
//#define SLAVE9_PHOS_ADCDELAY 2
#define PHOS_TRGDELAY 0




//Jaeyong newly added parameters 18 Aug 2015
//updated 19 September 2015

#define DATALOCA      "/home/sks/jaeyong_work/SugiDaqSsd_v1.7/FPGA_v1.7/conf/" // data location
#define SOURCELOCA    "/home/sks/jaeyong_work/SugiDaqSsd_v1.7/FPGA_v1.7/conf/" // source location
#define WARMUPLOCA    "/home/sks/jaeyong_work/SugiDaqSsd_v1.7/FPGA_v1.7/conf/" // warmup file calibration

#define FirNumOfParam 12
#define NumberOfSamples 8
#define FIRONOFF     1 // 1: on, 0: off
#define FIRONOFFSLV3 0 // 1: on, 0: off BAD MODULE Recommended to Turn off FIR Filter
#define FIRCALIB     0 // 1: do, 0: don't
#define FIRTEST      0 // 1: do, 0: don't
#define MAXFIRCAL    5000 // = TOTAL SAMPLE # / 100

#define APVDAQNUMB   6
#define APVCHIPNUMB  4 //3 for old sensor, 4 for new sensor

#define RAWZSSEL     1    //0 : test mode, 1 : ZS mode
#define L2TRG        1     //0 : OFF , 1 : ON

const  unsigned long PulseShapeParameter[4][2] = { {0x30c, 0x1000000},
						   {0x30c, 0x1000000},
						   {0x30c, 0x1000000},
						   {0x30c, 0x1000000} };
/*
static unsigned long PulseShapeParameter[4][2] = { {0x0, 0x0},
						   {0x0, 0x0},
						   {0x0, 0x0},
						   {0x0, 0x0} };
*/
#define DevThreshold 0.0
//const double MaxPedThreshold = 40.0;
// const double PedThreshold[6] =
//   //    {10.0,10.0,10.0,10.0,10.0,10.0};
//       {4.0,4.0,4.0,4.0,4.0,4.0};
  //{0.5,0.5,0.5,0.5,0.5,0.5};
  //      {2.0,2.0,2.0,2.0,2.0,2.0};
//  {10.0,10.0,10.0,10.0,10.0,10.0}; -- used at 06/12/2016

const double PedOffset[6][4] =
  {
    {0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0}
  };

//-------------------
//setparam.h +
//Joo Changwoo
//ver1.0 2011 11 08
//last modified 11 11 08
//------------------------


#define MAXADC       512   //maximum adc value for calibration data
#define MINADC       200   //minimum adc value for calibration data


#endif 
