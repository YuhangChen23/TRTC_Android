package com.tencent.trtc.audiocall;

public class JavaCallCpp {
    static {
        System.loadLibrary("crypto");
        System.loadLibrary("ssl");
        System.loadLibrary("tencentcrypy");
    }

    private static final String encryptKey = "Ak123456789012345678901234567890";
    private volatile static JavaCallCpp mInstance;

    public static JavaCallCpp sharedInstance() {
        if (mInstance == null) {
            synchronized (JavaCallCpp.class) {
                if (mInstance == null) {
                    mInstance = new JavaCallCpp();
                }
            }
        }
        return mInstance;
    }

    public long getEncodedDataProcessingListener() {
        return GetEncodedDataProcessingListener(encryptKey);
    }

    public native long GetEncodedDataProcessingListener(String encryptKey);
}
