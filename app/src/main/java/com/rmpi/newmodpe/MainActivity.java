package com.rmpi.newmodpe;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {
    private static final int PERMISSION_GRANT_CODE = 1;
    private static final String mcpeDataDir = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "games" + File.separator + "com.mojang" + File.separator;
    private static final String newDex = "NewModPEDex.dex";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                    || ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                    || ActivityCompat.checkSelfPermission(this, "net.zhuoweizhang.mcpelauncher.ADDON") != PackageManager.PERMISSION_GRANTED)
                ActivityCompat.requestPermissions(this, new String[] {
                    Manifest.permission.READ_EXTERNAL_STORAGE
                        , Manifest.permission.WRITE_EXTERNAL_STORAGE
                        , "net.zhuoweizhang.mcpelauncher.ADDON"
                }, PERMISSION_GRANT_CODE);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSION_GRANT_CODE)
            if (grantResults.length <= 0 || grantResults.length != permissions.length)
                finish();
            else for (int grantResult : grantResults)
                if (grantResult != PackageManager.PERMISSION_GRANTED)
                    finish();
    }

    public void dexCopyOnClick(View v) {
        try {
            File dexPath = new File(mcpeDataDir + File.separator + newDex);
            dexPath.getParentFile().mkdirs();
            InputStream dexIs = getAssets().open(newDex);
            FileOutputStream dexOs = new FileOutputStream(dexPath);
            byte[] chunk = new byte[1024];
            int read;
            while ((read = dexIs.read(chunk)) != -1)
                dexOs.write(chunk, 0, read);
            dexOs.getFD().sync();
            dexOs.close();
            dexIs.close();
            Toast.makeText(this, "Dex copying success", Toast.LENGTH_LONG).show();
        } catch (IOException e) {
            Toast.makeText(this, "Error during copy task", Toast.LENGTH_LONG).show();
        }
    }
}
