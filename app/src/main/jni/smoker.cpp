//
// Created by Maxim Ermakov on 9/8/16.
//
#include "com_example_max_ermakov_smoketunnel_SmokeNDKManager.h"
#include "smoke.h"

JNIEXPORT jstring JNICALL Java_com_example_max_1ermakov_smoketunnel_SmokeNDKManager_init
  (JNIEnv *env, jobject)
  {
            smoke* ptr = new smoke();
            ptr->request_big_packets = 0;
            ptr->local_port = 4987;
            ptr->remote_host_index = 0;
            ptr->handshake_timeout = 5000;
            ptr->connection_timeout = 10000;
            ptr->hop_timeout = 5000;
            ptr->hop_interval = 30 * 60 * 1000;
            ptr->hop_attempts = 10;
            ptr->remote_hosts = {std::make_pair("45.32.181.159",8080)};

            ptr->run();

            return env->NewStringUTF("Lets smoke!");
  }