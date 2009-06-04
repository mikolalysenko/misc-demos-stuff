module common.fourier;

import std.stdio, common.sound, common.sound_io, common.windows;

//FFTW3 externs
typedef void* fftw_plan;
extern(C) double* fftw_malloc(size_t);
extern(C) void fftw_free(double*);
extern(C) void fftw_execute(fftw_plan);
extern(C) fftw_plan fftw_plan_dft_1d(size_t, double*, double*, int, int);
extern(C) void fftw_destroy_plan(fftw_plan);

//Performs an FFT about the given sound
creal[] fft(Sound sound)
{
	auto samples = sound.sample;
	
	//Allocate temp arrays
	auto input = fftw_malloc(samples.length * cdouble.sizeof),
		 output = fftw_malloc(samples.length * cdouble.sizeof);

	//Allocate plan
	auto plan = fftw_plan_dft_1d(samples.length, input, output, -1, (1<<6));

	//Construct data
	for(int i=0; i<samples.length; i++)
	{
		input[i*2] = cast(double)(samples[i]);
		input[i*2+1] = 0.;
	}

	fftw_execute(plan);
	
	//Extract result
	auto result = new creal[samples.length];
	for(int i=0; i<samples.length; i++)
	{
		result[i] = cast(creal)(output[i*2] + output[i*2+1]*1i);
	}
	
	//Clean up
	fftw_destroy_plan(plan);
	fftw_free(input);
	fftw_free(output);

	//Done
	return result;
}


//Does an inverse fft
Sound ifft(creal[] data)
{
	//Allocate temp arrays
	auto input = fftw_malloc(data.length * cdouble.sizeof),
		 output = fftw_malloc(data.length * cdouble.sizeof);

	//Allocate plan
	auto plan = fftw_plan_dft_1d(data.length, input, output, -1, (1<<6));

	//Construct data
	for(int i=0; i<data.length; i++)
	{
		input[i*2] = cast(double)(data[i].re);
		input[i*2+1] = cast(double)(data[i].im);
	}

	fftw_execute(plan);
	
	//Extract result
	auto result = new real[data.length];
	for(int i=0; i<data.length; i++)
	{
		result[i] = cast(real)(output[i*2].re);
	}
	
	//Clean up
	fftw_destroy_plan(plan);
	fftw_free(input);
	fftw_free(output);

	//Done
	return new Sound(result);
}
