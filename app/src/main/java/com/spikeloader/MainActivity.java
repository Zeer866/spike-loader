package com.spikeloader;

import android.app.Activity;
import android.graphics.Typeface;
import android.os.Bundle;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends Activity {

    static {
        System.loadLibrary("spikeloader");
    }

    private native int nativeCountMods(String dir);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        File[] media = getExternalMediaDirs();
        File modsDir = new File(media[0], "mods");
        if (!modsDir.exists()) modsDir.mkdirs();

        List<String> names = new ArrayList<>();
        File[] files = modsDir.listFiles();
        if (files != null) {
            for (File f : files) {
                if (f.getName().endsWith(".so")) names.add(f.getName());
            }
        }

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(32, 64, 32, 32);

        TextView title = new TextView(this);
        title.setText("Spike Loader");
        title.setTextSize(28);
        title.setTypeface(Typeface.DEFAULT_BOLD);
        root.addView(title);

        TextView info = new TextView(this);
        info.setText("Carpeta de mods:\n" + modsDir.getAbsolutePath()
                + "\n\nMods detectados: " + nativeCountMods(modsDir.getAbsolutePath()));
        info.setPadding(0, 16, 0, 24);
        root.addView(info);

        ListView list = new ListView(this);
        if (names.isEmpty()) {
            names.add("(vacío, pon tus mods .so en la carpeta)");
        }
        list.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, names));
        root.addView(list);

        setContentView(root);
    }
          }
