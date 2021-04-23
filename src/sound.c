#include "sound.h"
#include "logging.h"
#include "omni_exit.h"
#include "constants.h"
#include "SDL2/SDL_mixer.h"
#include <stdlib.h>
#include <stddef.h>


static int musicVolume = 10;
static int soundVolume = 10;
//SDL_mixer has callback things we could use, but then we'd need to give each sound a callback and blah blah
static Sound *lastPlayedSounds[NUM_SOUND_CHANNELS];


/////////////////////////////////////////////////
// Library setup/teardown
/////////////////////////////////////////////////
void initSound(){
    //Mix_Init(...) will load the dynamic libraries specified by the flags in its argument
    //however, these will be loaded anyway if you try to load something
    //since we're only loading wav files, there's no need to check that we can support other file types
    
    //some mucking with parameters will be needed, see LazyFoo and the documentation for ideas
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
        LOG_ERR("SDL_Mixer could not be started: %s", Mix_GetError());
        displayErrorAndExit("Unable to setup audio");
    }
    
    //setup stuff for all the different sounds we can play
    int i;
    for (i = 0; i < NUM_SOUND_CHANNELS; i++){
        lastPlayedSounds[i] = NULL;
    }
    Mix_AllocateChannels(NUM_SOUND_CHANNELS); //from the documentation - this call never fails, only segfaults
}

void termSound(){
    //only need to call Mix_Quit() if we call Mix_Init(...), see initSound()
    
    //this should be called as many times as Mix_OpenAudio(...) is called; in theory just once
    Mix_CloseAudio();
}


/////////////////////////////////////////////////
// Init
/////////////////////////////////////////////////
Music *init_Music(Music *self){
    self->music = NULL;
    self->volumeAdjust = 0;    

    return self;
}

Sound *init_Sound(Sound *self){
    self->chunk = NULL;
    self->volumeAdjust = 0;
    self->channel = -1;

    return self;
}


/////////////////////////////////////////////////
// Free
/////////////////////////////////////////////////
void free_Music(Music *self){
    if (self == NULL){
        LOG_WAR("Received null pointer");
        return;
    }
    
    Mix_FreeMusic(self->music);
    self->music = NULL;
    self->volumeAdjust = 0;
    
    free(self);
}

void free_Sound(Sound *self){
    if (self == NULL){
        LOG_WAR("Received null pointer");
        return;
    }
    
    Mix_FreeChunk(self->chunk);
    self->chunk = NULL;
    self->volumeAdjust = 0;
    self->channel = -1;
    
    free(self);
}


/////////////////////////////////////////////////
// Loading
/////////////////////////////////////////////////
void fillMusicFromFile(char *filename, Music *self){
    //check for leaks, but load even if there is one
    if (self->music != NULL){
        LOG_WAR("Leaking the music from %p", self);
    }
    
    self->music = Mix_LoadMUS(filename);
    if (self->music == NULL){
        LOG_ERR("Music in %s could not be loaded: %s", filename, Mix_GetError());
        displayErrorAndExit("Failed to read file %s", filename);
    }
}

void fillSoundFromFile(char *filename, Sound *self){
    //check for leaks, but load even if there is one
    if (self->chunk != NULL){
        LOG_WAR("Leaking the sound from %p", self);
    }
    
    self->chunk = Mix_LoadWAV(filename);
    if (self->chunk == NULL){
        LOG_ERR("Sound in %s could not be loaded: %s", filename, Mix_GetError());
        displayErrorAndExit("Failed to read file %s", filename);
    }
}


/////////////////////////////////////////////////
// Music
/////////////////////////////////////////////////
void playMusic(Music *music){
    Mix_VolumeMusic(musicVolume * 12 + music->volumeAdjust);
    
    if (Mix_PlayMusic(music->music, -1) == -1) {
        LOG_ERR("Failed to play music %p: %s", music, Mix_GetError());
        displayErrorAndExit("Error playing music");
    }
}

void stopMusic(){
    Mix_HaltMusic();
}

void fadeInMusic(Music *music, int fadeDuration){
    int success;

    //NOTE: if you try to fade in while something is fading in, the volume just goes to max
    if (Mix_FadingMusic() == MIX_FADING_IN){
        LOG_WAR("Trying to fade in %p while something else is fading in", music);
    }
    
    //first set the volume
    Mix_VolumeMusic(musicVolume * 12 + music->volumeAdjust);
    if (Mix_FadeInMusic(music->music, -1, fadeDuration) == -1){
        LOG_ERR("Failed to fade in music %p: %s", music, Mix_GetError());
        displayErrorAndExit("Error fading music");
    }
}

void fadeOutMusic(int fadeDuration){
    if (Mix_FadeOutMusic(fadeDuration) == 0){ //yo why doesn't this return match the others?
        LOG_ERR("Failed to fade out music: %s", Mix_GetError());
        displayErrorAndExit("Error fading music");
    }
}

int musicIsPlaying(){
    return Mix_PlayingMusic(); //0 if not, 1 if yes
}

int musicIsFadingIn(){
    return (Mix_FadingMusic() == MIX_FADING_IN);
}

int musicIsFadingOut(){
    return (Mix_FadingMusic() == MIX_FADING_OUT);
}

void increaseMusicVolume(){
    if (musicVolume < 10){
        musicVolume++;
    }
}

void decreaseMusicVolume(){
    if (musicVolume > 0){
        musicVolume--;
    }
}


/////////////////////////////////////////////////
// Sounds
/////////////////////////////////////////////////
void playSound(Sound *sound){
    sound->channel = Mix_PlayChannel(-1, sound->chunk, 0);

    if (sound->channel == -1){
        LOG_ERR("Failed to play sound %p: %s", sound, Mix_GetError());
        displayErrorAndExit("Error playing sound");
    }
    
    Mix_Volume(sound->channel, soundVolume * 12 + sound->volumeAdjust);
    lastPlayedSounds[sound->channel] = sound;
}

void repeatSound(Sound *sound, int numRepeats){
    sound->channel = Mix_PlayChannel(-1, sound->chunk, numRepeats);
    if (sound->channel == -1){
        LOG_ERR("Failed to play sound %p: %s", sound, Mix_GetError());
        displayErrorAndExit("Error playing sound");
    }
    
    Mix_Volume(sound->channel, soundVolume * 12 + sound->volumeAdjust);
    lastPlayedSounds[sound->channel] = sound;
}

void stopSound(Sound *sound){
    if (lastPlayedSounds[sound->channel] == sound){
        Mix_HaltChannel(sound->channel); //always returns 0?
    } else {
        LOG_WAR("The sound %p was not playing", sound);
    }
}

void stopAllSound(){
    int i;
    for (i = 0; i < NUM_SOUND_CHANNELS; i++){
        Mix_HaltChannel(i); //always returns 0?
    }
}

void increaseSoundVolume(){
    if (soundVolume < 10){
        soundVolume++;
    }
}

void decreaseSoundVolume(){
    if (soundVolume > 0){
        soundVolume--;
    }
}

