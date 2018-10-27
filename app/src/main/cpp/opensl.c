#include "opensl.h"
#define CONV16BIT 32768
#define CONVMYFLT (1./32768.)

static void* createThreadLock(void);
static int waitThreadLock(void *lock);
static void notifyThreadLock(void *lock);
static void destroyThreadLock(void *lock);
static void bqPlayerCallBack(SLAndroidSimpleBufferQueueItf bq, void *context);
static void bqRecorderCallBack(SLAndroidSimpleBufferQueueItf bq, void *context);
static SLuint32 getSLSampleRate(SLuint32 sampleRate);

static SLresult openSLCreateEngine(OPENSL_STREAM *p) 
{
	SLresult result;
	result = slCreateEngine(&(p->engineObject), 0, NULL, 0, NULL, NULL);
	if (result != SL_RESULT_SUCCESS) return result;
	
	result = (*p->engineObject)->Realize(p->engineObject, SL_BOOLEAN_FALSE);
	if (result != SL_RESULT_SUCCESS) return result;

	result = (*p->engineObject)->GetInterface(p->engineObject, SL_IID_ENGINE, &(p->engineEngine));
	if (result != SL_RESULT_SUCCESS) return result;

	return SL_RESULT_SUCCESS;
}

static SLresult openSLPlayOpen(OPENSL_STREAM *p)
{
	SLresult result;
	SLuint32 sampleRate = p->sampleRate;
	SLuint32 channels = p->outChannels;

	if (channels)
	{
		SLDataLocator_AndroidSimpleBufferQueue localBuffQueue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };

		sampleRate = getSLSampleRate(sampleRate);
		if (sampleRate == -1) return -1;

		const SLInterfaceID ids[] = { SL_IID_VOLUME };
		const SLboolean req[] = { SL_BOOLEAN_FALSE };
		result = (*p->engineEngine)->CreateOutputMix(p->engineEngine, &(p->outputMixObject), 1, ids, req);
		if (result != SL_RESULT_SUCCESS) return result;
        result = (*p->outputMixObject)->Realize(p->outputMixObject, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) return result;

		int speakers;
		if (channels > 1)
        {
            speakers = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
        }
        else
        {
            speakers = SL_SPEAKER_FRONT_CENTER;
        }

        SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, channels, sampleRate, SL_PCMSAMPLEFORMAT_FIXED_16,
                                       SL_PCMSAMPLEFORMAT_FIXED_16, speakers, SL_BYTEORDER_LITTLEENDIAN};
		SLDataSource dataSource;
		dataSource.pFormat = &format_pcm;
		dataSource.pLocator = &localBuffQueue;

		SLDataLocator_OutputMix loc_outmix;
		loc_outmix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
		loc_outmix.outputMix = p->outputMixObject;
		SLDataSink dataSink;
		dataSink.pLocator = &loc_outmix;
		dataSink.pFormat = NULL;

		const SLInterfaceID playerInterfaces[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
		const SLboolean playerRequire[] = {SL_BOOLEAN_TRUE};
		result = (*p->engineEngine)->CreateAudioPlayer(p->engineEngine, &(p->bqPlayerObject),
		        &dataSource, &dataSink, 1, playerInterfaces, playerRequire);
        if (result != SL_RESULT_SUCCESS) return result;
        result = (*p->bqPlayerObject)->Realize(p->bqPlayerObject, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) return result;

		result = (*p->bqPlayerObject)->GetInterface(p->bqPlayerObject, SL_IID_PLAY, &(p->bqPlayerPlay));
		if (result != SL_RESULT_SUCCESS) return result;

		result = (*p->bqPlayerObject)->GetInterface(p->bqPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
			&(p->bqPlayerBufferQueue));
		if (result != SL_RESULT_SUCCESS) return result;

		result = (*p->bqPlayerBufferQueue)->RegisterCallback(p->bqPlayerBufferQueue, bqPlayerCallBack, p);
		if (result != SL_RESULT_SUCCESS) return result;

		result = (*p->bqPlayerPlay)->SetPlayState(p->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
		return result;
	}

	return SL_RESULT_SUCCESS;
}

static SLresult openSLRecorderOpen(OPENSL_STREAM *p)
{
	SLresult result;
	SLuint32 sampleRate = p->sampleRate;
	SLuint32 channels = p->inChannels;

	if (channels)
	{
		sampleRate = getSLSampleRate(sampleRate);
		if (sampleRate == -1) return -1;

//		SLuint32 inputDeviceIDs[3];
//		SLint32 numInputs = 3;
//		SLboolean micAvailable = SL_BOOLEAN_FALSE;
//		SLuint32 micDeviceID = 0;
//		SLAudioIODeviceCapabilitiesItf audioIODeviceCapabilitiesItf;
//		SLAudioInputDescriptor audioInputDescriptor;
//		result = (*p->engineObject)->GetInterface(p->engineObject, SL_IID_AUDIOIODEVICECAPABILITIES, &(audioIODeviceCapabilitiesItf));
//		if (result != SL_RESULT_SUCCESS)
//            return result;
//		result = (*audioIODeviceCapabilitiesItf)->GetAvailableAudioInputs(audioIODeviceCapabilitiesItf,
//		        &numInputs, inputDeviceIDs);
//        if (result != SL_RESULT_SUCCESS)
//            return result;
//        for (int i = 0; i < numInputs; ++i)
//        {
//            result = (*audioIODeviceCapabilitiesItf)->QueryAudioInputCapabilities(audioIODeviceCapabilitiesItf,
//                    inputDeviceIDs[i], &audioInputDescriptor);
//            if (result != SL_RESULT_SUCCESS)
//                return result;
//            if ((audioInputDescriptor.deviceConnection == SL_DEVCONNECTION_ATTACHED_WIRED) &&
//                    (audioInputDescriptor.deviceLocation == SL_DEVLOCATION_HEADSET))
//            {
//                micDeviceID = inputDeviceIDs[i];
//                micAvailable = SL_BOOLEAN_TRUE;
//                break;
//            }
//            else if ((audioInputDescriptor.deviceConnection == SL_DEVCONNECTION_INTEGRATED) &&
//                    (audioInputDescriptor.deviceScope == SL_DEVSCOPE_USER) &&
//                    (audioInputDescriptor.deviceLocation == SL_DEVLOCATION_HANDSET))
//            {
//                micAvailable = SL_BOOLEAN_TRUE;
//                micDeviceID = inputDeviceIDs[i];
//                break;
//            }
//        }
//        if (!micAvailable)
//            return -1;
		SLDataLocator_IODevice loc_dev = { SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT, 
			SL_DEFAULTDEVICEID_AUDIOINPUT, NULL };
		SLDataSource audioSrc = { &loc_dev, NULL };

		int speakers;
		if (channels > 1)
		{
			speakers = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
		}
		else
		{
			speakers = SL_SPEAKER_FRONT_CENTER;
		}

		SLDataLocator_AndroidSimpleBufferQueue loc_bq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
		SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, channels, sampleRate,
                                       SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                       speakers, SL_BYTEORDER_LITTLEENDIAN};
		SLDataSink audioSink = { &loc_bq, &format_pcm };

		const SLInterfaceID ids[1] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
		const SLboolean reqs[1] = { SL_BOOLEAN_TRUE };
		result = (*p->engineEngine)->CreateAudioRecorder(p->engineEngine, &(p->recordObject), &audioSrc,
			&audioSink, 1, ids, reqs);
		if (result != SL_RESULT_SUCCESS) return result;

		result = (*p->recordObject)->Realize(p->recordObject, SL_BOOLEAN_FALSE);
		if (result != SL_RESULT_SUCCESS) return result;

		result = (*p->recordObject)->GetInterface(p->recordObject, SL_IID_RECORD, &(p->recorderRecord));
		if (result != SL_RESULT_SUCCESS) return result;

		result = (*p->recordObject)->GetInterface(p->recordObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &(p->recorderBuffQueue));
		if (result != SL_RESULT_SUCCESS) return result;

		result = (*p->recorderBuffQueue)->RegisterCallback(p->recorderBuffQueue, bqRecorderCallBack, p);
		if (result != SL_RESULT_SUCCESS) return result;

		result = (*p->recorderRecord)->SetRecordState(p->recorderRecord, SL_RECORDSTATE_RECORDING);

		return result;
	}

	return SL_RESULT_SUCCESS;
}

static void openSLDestroyEngine(OPENSL_STREAM *p)
{
	if (p->bqPlayerObject != NULL)
	{
		(*p->bqPlayerObject)->Destroy(p->bqPlayerObject);
		p->bqPlayerObject = NULL;
		p->bqPlayerPlay = NULL;
		p->bqPlayerBufferQueue = NULL;
		p->bqPlayerEffectSend = NULL;
	}

	if (p->recordObject != NULL)
	{
		(*p->recordObject)->Destroy(p->recordObject);
		p->recordObject = NULL;
		p->recorderRecord = NULL;
		p->recorderBuffQueue = NULL;
	}

	if (p->outputMixObject != NULL)
	{
		(*p->outputMixObject)->Destroy(p->outputMixObject);
		p->outputMixObject = NULL;
	}

	if (p->engineObject != NULL)
	{
		(*p->engineObject)->Destroy(p->engineObject);
		p->engineObject = NULL;
		p->engineEngine = NULL;
	}
}

OPENSL_STREAM* android_OpenAudioDevice(int sampleRate, int inChannels, int outChannels, int bufferFrames)
{
	OPENSL_STREAM *p;
	p = (OPENSL_STREAM *)calloc(sizeof(OPENSL_STREAM), 1);

	p->inChannels = inChannels;
	p->outChannels = outChannels;
	p->sampleRate = sampleRate;
	p->inlock = createThreadLock();
	p->outlock = createThreadLock();

	if ((p->outBufSamples = bufferFrames * outChannels) != 0)
	{
		if ((p->outputBuffer[0] = (short *) calloc(p->outBufSamples, sizeof(char))) == NULL ||
			(p->outputBuffer[1] = (short *) calloc(p->outBufSamples, sizeof(char))) == NULL)
		{
			android_CloseAudioDevice(p);
			return NULL;
		}
	}

	if ((p->inBufSamples = bufferFrames * inChannels) != 0)
	{
		if ((p->inputBuffer[0] = (short *) calloc(p->inBufSamples, sizeof(char))) == NULL ||
			(p->inputBuffer[1] = (short *) calloc(p->inBufSamples, sizeof(char))) == NULL)
		{
			android_CloseAudioDevice(p);
			return NULL;
		}
	}

	p->currentInputIndex = 0;
	p->currentOutputBuffer = 0;
	p->currentInputIndex = p->inBufSamples;
	p->currentInputBuffer = 0;

	if (openSLCreateEngine(p) != SL_RESULT_SUCCESS)
	{
		android_CloseAudioDevice(p);
		return NULL;
	}

	if (openSLRecorderOpen(p) != SL_RESULT_SUCCESS)
	{
		android_CloseAudioDevice(p);
		return NULL;
	}

	if (openSLPlayOpen(p) != SL_RESULT_SUCCESS)
	{
		android_CloseAudioDevice(p);
		return NULL;
	}

	notifyThreadLock(p->outlock);
	notifyThreadLock(p->inlock);

	p->time = 0;
	return p;
}

void android_CloseAudioDevice(OPENSL_STREAM *p)
{
	if (p == NULL)
	{
		return;
	}

	openSLDestroyEngine(p);

	if (p->inlock != NULL)
	{
		notifyThreadLock(p->inlock);
		destroyThreadLock(p->inlock);
		p->inlock = NULL;
	}

	if (p->outlock != NULL)
	{
		notifyThreadLock(p->outlock);
		destroyThreadLock(p->outlock);
		p->outlock = NULL;
	}

	if (p->outputBuffer[0] != NULL)
	{
		free(p->outputBuffer[0]);
		p->outputBuffer[0] = NULL;
	}

	if (p->outputBuffer[1] != NULL)
	{
		free(p->outputBuffer[1]);
		p->outputBuffer[1] = NULL;
	}

	if (p->inputBuffer[0] != NULL)
	{
		free(p->inputBuffer[0]);
		p->inputBuffer[0] = NULL;
	}

	if (p->inputBuffer[1] != NULL)
	{
		free(p->inputBuffer[1]);
		p->inputBuffer[1] = NULL;
	}

	free(p);
}

void bqRecorderCallBack(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	OPENSL_STREAM *p = (OPENSL_STREAM *)context;
	notifyThreadLock(p->inlock);
}

int android_AudioIn(OPENSL_STREAM *p, short *buff, int size)
{
	short *inBuffer;
	int i, bufSamples = p->inBufSamples, index = p->currentInputIndex;
	if (p == NULL || bufSamples == 0) return 0;
	
	inBuffer = p->inputBuffer[p->currentInputBuffer];
	for (i = 0; i < size; i++)
	{
		if (index >= bufSamples)
		{
			waitThreadLock(p->inlock);
			(*p->recorderBuffQueue)->Enqueue(p->recorderBuffQueue, inBuffer, bufSamples * sizeof(char));
			p->currentInputBuffer = (p->currentInputBuffer ? 0 : 1);
			index = 0;
			inBuffer = p->inputBuffer[p->currentInputBuffer];
		}
		buff[i] = (short)inBuffer[index++];
	}

	p->currentInputIndex = index;
	if (p->outChannels == 0)
	{
		p->time += (double)size / (p->sampleRate * p->inChannels);
	}

	return i;
}

void bqPlayerCallBack(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    OPENSL_STREAM *p = (OPENSL_STREAM *) context;
    notifyThreadLock(p->outlock);
}

int android_AudioOut(OPENSL_STREAM *p, short *buff, int size)
{
	short *outBuffer;
	int i, buffSamples = p->outBufSamples, index = p->currentOutputIndex;
	if (p == NULL || buffSamples == 0) return 0;

	outBuffer = p->outputBuffer[p->currentOutputBuffer];
	for (i = 0; i < size; i++)
	{
		outBuffer[index++] = (short)(buff[i]);
		if (index >= p->outBufSamples)
		{
			waitThreadLock(p->outlock);
			(*p->bqPlayerBufferQueue)->Enqueue(p->bqPlayerBufferQueue, outBuffer, buffSamples * sizeof(short));
			p->currentOutputBuffer = (p->currentOutputBuffer) ? 0 : 1;
			index = 0;
			outBuffer = p->outputBuffer[p->currentOutputBuffer];
		}
	}

	p->currentOutputIndex = index;
	p->time += (double)size / (p->sampleRate * p->outChannels);
	return i;
}

double android_getTimestamp(OPENSL_STREAM *p)
{
	return p->time;
}

static SLuint32 getSLSampleRate(SLuint32 sampleRate)
{
	SLuint32 sampleRateLocal = sampleRate;
	switch (sampleRate)
	{
	case 8000:
		sampleRateLocal = SL_SAMPLINGRATE_8;
		break;
	case 11025:
		sampleRateLocal = SL_SAMPLINGRATE_11_025;
		break;
	case 16000:
		sampleRateLocal = SL_SAMPLINGRATE_16;
		break;
	case 22050:
		sampleRateLocal = SL_SAMPLINGRATE_22_05;
		break;
	case 24000:
		sampleRateLocal = SL_SAMPLINGRATE_24;
		break;
	case 32000:
		sampleRateLocal = SL_SAMPLINGRATE_32;
		break;
	case 44100:
		sampleRateLocal = SL_SAMPLINGRATE_44_1;
		break;
	case 48000:
		sampleRateLocal = SL_SAMPLINGRATE_48;
		break;
	case 64000:
		sampleRateLocal = SL_SAMPLINGRATE_64;
		break;
	case 88200:
		sampleRateLocal = SL_SAMPLINGRATE_88_2;
		break;
	case 96000:
		sampleRateLocal = SL_SAMPLINGRATE_96;
		break;
	case 192000:
		sampleRateLocal = SL_SAMPLINGRATE_192;
		break;
	default:
		return -1;
	}
	return sampleRateLocal;
}

void* createThreadLock(void)
{
	threadLock *p;
	p = (threadLock *)malloc(sizeof(threadLock));
	if (p == NULL)
		return NULL;

	memset(p, 0, sizeof(threadLock));
	if (pthread_mutex_init(&(p->m), NULL) != 0)
	{
		free(p);
		return NULL;
	}

	if (pthread_cond_init(&(p->c), NULL) != 0)
	{
		free(p);
		return NULL;
	}

	p->s = 1;
	return p;
}

int waitThreadLock(void *lock)
{
	threadLock *p;
	p = (threadLock *)lock;
	pthread_mutex_lock(&(p->m));
	while (!p->s)
	{
		pthread_cond_wait(&(p->c), &(p->m));
	}
	p->s = 0;
	pthread_mutex_unlock(&(p->m));
}

void notifyThreadLock(void *lock)
{
	threadLock *p;
	p = (threadLock*)lock;
	pthread_mutex_lock(&(p->m));
	p->s = 1;
	pthread_cond_signal(&(p->c));
	pthread_mutex_unlock(&(p->m));
}

void destroyThreadLock(void *lock)
{
	threadLock *p;
	p = (threadLock*)lock;
	if (p == NULL) return;
	notifyThreadLock(p);
	pthread_cond_destroy(&(p->c));
	pthread_mutex_destroy(&(p->m));
	free(p);
}