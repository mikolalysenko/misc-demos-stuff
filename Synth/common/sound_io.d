// Sound input/output module, handles access to wave files
module common.sound_io;

import
	std.stdio,
	std.file,
	std.string;
	
//Sampling rate for all audio samples
invariant uint BITS_PER_SAMPLE	= 32;
invariant uint SAMPLE_RATE		= 44100;

//Converts a time value to an index
int time2idx(real t) { return cast(int)(t * SAMPLE_RATE); }
real idx2time(int i) { return cast(real)i / SAMPLE_RATE; }

//The wave file header
align (1) struct WAVHeader
{
	char[4]	chunk_id;
	uint	chunk_size;
	char[4]	format;
	char[4]	subchunk1_id;
	uint	subchunk1_size;
	ushort	audio_format;
	ushort	num_channels;
	uint	sample_rate;
	uint	byte_rate;
	ushort	block_align;
	ushort	bits_per_sample;
	char[4]	subchunk2_id;
	uint	subchunk2_size;
	
	//Wave header constructor, just fills in data so alignment is correct
	this(const float[] sound)
	{
		chunk_id		= "RIFF";
		format			= "WAVE";
		subchunk1_id	= "fmt ";
		subchunk1_size	= 16;
		audio_format	= 3;
		num_channels	= 1;
		sample_rate		= SAMPLE_RATE;
		byte_rate		= SAMPLE_RATE * BITS_PER_SAMPLE / 8;
		block_align		= BITS_PER_SAMPLE / 8;
		bits_per_sample = BITS_PER_SAMPLE;
		subchunk2_id	= "data";
		subchunk2_size	= sound.length * BITS_PER_SAMPLE / 8;
		
		//Fix final size
		chunk_size		= 20 + subchunk1_size + subchunk2_size;
	}
	
	//Checks the validity of the header
	bool valid()
	{
		return
			chunk_id == "RIFF" &&
			format == "WAVE" &&
			subchunk1_id == "fmt " &&
			subchunk2_id == "data" &&
			chunk_size == 20 + subchunk1_size + subchunk2_size &&
			subchunk1_size == 16 &&
			audio_format == 3 &&
			num_channels == 1 &&
			bits_per_sample == BITS_PER_SAMPLE;
	}
}

/**
 * Saves a wave to a file.
 *
 * Params:
 *	filename = The file we are saving to
 *	
 */
void save_wav(const float[] sound, string filename)
{
	//Create header
	auto header = WAVHeader(sound);
	
	//Write data to file
	std.file.write(filename, 
		(cast(void*)(&header))[0..header.sizeof] ~
		cast(void[])sound);
}

/**
 * Loads a wave file
 */
float[] load_wav(string filename)
{
	auto file = std.file.read(filename);
	auto header = cast(WAVHeader*)file.ptr;

	assert(header.valid);
	
	return (cast(float*)(&file[WAVHeader.sizeof]))[0..header.subchunk2_size * 8 / BITS_PER_SAMPLE];
}

