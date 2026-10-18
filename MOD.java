package com.modloader.loader;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import androidx.documentfile.provider.DocumentFile;

import org.json.JSONArray;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class MOD {
    private static final String TAG = "TAML";
    private static final String PREFS_NAME = "modloader_prefs";
    private static final String KEY_SAVED_URI = "saved_saf_uri";
    static final int REQ_SAF = 1001;

    private static Activity currentActivity;

    // Нативный метод для загрузки модов
    public static native void nativeLoadMods();

    public static void sayHi(Activity activity) {
        Log.i(TAG, "Hi from java");
        if (activity == null) return;
        currentActivity = activity;

        String saved = activity.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
                .getString(KEY_SAVED_URI, null);
        if (saved != null) {
            Log.i(TAG, "URI already saved: " + saved);
            nativeLoadMods();
            return;
        }

        // Запуск SAF через 5 секунд у меня есть теория что callback не вызывается из-за того что unity не инцилизирован полностью
        new android.os.Handler(android.os.Looper.getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                try {
                    Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
                    intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
                            | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
                            | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION
                            | Intent.FLAG_GRANT_PREFIX_URI_PERMISSION);
                    currentActivity.startActivityForResult(intent, REQ_SAF);
                } catch (Throwable t) {
                    Log.e(TAG, "sayHi: failed to launch SAF", t);
                }
            }
        }, 5000);
    }

    public static boolean onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode != REQ_SAF) return false;
        Activity activity = currentActivity;
        if (activity == null) return true;

        if (resultCode == Activity.RESULT_OK && data != null && data.getData() != null) {
            Uri uri = data.getData();
            try {
                activity.getContentResolver().takePersistableUriPermission(
                        uri,
                        Intent.FLAG_GRANT_READ_URI_PERMISSION
                                | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                activity.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
                        .edit().putString(KEY_SAVED_URI, uri.toString()).apply();
                Log.i(TAG, "SAF tree URI saved: " + uri);
                
                nativeLoadMods(); 
                
            } catch (SecurityException e) {
                Log.e(TAG, "takePersistableUriPermission failed", e);
            }
        } else {
            Log.w(TAG, "SAF cancelled or failed");
        }
        return true;
    }

    public static Uri getSavedUri(Context context) {
        String s = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
                .getString(KEY_SAVED_URI, null);
        return s == null ? null : Uri.parse(s);
    }

    public static String[] listMods(Context context) {
        Uri treeUri = getSavedUri(context);
        if (treeUri == null) {
            Log.e(TAG, "listMods: no saved URI");
            return new String[0];
        }

        DocumentFile root = DocumentFile.fromTreeUri(context, treeUri);
        if (root == null || !root.isDirectory()) {
            Log.e(TAG, "listMods: root not a directory");
            return new String[0];
        }

        List<String> mods = new ArrayList<>();
        for (DocumentFile dir : root.listFiles()) {
            if (dir.isDirectory() && dir.findFile("main.lua") != null) {
                mods.add(dir.getName());
            }
        }
        Log.i(TAG, "Found " + mods.size() + " mods");
        return mods.toArray(new String[0]);
    }

    public static String readLoadJson(Context context, String modName) {
        Uri treeUri = getSavedUri(context);
        if (treeUri == null) return null;

        DocumentFile root = DocumentFile.fromTreeUri(context, treeUri);
        if (root == null) return null;

        DocumentFile modDir = root.findFile(modName);
        if (modDir == null || !modDir.isDirectory()) return null;

        DocumentFile loadJson = modDir.findFile("load.json");
        if (loadJson == null) return null;

        return readFile(context, loadJson.getUri());
    }

    public static String readMainLua(Context context, String modName) {
        Uri treeUri = getSavedUri(context);
        if (treeUri == null) return null;

        DocumentFile root = DocumentFile.fromTreeUri(context, treeUri);
        if (root == null) return null;

        DocumentFile modDir = root.findFile(modName);
        if (modDir == null) return null;

        DocumentFile mainLua = modDir.findFile("main.lua");
        if (mainLua == null) return null;

        return readFile(context, mainLua.getUri());
    }

    private static String readFile(Context context, Uri uri) {
        try {
            InputStream is = context.getContentResolver().openInputStream(uri);
            BufferedReader reader = new BufferedReader(new InputStreamReader(is));
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line).append("\n");
            }
            reader.close();
            return sb.toString();
        } catch (Exception e) {
            Log.e(TAG, "readFile failed: " + uri, e);
            return null;
        }
    }

    public static String[] sortModsByDependencies(Context context, String[] mods) {
        Map<String, List<String>> deps = new HashMap<>();
        for (String mod : mods) {
            List<String> list = new ArrayList<>();
            String json = readLoadJson(context, mod);
            if (json != null) {
                try {
                    JSONArray arr = new JSONArray(json);
                    for (int i = 0; i < arr.length(); i++) {
                        list.add(arr.getString(i));
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Failed to parse load.json for " + mod, e);
                }
            }
            deps.put(mod, list);
        }

        List<String> sorted = new ArrayList<>();
        Set<String> visited = new HashSet<>();
        Set<String> visiting = new HashSet<>();

        for (String mod : mods) {
            visit(mod, deps, visited, visiting, sorted);
        }
        return sorted.toArray(new String[0]);
    }

    private static void visit(String mod, Map<String, List<String>> deps,
                              Set<String> visited, Set<String> visiting, List<String> sorted) {
        if (visited.contains(mod)) return;
        if (visiting.contains(mod)) {
            Log.w(TAG, "Circular dependency detected: " + mod);
            return;
        }
        visiting.add(mod);
        for (String dep : deps.getOrDefault(mod, new ArrayList<>())) {
            if (deps.containsKey(dep)) {
                visit(dep, deps, visited, visiting, sorted);
            } else {
                Log.w(TAG, "Missing dependency: " + dep + " for mod " + mod);
            }
        }
        visiting.remove(mod);
        visited.add(mod);
        sorted.add(mod);
    }
}