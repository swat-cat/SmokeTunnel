//
// Created by Maxim Ermakov on 9/8/16.
//
#include "com_example_max_ermakov_smoketunnel_SmokeNDKManager.h"
#include "smoke_singleton.h"

JNIEXPORT jstring JNICALL Java_com_example_max_1ermakov_smoketunnel_SmokeNDKManager_init
  (JNIEnv *, jobject)\
  {
            smoke_singleton* ptr = smoke_singleton::getInstance();
            ptr->run();
  }