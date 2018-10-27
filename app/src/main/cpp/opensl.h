#ifndef OPENSL_IO
#define OPENSL_IO

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct threadLock_ 
{
	pthread_mutex_t m;
	pthread_cond_t c;
	unsigned char s;
}threadLock;

#ifdef __cplusplus
extern "c" {
#endif
	typedef struct opensl_stream 
	{
		SLObjectItf engineObject;
		SLEngineItf engineEngine;

		SLObjectItf outputMixObject;

		SLObjectItf bqPlayerObject;
		SLPlayItf bqPlayerPlay;
		SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
		SLEffectSendItf bqPlayerEffectSend;

		SLObjectItf recordObject;
		SLRecordItf recorderRecord;
		SLAndroidSimpleBufferQueueItf recorderBuffQueue;

		int currentInputIndex;
		int currentOutputIndex;

		int currentOutputBuffer;
		int currentInputBuffer;

		short *outputBuffer[2];
		short *inputBuffer[2];

		int outBufSamples;
		int inBufSamples;

		void* inlock;
		void* outlock;

		double time;
		int inChannels;
		int outChannels;
		int sampleRate;
	} OPENSL_STREAM;

	OPENSL_STREAM* android_OpenAudioDevice(int sampleRate, int inChannels, int outChannels, int bufferFrames);

	void android_CloseAudioDevice(OPENSL_STREAM *p);

	int android_AudioIn(OPENSL_STREAM *p, short *buff, int size);

	int android_AudioOut(OPENSL_STREAM *p, short *buff, int size);

	double android_getTimestamp(OPENSL_STREAM *p);

#ifdef __cplusplus
};
#endif


#endif // !OPENSL_IO



