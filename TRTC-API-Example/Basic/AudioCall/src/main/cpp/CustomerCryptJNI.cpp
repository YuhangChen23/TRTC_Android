// Copyright (c) 2021 Tencent. All rights reserved.

#include <android/log.h>
#include <jni.h>
#include <string>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "TXLiteAVEncodedDataProcessingListener.h"

class TRTCCustomerEncryptor : public liteav::ITXLiteAVEncodedDataProcessingListener {
public:
    TRTCCustomerEncryptor() {
        __android_log_print(ANDROID_LOG_ERROR, "TRTCCustomerEncryptor", "construct :%p", this);
        // 初始化 ssl 操作
        ctx_audio_send = EVP_CIPHER_CTX_new();
        ctx_audio_receive = EVP_CIPHER_CTX_new();

         const unsigned char cipherKey[] = "1234567890abcdef1234567890abcdef";
         encryptKeyLength = sizeof(cipherKey);
         const unsigned char cipherIv[] = "1234567812345678";
         encryptIvLength = sizeof(cipherIv);

        encryptKey = new unsigned char[encryptKeyLength];
        for (int i = 0; i < encryptKeyLength;i++) {
            encryptKey[i] = cipherKey[i];
        }
        encryptIv = new unsigned char[encryptIvLength];
        for (int i = 0;i < encryptIvLength;i++) {
            encryptIv[i] = cipherIv[i];
        }
        __android_log_print(ANDROID_LOG_ERROR, "cyh", "GET KEY: %s, GET IV: %s", encryptKey, encryptIv);

        resetCtx(ctx_audio_send, true);
        resetCtx(ctx_audio_receive, false);

        TestEncryptAndDecrypt();
    }


    bool didEncodeVideo(liteav::TXLiteAVEncodedData &videoData) override {
//        if (videoData.processedData) {
//          XORData(videoData);
//          return true;
//        }
        return false;
    }

    bool willDecodeVideo(liteav::TXLiteAVEncodedData &videoData) override {
//        if (videoData.processedData) {
//          XORData(videoData);
//          return true;
//        }
        return false;
    }

    bool didEncodeAudio(liteav::TXLiteAVEncodedData &audioData) override {
        if (audioData.processedData && audioData.originData) {
            EncryptAudioData(audioData);
//            XORData(audioData);
            return true;
        }
        return false;
    }

    bool willDecodeAudio(liteav::TXLiteAVEncodedData &audioData) override {
        if (audioData.processedData && audioData.originData) {
            DecryptAudioData(audioData);
//            XORData(audioData);
            return true;
        }
        return false;
    }


private:
    void resetCtx(EVP_CIPHER_CTX* ctx, bool isEncrypt) {
        if (isEncrypt) {
            EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr);
            EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, encryptIvLength, nullptr);
            EVP_EncryptInit_ex(ctx, nullptr, nullptr, encryptKey, encryptIv);
        } else {
            EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr);
            EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, encryptIvLength, nullptr);
            EVP_DecryptInit_ex(ctx, nullptr, nullptr, encryptKey, encryptIv);
        }
    }
    void TestEncryptAndDecrypt() {
        std::string cyh = "i am coder, i am stupid";
        int inputLen = cyh.length();
        const char* cyhChar = cyh.c_str();
        const auto* cyhUint8 = reinterpret_cast<const uint8_t*>(cyhChar);
        int outLength;
        uint8_t outBuf[inputLen];
        EVP_EncryptUpdate(ctx_audio_send, outBuf, &outLength, cyhUint8, inputLen);
        __android_log_print(ANDROID_LOG_ERROR, "cyh", "get encrypt: %d", outLength);

        int outLength1;
        uint8_t outBuf1[inputLen];
        EVP_DecryptUpdate(ctx_audio_receive, outBuf1, &outLength1, outBuf, inputLen);
        __android_log_print(ANDROID_LOG_ERROR, "cyh", "get decrypt: %d", outLength1);

        auto* ret = new char[outLength1+1];
        for (int i = 0;i<outLength1;i++) {
            ret[i] = outBuf1[i];
        }
        ret[outLength1] = '\0';
        __android_log_print(ANDROID_LOG_ERROR, "cyh", "get ret: %s", ret);
    }

    void EncryptAudioData(liteav::TXLiteAVEncodedData &originData) {
        auto srcData = originData.originData->cdata();
        auto srcDataSize = originData.originData->size();

        int outLength;
        auto* outBuf = new uint8_t[srcDataSize];
        resetCtx(ctx_audio_send, true);
        int updateRet = EVP_EncryptUpdate(ctx_audio_send, outBuf, &outLength, srcData, srcDataSize);
        if (updateRet != 1) {
            __android_log_print(ANDROID_LOG_ERROR, "cyh", "err when EVP_EncryptUpdate");
        }

        originData.processedData->SetSize(outLength);
        auto dstData = originData.processedData->data();
        for (int i = 0; i < outLength; ++i) {
            dstData[i] = outBuf[i];
        }
        delete[] (outBuf);
        __android_log_print(ANDROID_LOG_ERROR, "cyh", "encrypt, originData: %s, afterData: %s, originDataSize: %lu, aftersize: %d", srcData, originData.processedData->cdata(), srcDataSize, outLength);
    }

    void DecryptAudioData(liteav::TXLiteAVEncodedData &originData) {
        auto srcData = originData.originData->cdata();
        auto srcDataSize = originData.originData->size();

        int outLength;
        auto* outBuf = new uint8_t[srcDataSize];
        resetCtx(ctx_audio_receive, false);
        int updateRet = EVP_DecryptUpdate(ctx_audio_receive, outBuf, &outLength, srcData, srcDataSize);
        if (updateRet != 1) {
            __android_log_print(ANDROID_LOG_ERROR, "cyh", "err when EVP_DecryptUpdate");
        }

        originData.processedData->SetSize(outLength);
        auto dstData = originData.processedData->data();
        for (int i = 0; i < outLength; ++i) {
            dstData[i] = outBuf[i];
        }
        delete[] (outBuf);
        __android_log_print(ANDROID_LOG_ERROR, "cyh", "decrypt, originData: %s, afterData: %s, originDataSize: %lu, aftersize: %d", srcData, originData.processedData->cdata(), srcDataSize, outLength);
    }

    // old demo for test
    void XORData(liteav::TXLiteAVEncodedData& encodedData) {
        auto srcData = encodedData.originData->cdata();
        auto keySize = encrypt_key_.size();
        auto dataSize = encodedData.originData->size();
        encodedData.processedData->SetSize(dataSize);
        auto dstData = encodedData.processedData->data();
        __android_log_print(ANDROID_LOG_ERROR, "cyh", "originData size: %lu, encrypt key len: %lu", dataSize, keySize);
        for (int i = 0; i < dataSize; ++i) {
            dstData[i] = srcData[i] ^ encrypt_key_[i % keySize];
        }
    }

private:
    EVP_CIPHER_CTX *ctx_audio_send;
    EVP_CIPHER_CTX *ctx_audio_receive;
    //    EVP_CIPHER_CTX *ctx_video_send;
    //    EVP_CIPHER_CTX *ctx_video_receive;

    std::string encrypt_key_ = "Ak123456789012345678901234567890";

    unsigned char *encryptKey;
    unsigned char *encryptIv;
    int encryptKeyLength = 32;
    int encryptIvLength = 16;
};

static TRTCCustomerEncryptor s_customerCryptor;

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    jint result = -1;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return result;
    }

    result = JNI_VERSION_1_6;
    return result;
}

JNIEXPORT jlong JNICALL
Java_com_tencent_trtc_audiocall_JavaCallCpp_GetEncodedDataProcessingListener(
        JNIEnv *env,
        jobject thiz,
        jstring encryptKey) {
    if (encryptKey == nullptr)
        return 0;
    jsize keyLen = env->GetStringUTFLength(encryptKey);

    if (keyLen != 32)
        return 0;

    const char *c_str = nullptr;

    c_str = env->GetStringUTFChars(encryptKey, nullptr);

    if (c_str == nullptr) {
        return 0;
    }
//    s_customerCryptor.cipherKey = c_str;
    env->ReleaseStringUTFChars(encryptKey, c_str);

    return (jlong) &s_customerCryptor;
}

#ifdef __cplusplus
}
#endif