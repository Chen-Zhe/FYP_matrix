#include "Doa.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

using namespace std;

Doa::Doa(int samplingRate, int nChannels, ulong windowSize, ulong shiftSize)
:	samplingRate_(samplingRate), nChannels_(nChannels), windowSize_(windowSize), shiftSize_(shiftSize)	

{
   /*
   int samplingRate = 48000
   int nChannels = 8
   ulong windowSize = 46080
   ulong shiftSize = 7680
   */
   numMics = 8;
   micPos = new float*[numMics];

   float mic0Position[3] = { 1.9708196, -4.7579795, 0 };
   float mic1Position[3] = { -2.0291803, -4.7579795, 0 };
   float mic2Position[3] = { -4.8420204, -1.9708196, 0 };
   float mic3Position[3] = { -4.8420204, 2.0291803, 0 };
   float mic4Position[3] = { -2.9291803, 4.8420204, 0 };
   float mic5Position[3] = { 1.9708196, 4.8420204, 0 };
   float mic6Position[3] = { 4.7579795, 2.0291803, 0 };
   float mic7Position[3] = { 4.7579795, -1.9708196, 0 };

   micPos[0] = mic0Position;
   micPos[1] = mic1Position;
   micPos[2] = mic2Position;
   micPos[3] = mic3Position;
   micPos[4] = mic4Position;
   micPos[5] = mic5Position;
   micPos[6] = mic6Position;
   micPos[7] = mic7Position;
  
   inLookAngleRange = 16;
   searchRange = 200;
   peakToSecondPeakRatio = 0;
   doaDetectThreshold = 0;
   minNumMicPairs = 0;
   numMaxPrevFrames = 1;
   numMinPrevFrames = 1;
   nonDOAFrames = 0;
   numsTheta1 = 2 * 90 + 1;
   numsTheta2 = 2 * 90 + 1;
}

//-------------------------------------------------------------------------
//	@summary:
//		Initializes the buffers and/or resources required. This method will be 
//		called just before entering the processing loop
//-------------------------------------------------------------------------
void Doa::initialize()
{
   // init 8 half window buffers
   for (int i=0; i<nChannels_; ++i)
   {
      windows.push_back(new float[windowSize_]);
      memset(windows[i], 0, sizeof(float)*windowSize_);	
   }
	//init parameters
   theta1 = 0.0;
   theta2 = 0.0;

   sig_fft[0].readWisdom("FFTWf_wisdom.dat");
   hammWin.init(windowSize_);	

   // we have to generate the 8 channels of FFTr2c of the signals
   for (int i=0; i < nChannels_; i++)
   {
      sig_fft[i].init_N_r2c(windowSize_);
      gccPhat_N[i].init_N(windowSize_, searchRange);
   }
   sig_fft[0].writeWisdom("FFTWf_wisdom.dat");


   //------------------------------------------------------------
   //added for search space method   
   delay_range = searchRange;	   //constrain the delay based on mic array size	
   create_delayTable();
}

void Doa::create_delayTable()
{
   vect_theta1 = (float *)malloc(numsTheta1*sizeof(int));
   vect_theta2 = (float *)malloc(numsTheta2*sizeof(int));

   vectU = new float[3];     //unit vector of U (Ux,Uy,Uz)

   numMicPairs = numMics*(numMics-1)/2;

   r_GCC_range = new float[delay_range*2+1];

   GCCTable = (float **)malloc(numMicPairs * sizeof(float *));
   for (int i = 0; i< numMicPairs; i++)
      GCCTable[i] = (float *)malloc((delay_range*2+1)*sizeof(float));

   accGCCTable = (float **)malloc(numMicPairs * sizeof(float *));
   for (int i = 0; i< numMicPairs; i++)
      accGCCTable[i] = (float *)malloc((delay_range*2+1)*sizeof(float));

   for (int i=0; i<numMicPairs; i++)
      for (int j=0; j<2*delay_range+1; j++)
         accGCCTable[i][j] = 0;

   GCCTable_sumCol = (float*)malloc(numMicPairs * sizeof(float));
   GCCTable_highestPeak = (float*)malloc(numMicPairs * sizeof(float));
   highestPeakIdx = (int*)malloc(numMicPairs * sizeof(int));
   GCCTable_secondPeak = (float*)malloc(numMicPairs * sizeof(float));
   secondPeakIdx = (int*)malloc(numMicPairs * sizeof(int));
   selectedPairs = (int*)malloc(numMicPairs * sizeof(int));

   P_xx = (float **)malloc(numMicPairs*sizeof(float *));     //matrix:vectors from mic i to mic j row: numPairs col:x,y,z
   for (int i=0; i< numMicPairs; i++)
      P_xx[i] = (float *)malloc(3*sizeof(float));	

   Ref_vect = new int[numMics-1];
   for (int i = 0; i < numMics-1; i++)
      Ref_vect[i] = numMics - 1 - i;

   //be careful here
   int count = 0;
   for (int i = 0; i <= numMics-2; i++)
      for (int j = 1; j <= Ref_vect[i]; j++){
         for (int c = 0; c < 3; c++)
         {
            P_xx[count][c] = (micPos[i+j][c] - micPos[i][c])/100; //from cm to m				
         }
         count++;
      }      

      vect_theta1[0] = 0;
      for (int rowIdx=1; rowIdx<=(numsTheta1-1)/2; rowIdx++)
      {
         vect_theta1[rowIdx] = 180.0/((numsTheta1-1)/2) * rowIdx;
         vect_theta1[rowIdx + (numsTheta1-1)/2] = -vect_theta1[rowIdx];
      } 

      vect_theta2[0] = 0;
      for (int colIdx=1; colIdx<=(numsTheta2-1)/2; colIdx++)
      {
         vect_theta2[colIdx] = 90.0 /((numsTheta2-1)/2) * colIdx;
         vect_theta2[colIdx + (numsTheta2-1)/2] = -vect_theta2[colIdx];
      } 

      numRows_delayTable = numsTheta1*numsTheta2;
      delayTable = (int **)malloc(numRows_delayTable*sizeof(int *));  //matrix: row:numsTheta1*numsTheta2, col: delays of all mic pairs	
      for (int i = 0; i< numRows_delayTable; i++)
         delayTable[i] = (int *)malloc(numMicPairs*sizeof(int));

      count = 0;
      float tmp_val = 0.0;

      for (int row = 0; row < numsTheta1; row ++)
         for (int col = 0; col < numsTheta2; col ++)
         {
            vectU[0] = cos((float)vect_theta1[row]*PI/180)*cos((float)vect_theta2[col]*PI/180); //Ux
            vectU[1] = sin((float)vect_theta1[row]*PI/180)*cos((float)vect_theta2[col]*PI/180); //Uy
            vectU[2] = sin((float)vect_theta2[col]*PI/180); //Uz
            for (int p = 0; p < numMicPairs; p++){
               tmp_val = 0.0;
               for (int c = 0; c < 3; c++)
                  tmp_val += P_xx[p][c] * fabs(vectU[c]);
               delayTable[count][p] = (int) (-tmp_val * samplingRate_/SOUND_SPEED);
               
            }
            count++;
         }
}


//-------------------------------------------------------------------------
//	@summary:
//		Main processing method for consuming a known number of input buffers,
//		and producing a known number of output buffers.
//	
//	@remarks:
//		There is no direct input/output parameters to be passed to this method.
//		Instead, input/output information can be extracted from the protected 
//		members <Inputs> and <Outputs> of the super-class Processor, each of 
//		which is a vector of StreamRecord*. 
//
//		<StreamRecord::data> contains the pointer to the current input/output stream.
//		A Processor needs to cast this field to its required input/output data type
//		prior to doing the main processing code.
//
//		e.g. A TypeConverter which converts from int to float will prepare its 
//		stream as flowing, assuming that 1 TypeConverter block takes in only 1 
//		buffer and output only 1 buffer:	
//					int*	 input  = (int*)  Inputs[0]->data
//					float* output = (float*)Output[0]->data
//
//		Note that casting type must be consistent with the data type of each item 
//		when declaring the buffer.
//		
//
//		After the main processing, the processor need to update the number of 
//		processed items to the <StreamRecord::processed> field of each record 
//		from the <Inputs> and <Outputs> vectors.
//
//		e.g.	Inputs[0]->processed = Inputs[0]->length; // indicate that all input data has been consumed
//					Outputs[0]->processed = 1;								// indicate that the buffer has produced 1 output item
//-------------------------------------------------------------------------
DoaOutput Doa::processBuffer(float* buffer, bool vadPositive)
{
#ifdef _DEBUG_DOA
	clock_t start, end;
	start = clock();
#endif

   //
   // prepare your input/output streams here using Inputs and Outputs vectors
   //
   for(int i=0; i<nChannels_; ++i)
   {
      float* input = buffer+ shiftSize_*i;
      //if(samplesPerChannel < shiftSize_) throw("wrong buffer size");

      ulong reservedSize = windowSize_ - shiftSize_;
      memmove(windows[i], windows[i]+shiftSize_, reservedSize << 2);	// copy reservedSize from the 2nd part to the 1st part (use <<2 for faster multiplied by sizeof(float))	
	  memmove(windows[i] + reservedSize, input, shiftSize_ <<2);	
   }
  
   DoaOutput output;
   
   // get ready the fft of each channel's data first!nChannels_
   for(int i=0; i<nChannels_; i++)
   {
      sig_fft[i].update_input_r2c(windows[i], windowSize_);
      hammWin.apply(sig_fft[i].get_pInReal(), windowSize_);
      sig_fft[i].evaluate_plan();
   } 

   bool hasDOA = ProcessDOA(vadPositive)&&vadPositive;
   output.hasDOA = hasDOA;

   if (hasDOA){
      output.theta1 = theta1;
      output.theta2 = theta2;

	  float theta1_sent;
	  float adjust_theta1 = output.theta1;
	  if (adjust_theta1 >= -90 && adjust_theta1 <= 180)
		  theta1_sent = adjust_theta1 - (float)90;
	  if (adjust_theta1<-90 && adjust_theta1>-180)
		  theta1_sent = (float)270 + adjust_theta1;
	  if (output.theta2 == 0.0)
		  output.theta2 = 0.0;
	  if (output.theta1 == -200)
		  theta1_sent = (float)0.0;
	  std::cout << "adj: " << theta1_sent << std::endl;
   }

#ifdef _DEBUG_DOA
   end = clock();
   double elapsed = ((double) (end-start)) / CLOCKS_PER_SEC * 1000;
   printf("DOA took %.2f milliseconds.\n", elapsed);
#endif

   return output;
}

bool Doa::ProcessDOA(bool vadPositive)
{
   bool hasDOA;
   int row, col;   

   int pairs = 0;
   for (int i = 0; i <= numMics-2; i++)
   {
      for (int j = 1; j <= Ref_vect[i]; j++)
      {
         gccPhat_N[0].evaluate_FFT_gccPhat(sig_fft[i+j].get_pOutComplex(), sig_fft[i].get_pOutComplex(), windowSize_);
         gccPhat_N[0].fn_extractGccStats(GCCTable, delay_range, pairs);
		 
		 if (vadPositive)
			for (int k=0; k<2*delay_range+1; k++)
				accGCCTable[pairs][k] += GCCTable[pairs][k];

         pairs++;
      }
   }

   int peakIdx;
   int pairCount=0;

   for (int i=0; i<numMicPairs; i++)
   {
      GCCTable_sumCol[i] = 0;
      GCCTable_highestPeak[i] = 0.0;
      GCCTable_secondPeak[i] = 0.0;

      for (int j=0; j<=2*delay_range; j++)            
      {
         GCCTable_sumCol[i] += fabs(GCCTable[i][j]);

         if (GCCTable_highestPeak[i] <= (GCCTable[i][j]))
         {
            GCCTable_highestPeak[i] = (GCCTable[i][j]);
            highestPeakIdx[i] = j;
         }      
      }        

      for (int j=0; j<=2*delay_range; j++) 
      {
         if ((GCCTable_secondPeak[i] <= (GCCTable[i][j])) && (abs(j-highestPeakIdx[i]) > inLookAngleRange))         
         {
            GCCTable_secondPeak[i] = (GCCTable[i][j]); 
            secondPeakIdx[i] = j;
         }
      }

      if (((GCCTable_highestPeak[i] / GCCTable_secondPeak[i]) > peakToSecondPeakRatio))
      {
         selectedPairs[i] = 1;
         pairCount++;
      }
      else
         selectedPairs[i] = 0;  
      
#ifdef _DEBUG_DOA
      printf("Peak ratio: %f \n",(GCCTable_highestPeak[i] / GCCTable_secondPeak[i]));
#endif
   }
    
   if (pairCount < minNumMicPairs)
      hasDOA = false;
   else 
      hasDOA = true;
   
   int search_idx;
   int maxIdx = 0;   
   float maxRatio = 0.0;   
   float ratioInOutLookAngle, sumInLookAngle, sumOutLookAngle; 

   for (int n = 0; n < numRows_delayTable; n++)
   {              
      ratioInOutLookAngle = 0;
      int tempPairCount = 0;
      for (int p=0; p<numMicPairs; p++)
      { 
         search_idx = delayTable[n][p] + delay_range; 

         if ((GCCTable_highestPeak[p] > 1.0) || (selectedPairs[p] ==0))
            continue;

         sumInLookAngle = GCCTable[p][search_idx];
         sumOutLookAngle = (GCCTable_sumCol[p] - GCCTable[p][search_idx]) / (2*delay_range);         
         //ratioInOutLookAngle += sumInLookAngle / sumOutLookAngle; 
         ratioInOutLookAngle += (sumInLookAngle / GCCTable_secondPeak[p]);
		 tempPairCount++;
      }

      ratioInOutLookAngle = ratioInOutLookAngle / tempPairCount;
	  //std::cout << ratioInOutLookAngle << "  " << tempPairCount << std::endl;

      if (maxRatio < ratioInOutLookAngle){         
         maxRatio = ratioInOutLookAngle;
         maxIdx = n;
      }
   }

   row = maxIdx / numsTheta2;   
   col = maxIdx % numsTheta2;

   theta1 = vect_theta1[row];
   theta2 = vect_theta2[col];
   if (col>=numsTheta2 || theta2<-90)
      theta2 = 0;	   

   if (hasDOA && vadPositive)
   {
      nonDOAFrames--;
      nonDOAFrames = max(nonDOAFrames,0);

      prevTheta1s.push_back(theta1);
      if ((int) prevTheta1s.size() == numMaxPrevFrames + 1)
         prevTheta1s.pop_front();

      prevTheta2s.push_back(theta2);
      if ((int) prevTheta2s.size() == numMaxPrevFrames + 1)
         prevTheta2s.pop_front();
   }
   else
   {
      nonDOAFrames++;
      nonDOAFrames = min(nonDOAFrames, numMaxPrevFrames);

      if (nonDOAFrames + (int)prevTheta1s.size() > numMaxPrevFrames)
      {
         prevTheta1s.pop_front();
         prevTheta2s.pop_front();
      }
   }

   hasDOA = hasDOA && (maxRatio > doaDetectThreshold);
   //printf("hasDOA = %d NumPairs = %d InOutLookAngle Ratio = %f \n",hasDOA,pairCount,maxRatio);    
      

   if ((int) prevTheta1s.size() < numMinPrevFrames)
      hasDOA = false;
   else
   {
      theta1 = findMedian(prevTheta1s);
      theta2 = findMedian(prevTheta2s);
   }

   return hasDOA;
}

float Doa::findMedian(list<float> mlist)
{
   mlist.sort();
   list<float>::iterator it;
   
   int n = (int) mlist.size();

   int idx = 0;
   float v = 0.0;
   if (n%2==0)
   {
      for (it=mlist.begin() ; it != mlist.end(); it++ )
      {
         if (idx==n/2-1)
            v += (*it);

         if (idx==n/2)
         {
            v += (*it);
            break;
         }
         idx++;  
      }
      v /= 2;
   }
   else 
   {
      for (it=mlist.begin() ; it != mlist.end(); it++ )
      {
         if (idx==n/2)
         {
            v += (*it);
            break;
         }
         idx++;  
      }
   }
   
   return v;
}

//-------------------------------------------------------------------------
//	@summary:
//		Releases the buffers and/or resources allocated during initialization. 
//		This method will be automatically called when the DOA object is out of scope
//-------------------------------------------------------------------------
Doa::~Doa()
{

	delete[] r_GCC_range;

	for (int i = 0; i<numMicPairs; i++)
	{
		delete[] GCCTable[i];
		delete[] accGCCTable[i];
		delete[] P_xx[i];
	}
	delete[] GCCTable;
	delete[] accGCCTable;
	delete[] GCCTable_sumCol;
	delete[] P_xx;

	delete[] GCCTable_highestPeak;
	delete[] highestPeakIdx;
	delete[] secondPeakIdx;
	delete[] GCCTable_secondPeak;
	delete[] vect_theta1;
	delete[] vect_theta2;
	delete[] selectedPairs;

}