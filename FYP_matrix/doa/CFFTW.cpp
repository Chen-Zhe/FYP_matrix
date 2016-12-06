#include "CFFTW.h"
#include <string.h>
#include <math.h>

#define PI 3.14159265358979323

CFFTW::CFFTW()
{
 //printf("ctor %x\n", this);
 N = 0;
 p = NULL;
 in_float    = NULL;
 out_complex = NULL;
 in_complex = NULL;
 out_float = NULL;
 out_tmp = NULL;

}

CFFTW::~CFFTW()
{
 //printf("dtor %x\n", this);
  fftwf_destroy_plan(p);
  fftwf_free(in_complex); 
  fftwf_free(out_complex);
  fftwf_free(in_float); 
  fftwf_free(out_float);
  fftwf_free(out_tmp);

}

void
CFFTW::init_N_r2c(int v_N)
{  N     = v_N;
   N_2p1 = (N/2+1);
   N_2   = (N/2);
   flags = FFTW_FORWARD;

	in_float    = (float*) fftwf_malloc(sizeof(float) * N);
	out_complex = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * N_2p1);
	out_tmp     =  NULL;
	p   = fftwf_plan_dft_r2c_1d(N, in_float, out_complex, FFTW_MEASURE);
}


void
CFFTW::init_N_c2r(int v_N)
{  N     = v_N;
   N_2p1 = (N/2+1);
   N_2   = (N/2);
   flags = FFTW_BACKWARD;

	out_float  = (float*) fftwf_malloc(sizeof(float) * N);
	in_complex = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * N_2p1);
	out_tmp    = (float*) fftwf_malloc(sizeof(float) * N_2p1);
	p          = fftwf_plan_dft_c2r_1d(N, in_complex, out_float, FFTW_MEASURE);
}

void
CFFTW::evaluate_plan()
{
  fftwf_execute(p); /* repeat as needed */

  // scale the op divide by N
  float scaleFac = (float)(1.0/N);
  if (flags == FFTW_BACKWARD)
  {
	  for (int j = 0; j < N; j++)
		out_float[j] *= scaleFac;
  }
}

void
CFFTW::update_input_r2c(float *tmp_in, int v_N)
{
	if(v_N != N) throw("error");
	memcpy(in_float, tmp_in,  sizeof(float)*N);
}


void
CFFTW::update_input_c2r(fftwf_complex* tmp_in, int v_N)
{
	if(v_N != N) throw("error");
	memcpy(in_complex, tmp_in, sizeof(fftwf_complex)*N_2p1);
}


// You only need to readWisdom ONCE
void
CFFTW::readWisdom(char *infileName)
{ char tmpBuf[10000];
  char tmpLine[255];
  int idx = 0;
  tmpBuf[idx] = 0;

  FILE *infile = fopen(infileName,"r");
  if (infile == NULL) return;

  while (feof(infile) == 0)
  {
	  fgets(tmpLine, 255, infile);
	  if (strlen(tmpLine) > 0)
		  strncat(tmpBuf, tmpLine, 10000);
	  idx += (int) (strlen(tmpLine));
  }
	fftwf_import_wisdom_from_string(tmpBuf);	
	fclose(infile);
}

// You only need to writeWisdom ONCE
void
CFFTW::writeWisdom(char *opFileName)
{
	char *planStr = fftwf_export_wisdom_to_string();
	FILE *opfile = fopen(opFileName,"w");
	fprintf(opfile,"%s", planStr);
	fclose(opfile);
	fftwf_free(planStr);
}



void
CFFTW::fftshift()
{
  // NOT doing for FFTW_FORWARD
  if (flags == FFTW_BACKWARD)
  { 
	  memcpy(out_tmp, out_float, sizeof(float)*(N_2p1));

	  if (N%2 == 1) //is Odd
	  {
		  memcpy(&out_float[0], &out_float[N_2p1], sizeof(float)*(N_2));
		  memcpy(&out_float[N_2], out_tmp, sizeof(float)*(N_2p1));
	  }
	  else // is Even
	  {
		  memcpy(&out_float[0], &out_float[N_2], sizeof(float)*N_2);
		  memcpy(&out_float[N_2], out_tmp, sizeof(float)*(N_2));
	  }
  }
}


void 
fftw3_display_complex( const char *nameStr, const fftwf_complex  *out_complex, int N)
{
	printf("Complex Number [%s]\n", nameStr);
	for (int i=0; i < N; i++)
		printf("%d) %f  i%f\n", i, out_complex[i][0],out_complex[i][1]);
	printf("\n");
}

void 
fftw3_display_real( const char *nameStr, const float  *out_real, int N)
{
	printf("Real Number [%s]\n", nameStr);
	for (int i=0; i < N; i++)
		printf("%d) %f\n", i, out_real[i]);
	printf("\n");
}


CHammingWin::CHammingWin()
{
	N = 0;
	w = NULL;
}
void
CHammingWin::init(int vN)
{
	// Eqn w(n) = 0.54 - 0.46*cos( (2*pi*n)/(N-1))
  N   = vN;
  w   = (float*) fftwf_malloc(sizeof(float) * N);
  for (int i=0; i < N; i++)
	  w[i] = (float) (0.54 - 0.46*(cos ((2*PI*i)/(N-1))));
}

CHammingWin::~CHammingWin()
{
	fftwf_free(w); 
}

void
CHammingWin::apply(float *v, int vN)
{
 if(vN!=N) throw("error");
 for (int i=0; i < N; i++)
	  v[i] = v[i]*w[i];
}