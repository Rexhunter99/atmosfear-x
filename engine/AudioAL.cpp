
#include "AudioAL.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <string.h>

#include <cmath>
#include <cstdlib>

using namespace std;

#define MAX_SOUND_DISTANCE		68 * 256
#define HEADER_ID_PCM			( ('P'<<8) | ('C'<<16) | ('M'<<24) )
#define HEADER_ID_WAVE			( ('R') | ('I'<<8) | ('F'<<16) | ('F'<<24) )
#define V_AUDIO_MAJOR			1
#define V_AUDIO_MINOR			8


ALCdevice				*g_ALDevice			= NULL;
ALCcontext				*g_ALContext		= NULL;


const char* alGetErrorString( ALenum e )
{
	switch ( e )
	{
		case AL_NO_ERROR:			return "No error";
		case AL_INVALID_NAME:		return "Invalid Name paramater passed to AL call";
		case AL_INVALID_ENUM:		return "Invalid parameter passed to AL call";
		case AL_INVALID_VALUE:		return "Invalid enum parameter value";
		case AL_INVALID_OPERATION:	return "Invalid operation";
		case AL_OUT_OF_MEMORY:		return "Out of memory";
	}

	return "Unknown error";
}

bool alCheckError( FILE* fp, const char* func = " " )
{
	ALenum err = alGetError();

	if ( err == AL_NO_ERROR )
	return false;

	const char* str = alGetErrorString( err );
	fprintf( fp, "AL Error in %s: %s\n", func, str );

	return true;
}



AudioAL::AudioAL()
{
	m_stream = fopen( "AUDIO.LOG", "w" );
	fprintf( m_stream, "================================================\n" );
	fprintf( m_stream, "\tAtmosFEAR 2 - Audio Log\n" );
	fprintf( m_stream, "\tVersion: %u.%u\n", V_AUDIO_MAJOR, V_AUDIO_MINOR );
	fprintf( m_stream, "\tDate: %s %s\n", __TIME__, __DATE__ );
	fprintf( m_stream, "================================================\n\n" );

	m_looped_sound = SOUND_ID_INVALID;
	m_position[0] = 0.0f;
	m_position[1] = 0.0f;
	m_position[2] = 0.0f;
}

AudioAL::~AudioAL()
{
	fclose( m_stream );
}


bool AudioAL::init( uint8_t p_voices )
{
	m_ready = false;

	fprintf( m_stream, "InitAudioSystem( \"OpenAL\" )\n{\n" );

	const char* devices = alcGetString( NULL, ALC_DEVICE_SPECIFIER );

	fprintf( m_stream, "\tDevices: %s\n", devices );

	// -- Open the playback device
	fprintf( m_stream, "\tOpening default playback device... " );
	if ( !(g_ALDevice = alcOpenDevice( NULL )) )
		fprintf( m_stream, "Fail!\n" );
	else
		fprintf( m_stream, "Ok!\n" );

	//alcCaptureOpenDevice( NULL, 22050, AL_FORMAT_MONO8, 1024 );
	//alcCaptureCloseDevice()

	// -- Create Audio Context
	g_ALContext = alcCreateContext( g_ALDevice, NULL );
	if ( g_ALContext )
	{
		alcMakeContextCurrent( g_ALContext );
		alcProcessContext( g_ALContext );

		// -- Grab all the extensions
		m_extensions.clear();
		const char* drv_exts = alGetString( AL_EXTENSIONS );
		/* Valgrind doesn't like this (read/writes of size 1)
		if ( drv_exts != nullptr || strlen( drv_exts ) != 0 )
		{
			char* exts = (char*)malloc( strlen(drv_exts) );
			memcpy( exts, drv_exts, strlen(drv_exts)+1 );

			char* tok = strtok( exts, " " );

			while ( tok )
			{
				m_extensions.push_back( tok );
				tok = strtok( NULL, " " );
			}

			free(exts);
		}*/
	}
	else
	{
		fprintf( m_stream, "\tFailed to create the playback context\n" );
	}

	// -- AL Version
	fprintf( m_stream, "\tOpenAL Version:\t\t%s\n", alGetString( AL_VERSION ) );
	// -- AL Vender
	fprintf( m_stream, "\tOpenAL Vendor:\t\t%s\n", alGetString( AL_VENDOR ) );
	// -- AL Renderer
	fprintf( m_stream, "\tOpenAL Processor:\t%s\n", alGetString( AL_RENDERER ) );
	// -- AL EFX Version
	int32_t efx_version[2];
	alcGetIntegerv(g_ALDevice, ALC_EFX_MAJOR_VERSION, 1, &efx_version[0]);
	alcGetIntegerv(g_ALDevice, ALC_EFX_MINOR_VERSION, 1, &efx_version[1]);
	fprintf( m_stream, "\tOpenAL EFX Version:\t\t%d.%d\n", efx_version[0], efx_version[1] );
	// -- AL Extensions
	fprintf( m_stream, "\tOpenAL Extensions\n" );
	for ( int i=0; i<m_extensions.size(); i++ )
	{
		fprintf( m_stream, "\t\t%s\n", m_extensions[i].c_str() );
	}


	// -- Generate Voice Sources
	fprintf( m_stream, "\tCreating %u Voices...", p_voices );

	m_numvoices = p_voices;
	m_voices.clear();
	for ( int i=0; i<m_numvoices; i++ )
	{
		m_voices.push_back( voice_t() );
		alGenSources( 1, &m_voices[i] );
		alCheckError( m_stream );
	}

	alGenSources( 1, &m_looped_voice );
	alCheckError( m_stream );

	fprintf( m_stream, " Ok!\n" );

	alDistanceModel( AL_INVERSE_DISTANCE );

	fprintf( m_stream, "\tSpeed of Sound:\t %d units\n", alGetInteger( AL_SPEED_OF_SOUND ) );

	//while ( alGetError != AL_NO_ERROR );

	fprintf( m_stream, "}\n\n" );
	fprintf( m_stream, "GameLoop()\n{\n" );

	m_ready = true;

	return true;
}

void AudioAL::shutdown()
{
	if ( !m_ready ) return;

	fprintf( m_stream, "}\n\n" );

	fflush( m_stream );
	fprintf( m_stream, "ShutdownAudioSystem()\n{\n" );

	fprintf( m_stream, "\tReleasing AL voices: " );

	alSourceStop( m_looped_voice );
	alDeleteSources( 1, &m_looped_voice );

	for ( int i=0; i<m_voices.size(); i++ )
	{
		alSourceStop( m_voices[i] );
		alDeleteSources( 1, &m_voices[i] );
	}
	m_voices.clear();

	fprintf( m_stream, "Ok\n" );

	this->freeSounds();

	fprintf( m_stream, "\tShutdown audio device: " );

	alcMakeContextCurrent( NULL );
	alcDestroyContext( g_ALContext );

	alcCloseDevice( g_ALDevice );

	fprintf( m_stream, "Ok\n" );
	fprintf( m_stream, "}\n" );

	m_ready = false;

	return;
}

bool AudioAL::stop( )
{
	if ( !m_ready ) return false;

	fprintf( m_stream, "\tStopping AL sounds: " );

	for ( int i=0; i<m_voices.size(); i++ )
	{
		alSourceStop( m_voices[i] );
	}

	alSourceStop( m_looped_voice );

	fprintf( m_stream, "Ok\n" );

	return true;
}

bool AudioAL::process( )
{
	if ( !m_ready ) return false;

	return true;
}

const char* AudioAL::getName()
{
	return "OpenAL 1.1";
}

void AudioAL::setCameraOrientation( float p_alpha, float p_beta )
{
	if ( !m_ready ) return;

	float dir[] = {
		sinf(3.14f-p_alpha)*cosf(p_beta),
		0.0f,
		cosf(3.14f-p_alpha)*cosf(p_beta),
		0.0f, 1.0f, 0.0f
	};
	alListenerfv( AL_ORIENTATION, dir );
}

void AudioAL::setCameraPosition( float p_x, float p_y, float p_z )
{
	if ( !m_ready ) return;

	m_position[0] = p_x;
	m_position[1] = p_y;
	m_position[2] = p_z;
	alListener3f( AL_POSITION, p_x, p_y, p_z );
}

bool AudioAL::addVoice( sound_t p_snd, float x, float y, float z, float p_volume )
{
	if ( !m_ready ) return false;
	if ( p_snd == SOUND_ID_INVALID ) return false;
	// -- If Volume is inaudible, quit
	if ( p_volume < 0.001f ) return false;
	if ( p_volume > 1.0f ) p_volume = 1.0f;

	#if defined( AF_DEBUG )
	if ( !alIsBuffer( m_sounds[p_snd] ) )
	{
		fprintf( m_stream, "\taddVoice( %u ) -> ALBuffer is invalid!\n", p_snd );
		return false;
	}
	#endif

	ALuint src = 0;

	for ( int i=0; i<m_voices.size(); i++ )
	{
		ALint	state = AL_STOPPED;
		alGetSourcei( m_voices[i], AL_SOURCE_STATE, &state );
		if ( state == AL_PLAYING )
		{
			//fprintf( m_stream, "\taddVoice( ) -> Voice[%d] is in use!\n", i );
			continue;
		}
		src = m_voices[i];

		alSourceStop( src );
		alSource3f( src, AL_POSITION, x, y, z );
		alSourcei( src, AL_BUFFER, m_sounds[p_snd] );
		alSourcef( src, AL_GAIN, p_volume );
		alSourcef( src, AL_PITCH, 1.0f );		// Modify this slightly over or under 1.0f and we have some "unique" sound effects
		alSourcef( src, AL_REFERENCE_DISTANCE, 256.0f * 4.0f ); // Change the 4 higher to make the sound heard over longer distances before it fades
		alSourcef( src, AL_MAX_DISTANCE, MAX_SOUND_DISTANCE ); // 256 == 1 meter, 68 == "standard" view distance
		alSourcePlay( src );
		return true;
	}

#if defined( AF_DEBUG )
	fprintf( m_stream, "\tDEBUG -> Ran out of available voices.\n" );
#endif

	return false;
}

sound_t AudioAL::loadSound( const char* p_file )
{
	if ( !m_ready ) return SOUND_ID_INVALID;
	if ( !p_file ) return SOUND_ID_INVALID;

	sound_t p_snd = 0;
	FILE* fp = fopen( p_file, "rb" );

	if ( !fp )
	{
		fprintf( m_stream, "\tFailed to open the audio file \"%s\" for reading!\n", p_file );
		return SOUND_ID_INVALID;
	}

	uint32_t tag = 0;
	fread( &tag, 4, 1, fp );

	if ( tag == HEADER_ID_WAVE )
	{
		fseek( fp, 4, SEEK_CUR ); // -- Skip the container size
		fseek( fp, 12, SEEK_CUR ); // -- Skip the WAVE Format Header

		uint16_t type = 0;
		uint16_t channels = 0;
		uint32_t samplerate = 0;
		uint32_t avgbytespersec = 0;
		uint16_t blockalign = 0;
		uint16_t bitdepth = 0;
		uint32_t pcm_length = 0;

		fread( &type, 2, 1, fp );
		fread( &channels, 2, 1, fp );
		fread( &samplerate, 4, 1, fp );
		fread( &avgbytespersec, 4, 1, fp );
		fread( &blockalign, 2, 1, fp );
		fread( &bitdepth, 2, 1, fp );

		fseek( fp, 4, SEEK_CUR ); // -- Skip data tag

		fread( &pcm_length, 4, 1, fp );

		char* pcm_data = new char [ pcm_length ];

		fread( pcm_data, pcm_length, 1, fp );

		if ( type == 1 )
		{
			ALenum format = 0;
			if ( channels == 1 && bitdepth == 8 ) format = AL_FORMAT_MONO8;
			if ( channels == 1 && bitdepth == 16 ) format = AL_FORMAT_MONO16;
			if ( channels == 2 && bitdepth == 8 ) format = AL_FORMAT_STEREO8;
			if ( channels == 2 && bitdepth == 16 ) format = AL_FORMAT_STEREO16;

			ALuint buffer = 0;

			// -- Get the index
			p_snd = m_sounds.size();

			// -- Generate the resource
			alGenBuffers( 1, &buffer );
			alCheckError( m_stream, "loadSound()::alGenBuffers" );
			this->m_sounds.push_back( buffer );

			alBufferData( buffer, format, pcm_data, pcm_length, samplerate );
			alCheckError( m_stream, "loadSound()::alBufferData" );
		}
		else
		{
			fprintf( m_stream, "\tThe specified WAVE is not a raw PCM!\n" );
		}

		delete [] pcm_data;
	}
	else if ( tag == HEADER_ID_PCM )
	{
		p_snd = SOUND_ID_INVALID;
		fprintf( m_stream, "\tPCM Files not supported.\n" );
	}

	fclose( fp );

	return p_snd;
}

sound_t AudioAL::loadSound( void* p_data, size_t p_length, uint32_t p_rate, uint32_t p_channels, uint32_t p_bits )
{
	if ( !m_ready || !p_data || !p_length ) return SOUND_ID_INVALID;

	sound_t p_snd = SOUND_ID_INVALID;

	ALenum format = 0;

	if ( p_channels == 1 && p_bits == 8 ) format = AL_FORMAT_MONO8;
	if ( p_channels == 1 && p_bits == 16 ) format = AL_FORMAT_MONO16;
	if ( p_channels == 2 && p_bits == 8 ) format = AL_FORMAT_STEREO8;
	if ( p_channels == 2 && p_bits == 16 ) format = AL_FORMAT_STEREO16;

	ALuint buffer = 0;

	// -- Get the index
	p_snd = m_sounds.size();

	// -- Generate the resource
	alGenBuffers( 1, &buffer );
	alCheckError( m_stream, "loadSound()::alGenBuffers" );
	this->m_sounds.push_back( buffer );

	alBufferData( buffer, format, p_data, p_length, p_rate );
	alCheckError( m_stream, "loadSound()::alBufferData" );

	return p_snd;
}

void AudioAL::freeSounds( )
{
	if ( !m_ready ) return;

	fprintf( m_stream, "\tReleasing AL sounds: " );

	for ( int i=0; i<m_sounds.size(); i++ )
	{
		alDeleteBuffers( 1, &m_sounds[i] );
		ALenum e = alGetError();
		if ( e != AL_NO_ERROR )
		{
			fprintf( m_stream, "\tFailed to delete buffer %d\n\t%s\n", i, alGetErrorString( e ) );
		}
	}
	m_sounds.clear();

	fprintf( m_stream, "Ok\n" );
}

void AudioAL::freeSound( sound_t p_snd )
{
	if ( !m_ready ) return;
	if ( p_snd = SOUND_ID_INVALID ) return;

	vector<sound_t>::iterator i=m_sounds.begin() + p_snd;

	ALuint buffer = *i;
	alDeleteBuffers( 1, &buffer );

	ALenum e = alGetError();
	if ( e != AL_NO_ERROR )
	{
		fprintf( m_stream, "\tFailed to delete buffer %d\n\t%s\n", i, alGetErrorString( e ) );
	}

	m_sounds.erase( i );
}

bool AudioAL::startLoopedVoice( sound_t p_snd )
{
	if ( !m_ready ) return false;
	if ( m_looped_sound == p_snd ) return true;

	#if defined( AF_DEBUG )
	if ( !alIsBuffer( m_sounds[p_snd] ) )
	{
		fprintf( m_stream, "\tstartLoopedVoice( %u ) -> ALBuffer is invalid!\n", p_snd );
		return false;
	}
	#endif

	alSource3f( m_looped_voice, AL_POSITION, m_position[0], m_position[1], m_position[0] );
	alSourcei( m_looped_voice, AL_BUFFER, m_sounds[p_snd] );
	alSourcef( m_looped_voice, AL_GAIN, 1.0f );
	alSourcei( m_looped_voice, AL_LOOPING, AL_TRUE );
	alSourcePlay( m_looped_voice );
	m_looped_sound = p_snd;

	return true;
}

bool AudioAL::stopLoopedVoice( )
{
	if ( !m_ready ) return false;

	alSourceStop( m_looped_voice );

	return true;
}
