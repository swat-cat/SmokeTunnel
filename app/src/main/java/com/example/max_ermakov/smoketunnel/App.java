package com.example.max_ermakov.smoketunnel;

import android.app.Application;
import android.content.Context;

/**
 * Created by max_ermakov on 9/8/16.
 */
public class App extends Application {
    private static App sInstance;
    private static Context context;

    public static App getInstance() {
        return sInstance;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        sInstance = this;
        App.context = getApplicationContext();
    }
}
