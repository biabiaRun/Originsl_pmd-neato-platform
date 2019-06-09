package com.pmdtec.royaleviewer.level2;


import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.Toast;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        PackageManager manager = this.getPackageManager();
        Intent i = manager.getLaunchIntentForPackage("org.pmdtec.qtviewer");
        if (i == null) {
            Toast.makeText(this, "Royale Viewer not found!", Toast.LENGTH_SHORT).show();
            return;
        }
        i.putExtra("activationCode", "${ROYALE_ACCESS_CODE_LEVEL2}");
        i.addCategory(Intent.CATEGORY_LAUNCHER);
        this.startActivity(i);
        finish ();
    }
}
