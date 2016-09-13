package com.example.max_ermakov.smoketunnel;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import com.getkeepsafe.relinker.ReLinker;

public class MainActivity extends Activity {

    final SmokeNDKManager smokeNDKManager = new SmokeNDKManager();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        new Thread(new Runnable() {
            @Override
            public void run() {
                String log = smokeNDKManager.init();
                Log.i("SMOKER: ", log);
            }
        }).start();
    }

    @Override
    protected void onPause() {
        super.onPause();
//        smokeNDKManager.close();
    }
}
