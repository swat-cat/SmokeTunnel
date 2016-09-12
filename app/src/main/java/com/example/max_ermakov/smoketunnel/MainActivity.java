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


        final SmokeNDKManager smokeNDKManager = new SmokeNDKManager();

        new Thread(new Runnable() {
            @Override
            public void run() {
                String log = smokeNDKManager.init();
                Log.i("SMOKER: ", log);
            }
        }).start();
    }
}
