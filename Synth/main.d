import
	std.math,
	std.stdio,
	std.random,
	common.sound,
	common.waveforms,
	common.windows,
	common.fourier,
	common.effects;



void main()
{
	auto test = square(150.)[0..1];
	
	test.save("test.wav");
}
