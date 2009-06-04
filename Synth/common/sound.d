module common.sound;

import std.stdio, std.math, common.sound_io;

class Sound
{
	real time;
	real delegate(real) func;

	//Constructs a sound from sampled data
	this(real[] samples)
	{
		time = idx2time(samples.length);
		func = (real t)
		{
			int i = time2idx(t);
			if(i < 0 || i >= samples.length)
				return cast(real)0.;
			return samples[i];
		};
	}

	//Constructs a sound from a wave file
	this(string file)
	{
		auto data = load_wav(file);
		time = idx2time(data.length);
		func = (real t)
		{
			int i = time2idx(t);
			if(i < 0 || i >= data.length)
				return cast(real)0.;
			return cast(real)data[i];
		};
	}
	
	//Constructs a sound from a waveform
	this(real delegate(real) f, real t=real.infinity)
	{
		time = t;
		func = f;
	}

	//Saves the sound
	void save(string file)
	{
		//Need to convert to float
		auto samples = new float[time2idx(time)];
		foreach(int i, inout float s; samples)
			s = func(idx2time(i));
		samples.save_wav(file);
	}
	
	//Samples the sound at time t
	real opCall(real t)
	{
		if(t < 0 || t >= time)
			return 0;
		return func(t);
	}
	
	//Samples the sound
	real[] sample()
	{
		auto res = new real[cast(int)(time * SAMPLE_RATE)];
		foreach(int i, inout real r; res)
			res[i] = opCall(cast(real)i / cast(real)SAMPLE_RATE);
		return res;
	}
	
	//Scales the sound by a scalar
	Sound opMul(real s)
	{
		return new Sound((real t) { return func(t) * s; }, time);
	}
	
	//Computes the pointwise product of two samples
	Sound opMul(Sound other)
	{
		return new Sound(
			(real t) { return opCall(t) * other(t); },
			fmax(time, other.time) );
	}
	
	//Adds two sounds together for mixing
	Sound opAdd(Sound other)
	{
		return new Sound(
		(real t){
			return opCall(t) + other(t);
		}, fmax(time, other.time));
	}
	
	//Concatenates two sounds
	Sound opCat(Sound other)
	{
		return new Sound(
		(real t){
			if(t <= time)
				return func(t);
			return other.func(t - time);
		}, time + other.time);
	}
	
	//Returns a segment of the sound
	Sound opSlice(real start_t, real end_t)
	{
		assert(start_t <= end_t);
		return new Sound((real t) { return func(t - start_t); }, end_t - start_t);
	}
	
	//Does a frequency shift of the sound
	Sound fshift(real f)
	{
		return new Sound((real t) { return func(t * f); }, time / f);
	}
	
	//Does a phase shift of the sound
	Sound pshift(real d)
	{
		return new Sound((real t)
		{
			if(t < d) return cast(real)0.;
			return func(t - d);
		}, time + d);
	}
	
	//Loops the sound n times
	Sound loop(real n)
	{
		return new Sound((real t)
		{
			return func(t - floor(t/time)*time);
		}, time * n);
	}
}
