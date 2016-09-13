package com.example.max_ermakov.smoketunnel;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import com.getkeepsafe.relinker.ReLinker;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ReLinker.Logger logcatLogger = new ReLinker.Logger() {
            @Override
            public void log(String message) {
                Log.d("ReLinker", message);
            }
        };

        ReLinker.log(logcatLogger)
                .force()
                .recursively()
                .loadLibrary(MainActivity.this, "smoker", new ReLinker.LoadListener() {
                            @Override
                            public void success() {
                                /* Yay */
                                new Thread(new Runnable() {
                                    @Override
                                    public void run() {
                                        String log = init();
                                        Log.i("SMOKER: ", log);
                                    }
                                }).start();
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
