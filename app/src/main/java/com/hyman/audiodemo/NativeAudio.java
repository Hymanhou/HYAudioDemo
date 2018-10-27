package com.hyman.audiodemo;

public class NativeAudio {

    private boolean mIsTestCapture = true;

    public NativeAudio(boolean isTestCapture) {
        mIsTestCapture = isTestCapture;
    }

    public boolean startTesting() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                if (mIsTestCapture) {
                    nativeStartCapture();
                } else {
                    nativeStartPlayback();
                }
            }
        }).start();
        return false;
    }

    public boolean stopTesting() {
        if (mIsTestCapture) {
            nativeStopCapture();
        } else {
            nativeStopPlayback();
        }
        return false;
    }

    private native boolean nativeStartCapture();
    private native boolean nativeStopCapture();
    private native boolean nativeStartPlayback();
    private native boolean nativeStopPlayback();

    static {
        System.loadLibrary("native_audio");
    }

}

