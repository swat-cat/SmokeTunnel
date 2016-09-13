//
// Created by Maxim Ermakov on 9/8/16.
//
#include "com_example_max_ermakov_smoketunnel_SmokeNDKManager.h"
#include "smoke.h"
#include <android/log.h>

static smoke* ptr = NULL;

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "smoker::", __VA_ARGS__))


JNIEXPORT jstring JNICALL Java_com_example_max_1ermakov_smoketunnel_SmokeNDKManager_init
  (JNIEnv *env, jobject)
  {
            LOGI("Before smoker create");
            ptr = new smoke();
            ptr->request_big_packets = 0;
            ptr->local_port = 4987;
            ptr->remote_host_index = 0;
            ptr->handshake_timeout = 5000;
            ptr->connection_timeout = 10000;
            ptr->hop_timeout = 5000;
            ptr->hop_interval = 30 * 60 * 1000;
            ptr->hop_attempts = 10;
            ptr->remote_hosts = {std::make_pair("45.32.181.159",8080)};
            LOGI("Before smoker run");
            ptr->run();

            return env->NewStringUTF("Lets smoke!");
  }

JNIEXPORT jstring JNICALL Java_com_example_max_1ermakov_smoketunnel_SmokeNDKManager_close
  (JNIEnv *env, jobject)
  {         LOGI("Before close");
            ptr->close_smoke();
            LOGI("Before delete");
            delete ptr;
            LOGI("After delete");
            return env->NewStringUTF("Lets quit smoking!");
  }