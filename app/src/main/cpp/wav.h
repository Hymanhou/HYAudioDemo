//
// Created by hyuan on 2018/10/28.
//

#ifndef HYAUDIODEMO_WAV_H
#define HYAUDIODEMO_WAV_H

typedef struct _WAV_HEAD{
    char chunkID[4];
    unsigned int chunkSize;
    char format[4];

    char subChunk1ID[4];
    unsigned int subChunk1Size;
    unsigned short audioFormat;
    unsigned short numChannels;
    unsigned int sampleRate;
    unsigned int byteRate;
    unsigned short blockAlign;
    unsigned short bitPerSample;

    char subChunk2ID[4];
    unsigned int subChunk2Size;
}WAV_HEAD;

#ifdef __cplusplus
extern "c" {
#endif
    int writeWav(const char *pcmFile, const char *wavFile, int sampleRate, int bitPerSample, short channels);
#ifdef __cplusplus
};
#endif
#endif //HYAUDIODEMO_WAV_H
