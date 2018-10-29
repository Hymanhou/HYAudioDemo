//
// Created by hyuan on 2018/10/28.
//
#include <malloc.h>
#include <string.h>
#include "wav.h"

#define LITTLE_ENDIAN   1
#define BIG_ENDIAN      2

typedef union _ENDIAN {
    short shortValue;
    char charValue[2];
}ENDIAN_UNION;

static int gLocalEndian = 0;

int getLocalIndianType() {
    ENDIAN_UNION sample;
    sample.shortValue = 0xFEEE;
    if (sample.charValue[0] == 0xFE && sample.charValue[1] == 0xEE) {
        return BIG_ENDIAN;
    }
    return LITTLE_ENDIAN;
}

void* toLittleEndian(void *pIn, int byteLen) {
    if (gLocalEndian == 0) {
        gLocalEndian = getLocalIndianType();
    }
    if (gLocalEndian == LITTLE_ENDIAN) {
        return pIn;
    }
    char *ps = (char*)pIn, *pe = (char*)pIn + byteLen;
    while (ps <= pe) {
        unsigned char c;
        c = *ps;
        *ps = *pe;
        *pe = c;
        ps++;
        pe--;
    }
    return pIn;
}

int writeWav(const char *pcmFile, const char *wavFile, int sampleRate, int bitPerSample, short channels) {
    FILE *fpPCM = fopen(pcmFile, "rb");
    if (!fpPCM)
        return -1;
    FILE *fpWAVE = fopen(wavFile, "wb");
    if (!fpWAVE)
        return -1;

    fseek(fpPCM, 0, SEEK_END);
    int PCMSize = ftell(fpPCM);

    WAV_HEAD wavHead;
    memset(&wavHead, 0, sizeof(wavHead));
    memcpy(wavHead.chunkID, "RIFF", 4);
    int si = sizeof(WAV_HEAD) - 8;
    wavHead.chunkSize = PCMSize + sizeof(WAV_HEAD) - 8;
    toLittleEndian(&(wavHead.chunkSize), sizeof(wavHead.chunkSize));
    memcpy(wavHead.format, "WAVE", 4);

    memcpy(wavHead.subChunk1ID, "fmt", 3);
    wavHead.subChunk1ID[3] = 0x20;
    wavHead.subChunk1Size = 16;
    wavHead.audioFormat = 1;
    wavHead.numChannels = channels;
    wavHead.sampleRate = sampleRate;
    wavHead.byteRate = sampleRate * channels * bitPerSample / 8;
    wavHead.blockAlign = channels * bitPerSample / 8;
    wavHead.bitPerSample = bitPerSample;

    toLittleEndian(&(wavHead.subChunk1Size), sizeof(wavHead.subChunk1Size));
    toLittleEndian(&(wavHead.audioFormat), sizeof(wavHead.audioFormat));
    toLittleEndian(&(wavHead.numChannels), sizeof(wavHead.numChannels));
    toLittleEndian(&(wavHead.sampleRate), sizeof(wavHead.sampleRate));
    toLittleEndian(&(wavHead.byteRate), sizeof(wavHead.byteRate));
    toLittleEndian(&(wavHead.blockAlign), sizeof(wavHead.blockAlign));
    toLittleEndian(&(wavHead.bitPerSample), sizeof(wavHead.bitPerSample));

    memcpy(wavHead.subChunk2ID, "data", 4);
    wavHead.subChunk2Size = PCMSize;
    toLittleEndian(&(wavHead.subChunk2Size), sizeof(wavHead.subChunk2Size));

    fseek(fpPCM, 0, 0);

    fwrite(&wavHead, 1, sizeof(WAV_HEAD), fpWAVE);
    short buff[1024];
    int readLen = 0;
    while ((readLen = fread(buff, 2, 1024, fpPCM)) > 0) {
        fwrite(buff, 2, readLen, fpWAVE);
    }

    fclose(fpPCM);
    fclose(fpWAVE);
    return 0;
}
