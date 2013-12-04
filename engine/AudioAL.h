/************************************************************************
* OpenAL Audio Driver
* Version:	1.0
* By:		James Ogden
*/

#pragma once

#include <stdint.h>
#include <cstdio>
#include <string>
#include <vector>

typedef uint32_t	sound_t;
typedef uint32_t	voice_t;

#define SOUND_ID_INVALID		0xFFFFFFFF

enum {
	AUDIO_CAMERA_POSITION,
	AUDIO_CAMERA_ORIENTATION
};

class AudioAL
{
public:

	AudioAL( );
	~AudioAL( );

	/* Initialise the Audio Driver */
	bool init( uint8_t p_voices = 16 );

	/* Shutdown the Audio Driver */
	void shutdown( );

	/* Pause the audio */
	bool pause();

	/* Resume the audio */
	bool resume();

	/* Stop the audio */
	bool stop();

	/* Set camera position */
	void setCameraPosition( float p_x, float p_y, float p_z );

	/* Set camera orientation */
	void setCameraOrientation( float p_alpha, float p_beta );

	/* Process an audio stream, wait for the buffer to complete playing */
	bool process( );

	/* Get the name of this Audio */
	const char* getName();

	/* Load a sound raw PCM or MS-WAVE */
	sound_t loadSound( const char* p_file );
	sound_t loadSound( void* p_data, size_t p_length, uint32_t p_rate = 22050, uint32_t p_channels = 2, uint32_t p_bits = 16 );

	/* Free the resources for the desired sound */
	void freeSound( sound_t p_snd );

	/* Free the resources associated with the loaded sounds */
	void freeSounds( );

	/* Add a voice to the world */
	bool addVoice( sound_t p_snd, float x = 0.0f, float y = 0.0f, float z = 0.0f, float p_volume = 1.0f );

	/* Set the looped sound and play it (For Music/Ambient) */
	bool startLoopedVoice( sound_t p_snd );

	/* Stop the looped sound */
	bool stopLoopedVoice( );

private:

	bool					m_ready;
	float					m_position[3];
	sound_t					m_looped_sound;
	uint32_t				m_looped_voice;
	uint8_t					m_numvoices;	// Software AL supports 256 voices only it seems
	FILE					*m_stream;
	std::vector<sound_t>	m_sounds;
	std::vector<voice_t>	m_voices;
	std::vector<std::string>m_extensions;
};
