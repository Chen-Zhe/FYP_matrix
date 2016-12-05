// FileName: CFFTW.h
// Author: Chng Eng Siong
// 15 Oct 2009

#ifndef INC_CFFTW
#define INC_CFFTW

#include "fftw/fftw3.h"

class CFFTW
{
private:
int           N;
int           N_2p1;
int           N_2;
unsigned int  flags; // 
//r2c DFTs are always FFTW_FORWARD and c2r DFTs are always FFTW_BACKWARD.
fftwf_plan      p;

float			*window;  // we will initialize a window

float          *in_float;
fftwf_complex  *out_complex;

float          *out_float;
float          *out_tmp;  // for fftshift
fftwf_complex  *in_complex;


public:
	CFFTW();
	~CFFTW();

	void init_N_r2c(int v_N);
    void init_N_c2r(int v_N);

	
    void evaluate_plan();
	void fftshift();

	void update_input_r2c(float *tmp_in, int N);
	void update_input_c2r(fftwf_complex  *in_complex, int N);
	int  get_N_2p1() { return N_2p1;};
	int  get_N() { return N;};

	fftwf_complex *get_pOutComplex() { return out_complex;};
	float *get_pOutReal() { return out_float;};

	fftwf_complex *get_pInComplex() { return in_complex;};
	float *get_pInReal() { return in_float;};

	void  readWisdom(char *infileName);
	void  writeWisdom(char *opFileName);
};

void
fftw3_display_complex( const char *nameStr, const fftwf_complex  *out_complex, int N);

void 
fftw3_display_real( const char *nameStr, const float  *out_real, int N);



class CHammingWin
{
private:
	// Eqn w(n) = 0.54 - 0.46*cos( (2*pi*n)/(N-1))
int   N;
float *w;

public:
	CHammingWin();
	~CHammingWin();
	void init(int vN);
	void apply(float*v, int vN);

};
#endif