module common.effects;

import common.fourier, common.sound, common.sound_io;

Sound reverb(Sound s, real delay)
{
	return s + s.pshift(delay);
}

Sound smooth(Sound s, real sigma)
{
	return new Sound((real t)
	{
		real x = 0., d = 0.;
		for(real v=t-sigma; v<=t+sigma; v+=1./SAMPLE_RATE)
			d += 1., x += s(v);
		return x / d;
	}, s.time);
}

