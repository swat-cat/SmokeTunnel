//
// Created by Maxim Ermakov on 9/8/16.
//
#include "com_example_max_ermakov_smoketunnel_MainActivity.h"
#include <jni.h>
#include "smoke.h"

JNIEXPORT jstring JNICALL Java_com_example_max_1ermakov_smoketunnel_MainActivity_init(JNIEnv *env, jobject thiz)
  {
            smoke* ptr = new smoke();
            ptr->run();

            return env->NewStringUTF("Lets smoke!");
  }