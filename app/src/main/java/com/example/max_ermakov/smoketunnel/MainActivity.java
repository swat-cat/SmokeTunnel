package com.example.max_ermakov.smoketunnel;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

import com.getkeepsafe.relinker.ReLinker;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        final SmokeNDKManager smokeNDKManager = new SmokeNDKManager();

        ReLinker.Logger logcatLogger = new ReLinker.Logger() {
            @Override
            public void log(String message) {
                Log.d("ReLinker", message);
            }
        };

        ReLinker.log(logcatLogger)
                .force()
                .recursively()
                .loadLibrary(MainActivity.this, "smoker");

        new Thread(new Runnable() {
            @Override
            public void run() {
                smokeNDKManager.init();
            }
        }).start();
    }
}
