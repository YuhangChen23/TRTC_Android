// Copyright (c) 2021 Tencent. All rights reserved.

#include <android/log.h>
#include <jni.h>
#include <string>

#include "TXLiteAVEncodedDataProcessingListener.h"

class TRTCCustomerEncryptor : public liteav::ITXLiteAVEncodedDataProcessingListener {
 public:
  TRTCCustomerEncryptor() {
    __android_log_print(ANDROID_LOG_ERROR, "TRTCCustomerEncryptor", "construct :%p", this);
  }
  bool didEncodeVideo(liteav::TXLiteAVEncodedData& videoData) override {
    if (videoData.processedData && !encrypt_key_.empty()) {
      XORData(videoData);
      return true;
    }
    return false;
  }

  bool willDecodeVideo(liteav::TXLiteAVEncodedData& videoData) override {
    if (videoData.processedData && !encrypt_key_.empty()) {
      XORData(videoData);
      return true;
    }
    return false;
  }

  bool didEncodeAudio(liteav::TXLiteAVEncodedData& audioData) override {
    if (audioData.processedData && !encrypt_key_.empty()) {
      XORData(audioData);
      return true;
    }
    return false;
  }

  bool willDecodeAudio(liteav::TXLiteAVEncodedData& audioData) override {
    if (audioData.processedData && !encrypt_key_.empty()) {
      XORData(audioData);
      return true;
    }
    return false;
  }

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

  std::string encrypt_key_;
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

    assert(env != nullptr);
    result = JNI_VERSION_1_6;
    return result;
}

JNIEXPORT jlong JNICALL
Java_com_tencent_trtc_audiocall_JavaCallCpp_GetEncodedDataProcessingListener(
    JNIEnv* env,
    jobject thiz,
    jstring encryptKey) {
  if (encryptKey == nullptr)
    return 0;
  jsize keyLen = env->GetStringUTFLength(encryptKey);

  if (keyLen != 32)
    return 0;

  const char* c_str = nullptr;

  c_str = env->GetStringUTFChars(encryptKey, nullptr);

  if (c_str == nullptr) {
    return 0;
  }
  s_customerCryptor.encrypt_key_ = c_str;
  env->ReleaseStringUTFChars(encryptKey, c_str);

  return (jlong)&s_customerCryptor;
}

#ifdef __cplusplus
}
#endif