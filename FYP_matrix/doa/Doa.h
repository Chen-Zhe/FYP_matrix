#pragma once

#include "svd.h"
#include "DoaFuncs.h"
#include "CFFTW.h"
#include "CGccPhat.h"
#include "DoaOutput.h"
#include <list>

const int SOUND_SPEED=343;
const int MAX_CHANNEL=8;
const float PI = 3.141592;
const float NA = -1000000;

class Doa
{
public:
   Doa(int samplingRate, int nChannels, ulong windowSize, ulong shiftSize);
   virtual ~Doa();

   static micarray::Processor* create(CIniFile* ini, std::string instanceName);
   //socket	
   ClientSocket* udp;
   //brain socket
   ClientSocket* brain;
protected:
   void	OnInitialization();
   void	OnProcessing();
   void	OnTermination();

private:

   int samplingRate_;
   int nChannels_;
   ulong windowSize_;
   ulong shiftSize_;

   vector<float*> windows;

private:
   CHammingWin  hammWin;  // creating a hammingWindow functions
   CFFTW        sig_fft[MAX_CHANNEL];  // 8 mic's channels FFT
   CGccPhat     gccPhat_N[MAX_CHANNEL];  // lets generate 8 pairs of GCC Phat mic00, mic01, mic02,..mic07
   int          range;
   int          gccNmicPair;
   int          gccmicPairList[MAX_CHANNEL];
   float        gccMaxVal[MAX_CHANNEL],  gccMaxMeanRatio[MAX_CHANNEL];
   int          gccMaxIdx[MAX_CHANNEL];

private:	
   
   // copied from old code
   string		ipAddress;
   float**     micPos;
   int			serverConnect;
   int         serverPort;
   float		   theta1,theta2;	
   int         inLookAngleRange;
   int         searchRange;
   float       peakToSecondPeakRatio;
   float       doaDetectThreshold;
   int         minNumMicPairs;

   //new parameters for doa searching space
   int			delay_range;	         //constrain the delay due to mic array size
   int			numMicPairs;
   int			numMics;
   int			Fs;
   float**     P_xx;                   //matrix:vectors from mic i to mic j row: numPairs col:x,y,z
   int			numsTheta1;
   int         numsTheta2;             //number of thetas to be searched
   float*      vectU;                  //unit vector of U
   int**       delayTable;             //matrix: row:numsTheta1*numsTheta2, col: delays of all mic pairs
   int			numRows_delayTable;
   float*      vect_theta1;
   float*      vect_theta2;
   int			sTheta1;
   int         sTheta2;                //search resolutions of theta1 and theta2
   int*        Ref_vect;
   float*      r_GCC_range;            //vector storing GCC values
   float**     GCCTable;		         // a matrix with row: numMicPairs and col: r_GCC_range values   
   float**     accGCCTable;
   float*      GCCTable_sumCol;   
   float*      GCCTable_highestPeak;   
   float*      GCCTable_secondPeak;
   int*        selectedPairs;
   int*        highestPeakIdx;
   int*        secondPeakIdx;
   list<float> prevTheta1s;
   list<float> prevTheta2s;
   int         numMaxPrevFrames;
   int         numMinPrevFrames;
   int         nonDOAFrames;
   bool        hasSpeech;

private:    // methods
   void		InitParameters();
   void		creat_delayTable();
   bool		ProcessDOA();   
   float    findMedian(list<float> mlist);

   FILE*    fp;
};
