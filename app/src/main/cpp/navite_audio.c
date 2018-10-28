//
// Created by 侯塬 on 2018/10/25.
//
#include "native_audio.h"
#include "wav.h"

/*
 * Class:     com_hyman_audiodemo_NativeAudio
 * Method:    nativeStartCapture
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_hyman_audiodemo_NativeAudio_nativeStartCapture(JNIEnv *env, jobject thiz)
{
    FILE *fp = fopen(TEST_CAPTURE_FILE_PATH, "wb");
    if (fp == NULL) {
        LOG("cannot open file (%s)\n", TEST_CAPTURE_FILE_PATH);
        return -1;
    }

    OPENSL_STREAM* stream = android_OpenAudioDevice(SAMPLE_RATE, CHANNELS, CHANNELS, FRAME_SIZE);
    if (stream == NULL) {
        fclose(fp);
        LOG("failed to open audio device!\n");
        return JNI_FALSE;
    }

    int samples;
    short buffer[BUFFER_SIZE];
    g_loop_exit = 0;
    while (!g_loop_exit) {
        samples = android_AudioIn(stream, buffer, BUFFER_SIZE);
        if (samples < 0) {
            LOG("android_AudioIn failed\n");
            break;
        }
        if (fwrite((unsigned char *)buffer, samples * sizeof(short), 1, fp) != 1) {
            LOG("failed to save captured data!\n");
            break;
        }
        LOG("capture %d samples\n", samples);
    }

    android_CloseAudioDevice(stream);
    fclose(fp);
    LOG("naviteStartCapture complete!");

    writeWav(TEST_CAPTURE_FILE_PATH, "/sdcard/wav_test.wav", samples, 16, CHANNELS);

    return JNI_TRUE;
}

/*
 * Class:     com_hyman_audiodemo_NativeAudio
 * Method:    nativeStopCapture
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_hyman_audiodemo_NativeAudio_nativeStopCapture(JNIEnv *env, jobject thiz)
{
    g_loop_exit = 1;
    return JNI_TRUE;
}

/*
 * Class:     com_hyman_audiodemo_NativeAudio
 * Method:    nativeStartPlayback
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_hyman_audiodemo_NativeAudio_nativeStartPlayback(JNIEnv *env, jobject thiz)
{
    FILE *fp = fopen(TEST_CAPTURE_FILE_PATH, "rb");
    if (fp == NULL) {
        LOG("cannot open file (%s)!\n", TEST_CAPTURE_FILE_PATH);
        return -1;
    }

    OPENSL_STREAM* stream = android_OpenAudioDevice(SAMPLE_RATE, CHANNELS, CHANNELS, FRAME_SIZE);
    if (stream == NULL) {
        fclose(fp);
        LOG("failed to open audio device!\n");
        return JNI_FALSE;
    }

    int samples;
    short buffer[BUFFER_SIZE];
    g_loop_exit = 0;
    while (!g_loop_exit && !feof(fp)) {
        if (fread((unsigned char *)buffer, BUFFER_SIZE*2, 1, fp) != 1) {
            LOG("failed to read data!\n");
            break;
        }
        samples = android_AudioOut(stream, buffer, BUFFER_SIZE);
        if (samples < 0) {
            LOG("android_AudioOut failed!\n");
        }
        LOG("playback %d samples!\n", samples);
    }

    android_CloseAudioDevice(stream);
    fclose(fp);
    LOG("nativeStartPlayback completed!");
    return JNI_TRUE;
}

/*
 * Class:     com_hyman_audiodemo_NativeAudio
 * Method:    nativeStopPlayback
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_hyman_audiodemo_NativeAudio_nativeStopPlayback(JNIEnv *env, jobject thiz)
{
    g_loop_exit = 1;
    return JNI_TRUE;
}

