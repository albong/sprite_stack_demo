#ifndef SOUND_H
#define SOUND_H

#include "SDL2/SDL_mixer.h"

/*
 * Our abstraction for sound, sitting on top of SDL_mixer.
 * 
 * I did test that all of this works, but because I don't personally use sound all that much,
 * almost none of the demo stuff have sound effects.  As a result, it may be that more work is
 * required here.
 */
 

//volumes should probably be stored in the sound objects and loaded from
//a datafile, to enable use to tweak levels without having to recompile

//WAV files only, MP3 has gaps at the beginnings and ends, no good for loops
//MP3 not always supported on all platforms either?

/*
 * VOLUME:
 * Overall, the volume can go up to 128, so to have a 10 notch slider in the menu,
 * we'll think of volume in ten chunks of size 12, going up to 120.
 * Music/sounds can specify a volume adjustment up to +-6.  Suppose the sound in the menu
 * is set to 5, and we have sound that has an adjustment of +3.  Then its SDL volume is
 * 12*5+3.  This gives us a way to do some balancing of audio without remixing, while
 * still having a linear scale for volume from the menu.  It also prevents us from
 * unintentionally having no volume for something when the menu volume is set to 1,
 * and the adjusted volume causes the SDL volume to be 0 (which could happen if we
 * allowed arbitrary adjustments)
 * 
 * I'm worried about the performance cost of setting the volume so often though?  maybe
 * we should test to see how it impacts, or examine the source?
 */

//PIZZA
//we could store pointers to the currently playing music and sound to check if a particular sound/music is playing


/////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////
//Our basic data structure for music
//this is essentially the same as sound right now, but we could potentially have music be more complicated, with looping sections?
typedef struct Music{
    Mix_Music *music;
    //an amount to add to the volume so that we can tweak the volume of individual sounds, as they may be unbalanced
    int volumeAdjust;
} Music;

//Our basic data structure for sound effects
typedef struct Sound{
    Mix_Chunk *chunk;
    //an amount to add to the volume so that we can tweak the volume of individual sounds, as they may be unbalanced
    int volumeAdjust;
    //what channel the sound is played on
    //cannot be used to check if playing, since the channel may have been given away
    int channel; 
} Sound;


/////////////////////////////////////////////////
// Loading/Unloading
/////////////////////////////////////////////////
void initSound();
void termSound();
Music *init_Music(Music *self);
Sound *init_Sound(Sound *self);
//NOTE: these don't check if the music/sound is stopped
void free_Music(Music *self);
void free_Sound(Sound *self);


/////////////////////////////////////////////////
// Loading from file
/////////////////////////////////////////////////
/*
 * Just like Images aren't loaded by data_reader, neither are sounds or music.
 * Unlike Images, they do read an auxillary JSON file first, as there are other attributes to consider.
 * These will crash the game if you pass NULL or there are problems reading the files.
 */
void fillMusicFromFile(char *filename, Music *self);
void fillSoundFromFile(char *filename, Sound *self);


/////////////////////////////////////////////////
// Music
/////////////////////////////////////////////////
void playMusic(Music *music);
void stopMusic();
void fadeInMusic(Music *music, int fadeDuration); //duration is in milliseconds
void fadeOutMusic(int fadeDuration);
// void fadeToMusic(Music *newMusic, int fadeDuration); //SDL_Mixer DOES NOT DO CROSSFADES
int musicIsPlaying();
int musicIsFadingIn();
int musicIsFadingOut();
void increaseMusicVolume();
void decreaseMusicVolume();


/////////////////////////////////////////////////
// Sounds
/////////////////////////////////////////////////
void playSound(Sound *sound);
void repeatSound(Sound *sound, int numRepeats);
void stopSound(Sound *sound);
void stopAllSound();
void increaseSoundVolume();
void decreaseSoundVolume();

#endif
