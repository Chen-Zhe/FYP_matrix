# Final Year Project

## Overview
- Involving Matrix Creator, Raspberry Pi and PC
- Repository Contains:
	- [Matrix HAL library](https://github.com/matrix-io/matrix-creator-hal)
	- fftw3 Fast Fourier Transform library header
	- GNU Scientific Library header
	- Google services header

## Requirements
- [Matrix Creator Base](https://github.com/matrix-io/matrix-creator-quickstart/wiki/2.-Getting-Started)
- Visual Studio 2015
- [VC for Linux development plugin](https://blogs.msdn.microsoft.com/vcblog/2016/03/30/visual-c-for-linux-development/)
- Visual Studio plugin for Github
- Linker option `-lpthread -lfftw3f -lsocket++` (Saved in the solution's option)
- Install [FFTW3](http://www.fftw.org/). Need to use `$ ./configure --enable-shared`
- Install FFTW3 dev: `$ sudo apt-get install libfftw3-dev`
- Install [libsocket library](https://github.com/dermesser/libsocket)
- Install [GNU Scientific Library (GSL)](https://github.com/ampl/gsl)
- Compile and install Google service API [Guide](https://github.com/Chen-Zhe/google-speech)

## Notes
- Do not install matrix-creator-malos! It causes [problem](http://community.matrix.one/t/noise-in-sound-recording/212) for recording
