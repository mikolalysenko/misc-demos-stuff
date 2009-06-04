module common.windows;

import std.math, common.sound;

//Applies a window to a sound
Sound window(Sound sound, real function(real) wind)
{
	return new Sound(
		(real t)
		{
			return sound.func(t) * wind(t / sound.time); 
		}, sound.time);
}

//Not really a window at all
real rect(real t)
{
	return 1.;
}

//High range filters
real hamming(real t)
{
	return 0.54 - 0.46 * cos(2 * PI * t);
}

real hann(real t)
{
	return 0.5 * (1. - cos(2 * PI * t));
}

//Mid range filter
real gauss(real t)
{
	t -= 0.5;
	return exp(-5. * t * t);
}


//Low range filters
real nuttall(real t)
{
	return 0.355768 
		- 0.487396 * cos(2. * PI * t)
		+ 0.144232 * cos(4. * PI * t)
		- 0.012604 * cos(6. * PI * t);
}

real blackman_nuttall(real t)
{
	return 0.3635819
		- 0.4891775 * cos(2. * PI * t)
		+ 0.1365995 * cos(4. * PI * t)
		- 0.0106411 * cos(6. * PI * t);
}

//wtf filter
real flattop(real t)
{
	return 1.
		- 1.93 * cos(2. * PI * t)
		+ 1.29 * cos(4. * PI * t)
		- 0.388 * cos(6. * PI * t)
		+ 0.032 * cos(8. * PI * t);
}

Sound attack_release(real attack, real release)
{
	return new Sound((real t)
	{
		return attack * max(cast(real)0., cast(real)1. - t / release);
	}, release);
}


