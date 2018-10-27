package com.hyman.audiodemo;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;


public class MainActivity extends AppCompatActivity {

    private Spinner mTestSpinner;
    private NativeAudio nativeAudio;

    private static String[] mRequestPermissions = new String[]{
            Manifest.permission.RECORD_AUDIO,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE
    };

    public static final String[] TEST_PROGRAM_ARRAY = {
//            "录制 wav 文件",
//            "播放 wav 文件",
            "OpenSL ES 录制",
            "OpenSL ES 播放",
//            "音频编解码"
    };

    public void requestAllPower() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            List<String> needPermissions = new ArrayList<>();

            //判断是否已经赋予权限
            for (int i = 0; i < mRequestPermissions.length; i++){
                if (ContextCompat.checkSelfPermission(this, mRequestPermissions[i])
                        != PackageManager.PERMISSION_GRANTED) {
                    //如果应用之前请求过此权限但用户拒绝了请求，此方法将返回 true。
                    if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                            mRequestPermissions[i])) {
                    } else {
                        needPermissions.add(mRequestPermissions[i]);
                    }
                }
            }

            if (needPermissions.size() > 0) {
                String[] permissions = needPermissions.toArray(new String[needPermissions.size()]);
                //申请权限，字符串数组内是一个或多个要申请的权限，1是申请权限结果的返回参数，在onRequestPermissionsResult可以得知申请结果
                ActivityCompat.requestPermissions(this, permissions, 1);
            }

        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTestSpinner = (Spinner) findViewById(R.id.TestSpinner);
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, TEST_PROGRAM_ARRAY);
        mTestSpinner.setAdapter(adapter);

        requestAllPower();
    }

    public void onClickStartTest(View v) {
        switch (mTestSpinner.getSelectedItemPosition()) {
            case 0:
                nativeAudio = new NativeAudio(true);
                break;
            case 1:
                nativeAudio = new NativeAudio(false);
                break;
            default:
                break;
        }
        if (nativeAudio != null) {
            nativeAudio.startTesting();
            Toast.makeText(this, "Start Testing !", Toast.LENGTH_SHORT).show();
        }
    }

    public void onClickStopTest(View v) {
        if (nativeAudio != null) {
            nativeAudio.stopTesting();
            Toast.makeText(this, "Stop Testing !", Toast.LENGTH_SHORT).show();
        }
    }
}
