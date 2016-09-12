package com.example.max_ermakov.smoketunnel;

import android.util.Log;

import com.getkeepsafe.relinker.ReLinker;

/**
 * Created by max_ermakov on 9/7/16.
 */
public class SmokeNDKManager {

    public SmokeNDKManager() {
        ReLinker.Logger logcatLogger = new ReLinker.Logger() {
            @Override
            public void log(String message) {
                Log.d("ReLinker", message);
            }
        };

        ReLinker.log(logcatLogger)
                .force()
                .recursively()
                .loadLibrary(App.getInstance().getApplicationContext(), "smoker", new ReLinker.LoadListener() {
                            @Override
                            public void success() {
                                /* Yay */
                            }

                            @Override
                            public void failure(Throwable t) {
                                /* Boo */
                                t.printStackTrace();
                            }
                        }
                );
    }

    public native String init();
}
