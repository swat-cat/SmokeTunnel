package com.example.max_ermakov.smoketunnel;

/**
 * Created by max_ermakov on 9/7/16.
 */
public class SmokeNDKManager {
    static {
        System.loadLibrary("libuv");
        System.loadLibrary("smoke");
    }

//    public void native init();
}
