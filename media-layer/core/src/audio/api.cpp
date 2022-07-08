#include <vector>
#include <cmath>

#include <AL/al.h>

#include <media-layer/audio.h>
#include <libreborn/libreborn.h>

#include "file.h"
#include "engine.h"
#include "api.h"

// Store Audio Sources
static std::vector<ALuint> &get_sources() {
    static std::vector<ALuint> sources;
    return sources;
}

// Store Idle Audio Sources
#define MAX_IDLE_SOURCES 50
static std::vector<ALuint> &get_idle_sources() {
    static std::vector<ALuint> sources;
    return sources;
}

// Error Checking
#define AL_ERROR_CHECK() AL_ERROR_CHECK_MANUAL(alGetError())
#define AL_ERROR_CHECK_MANUAL(val) \
    { \
        ALenum __err = val; \
        if (__err != AL_NO_ERROR) { \
            ERR("OpenAL Error: %s", alGetString(__err)); \
        } \
    }

// Delete Sources
void _media_audio_delete_sources() {
    if (_media_audio_is_loaded()) {
        for (ALuint source : get_idle_sources()) {
            alDeleteSources(1, &source);
            AL_ERROR_CHECK();
        }
        for (ALuint source : get_sources()) {
            alDeleteSources(1, &source);
            AL_ERROR_CHECK();
        }
    }
    get_idle_sources().clear();
    get_sources().clear();
}

// Update Listener
void media_audio_update(float volume, float x, float y, float z, float yaw) {
    // Check
    if (_media_audio_is_loaded()) {
        // Update Listener Volume
        alListenerf(AL_GAIN, volume);
        AL_ERROR_CHECK();

        // Update Listener Position
        alListener3f(AL_POSITION, x, y, z);
        AL_ERROR_CHECK();

        // Update Listener Orientation
        float radian_yaw = yaw * (M_PI / 180);
        ALfloat orientation[] = {-sinf(radian_yaw), 0.0f, cosf(radian_yaw), 0.0f, 1.0f, 0.0f};
        alListenerfv(AL_ORIENTATION, orientation);
        AL_ERROR_CHECK();

        // Clear Finished Sources
        std::vector<ALuint>::iterator it = get_sources().begin();
        while (it != get_sources().end()) {
            ALuint source = *it;
            bool remove = false;
            // Check
            if (source && alIsSource(source)) {
                // Is Valid Source
                ALint source_state;
                alGetSourcei(source, AL_SOURCE_STATE, &source_state);
                AL_ERROR_CHECK();
                if (source_state != AL_PLAYING) {
                    // Finished Playing
                    remove = true;
                    if (get_idle_sources().size() < MAX_IDLE_SOURCES) {
                        get_idle_sources().push_back(source);
                    } else {
                        alDeleteSources(1, &source);
                        AL_ERROR_CHECK();
                    }
                }
            } else {
                // Not A Source
                remove = true;
            }
            // Remove If Needed
            if (remove) {
                it = get_sources().erase(it);
            } else {
                ++it;
            }
        }
    }
}

// Play
void media_audio_play(const char *source, const char *name, float x, float y, float z, float pitch, float volume, int is_ui) {
    // Check
    if (_media_audio_is_loaded()) {
        // Load Sound
        ALuint buffer = _media_audio_get_buffer(source, name);
        if (volume > 0.0f && buffer) {
            // Get Source
            ALuint al_source;
            if (get_idle_sources().size() > 0) {
                // Use Idle Source
                al_source = get_idle_sources().back();
                get_idle_sources().pop_back();
            } else {
                // Create Source
                alGenSources(1, &al_source);
                // Special Out-Of-Memory Handling
                {
                    ALenum err = alGetError();
                    if (err == AL_OUT_OF_MEMORY) {
                        return;
                    } else {
                        AL_ERROR_CHECK_MANUAL(err);
                    }
                }
            }

            // Set Properties
            alSourcef(al_source, AL_PITCH, pitch);
            AL_ERROR_CHECK();
            alSourcef(al_source, AL_GAIN, volume);
            AL_ERROR_CHECK();
            alSource3f(al_source, AL_POSITION, x, y, z);
            AL_ERROR_CHECK();
            alSource3f(al_source, AL_VELOCITY, 0, 0, 0);
            AL_ERROR_CHECK();
            alSourcei(al_source, AL_LOOPING, AL_FALSE);
            AL_ERROR_CHECK();
            alSourcei(al_source, AL_SOURCE_RELATIVE, is_ui ? AL_TRUE : AL_FALSE);
            AL_ERROR_CHECK();

            // Set Attenuation
            alSourcei(al_source, AL_DISTANCE_MODEL, AL_LINEAR_DISTANCE);
            AL_ERROR_CHECK();
            alSourcef(al_source, AL_MAX_DISTANCE, 16.0f);
            AL_ERROR_CHECK();
            alSourcef(al_source, AL_ROLLOFF_FACTOR, 1.0f);
            AL_ERROR_CHECK();
            alSourcef(al_source, AL_REFERENCE_DISTANCE, 0.0f);
            AL_ERROR_CHECK();

            // Set Buffer
            alSourcei(al_source, AL_BUFFER, buffer);
            AL_ERROR_CHECK();

            // Play
            alSourcePlay(al_source);
            AL_ERROR_CHECK();
            get_sources().push_back(al_source);
        }
    }
}
