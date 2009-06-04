//Various common waveforms
module common.waveforms;

import std.math, std.random, common.sound;

/**
 * 0 sound
 */
Sound silence(real dur = real.infinity)
{
	return new Sound(
		(real t){ return cast(real)0.; },
		dur);
}

/**
 * Constant sound
 */
Sound constant(real val, real dur = real.infinity)
{
	return new Sound((real t){ return val; }, dur);
}

/**
 * Generates white noise
 */
Sound noise(real dur = real.infinity)
{
	Mt19937 rng;
	rng.seed(unpredictableSeed);
	return new Sound((real t){ return cast(real)uniform(-1., 1., rng); }, dur);
}

/**
 * Generates a sine wave with the specified frequency (in Hz)
 *
 * Params:
 *	freq = Frequency of wave (default 1Hz)
 */
Sound sine(real freq = 1., real dur = real.infinity)
{
	return new Sound((real t){ return sin(freq * t * 2. * PI); }, dur);
}

/**
 * Generates a sawtooth wave with the specified frequency (in Hz)
 *
 * Params:
 *	freq = Frequency of wave (default 1Hz)
 */
Sound sawtooth(real freq = 1., real dur = real.infinity)
{
	return new Sound((real t){ t*=freq; return cast(real)(2. * (t - floor(t)) - 1.); }, dur);
}

/**
 * Generates a triangle wave with the specified frequency (in Hz)
 *
 * Params:
 *	freq = Frequency of wave (default 1Hz)
 */
Sound triangle(real freq = 1., real dur=real.infinity)
{
	return new Sound(
	(real t)
	{
		t *= freq;
		int x = cast(int)t;
		return 1. - 2. * (t - x);
	}, dur);
}

/**
 * Generates a square wave with the specified frequency (in Hz)
 *
 * Params:
 *	freq = Frequency of wave (default 1Hz)
 */
Sound square(real freq = 1., real dur=real.infinity)
{
	auto saw = sawtooth(freq);
	return new Sound((real t) {
		t *= freq;
		int x = cast(int)t;
		return (x - t >= 0.5 ? -1. : 1.);
	}, dur);
}

/**
 * FM Synthesis.  Useful for creating cool effects.  Shifts the
 * carrier waveform in frequency by the modulator waveform + base index
 *
 * Params:
 *	carrier = The carrier waveform, (usually frequency 1HZ)
 *	modulator = The modulating waveform, shifts frequency
 *	base = The base wave form (optional argument, used to adjust frequency)
 */
Sound fm_synth(
	Sound carrier, 
	Sound modulator,
	real carrier_freq,
	real dur = real.infinity)
{
	return new Sound((real t) { return carrier(t * carrier_freq + modulator(t)); }, dur);
}
