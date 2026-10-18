#include <jni.h>
#include <android/log.h>
#include <string>
#include <vector>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#define LOG_TAG "TAML"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static JavaVM* g_vm = nullptr;
static jobject g_activity = nullptr;
static jobject g_context = nullptr;

static int lua_log(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    LOGI("Lua: %s", msg);
    return 0;
}

static jobject findCurrentActivity(JNIEnv* env) {
    jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
    if (!activityThreadClass) return nullptr;

    jmethodID currentActivityThread = env->GetStaticMethodID(
        activityThreadClass, "currentActivityThread",
        "()Landroid/app/ActivityThread;");
    if (!currentActivityThread) return nullptr;

    jobject at = env->CallStaticObjectMethod(activityThreadClass, currentActivityThread);
    if (!at) return nullptr;

    jfieldID activitiesField = env->GetFieldID(
        activityThreadClass, "mActivities", "Landroid/util/ArrayMap;");
    if (!activitiesField) return nullptr;

    jobject activities = env->GetObjectField(at, activitiesField);
    if (!activities) return nullptr;

    jclass arrayMapClass = env->FindClass("android/util/ArrayMap");
    jmethodID sizeMethod = env->GetMethodID(arrayMapClass, "size", "()I");
    jmethodID valueAtMethod = env->GetMethodID(arrayMapClass, "valueAt", "(I)Ljava/lang/Object;");

    jint size = env->CallIntMethod(activities, sizeMethod);
    jobject activity = nullptr;

    for (jint i = 0; i < size; i++) {
        jobject activityRecord = env->CallObjectMethod(activities, valueAtMethod, i);
        if (!activityRecord) continue;

        jclass recordClass = env->GetObjectClass(activityRecord);
        jfieldID activityField = env->GetFieldID(recordClass, "activity", "Landroid/app/Activity;");
        if (activityField) {
            jobject act = env->GetObjectField(activityRecord, activityField);
            if (act) {
                activity = act;
                break;
            }
        }
    }
    return activity;
}

extern "C" void callJavaHi(JNIEnv* env, jobject activity) {
    jclass cls = env->FindClass("com/modloader/loader/MOD");
    if (!cls) return;
    jmethodID mid = env->GetStaticMethodID(cls, "sayHi", "(Landroid/app/Activity;)V");
    if (mid && activity) env->CallStaticVoidMethod(cls, mid, activity);
}

static void loadMods(JNIEnv* env) {
    jclass cls = env->FindClass("com/modloader/loader/MOD");
    if (!cls) { LOGI("MOD class not found"); return; }

    jmethodID listModsMid = env->GetStaticMethodID(cls, "listMods", "(Landroid/content/Context;)[Ljava/lang/String;");
    jmethodID sortModsMid = env->GetStaticMethodID(cls, "sortModsByDependencies", "(Landroid/content/Context;[Ljava/lang/String;)[Ljava/lang/String;");
    jmethodID readMainLuaMid = env->GetStaticMethodID(cls, "readMainLua", "(Landroid/content/Context;Ljava/lang/String;)Ljava/lang/String;");

    if (!listModsMid || !sortModsMid || !readMainLuaMid) return;

    jobjectArray modsArray = (jobjectArray) env->CallStaticObjectMethod(cls, listModsMid, g_context);
    if (!modsArray) return;

    jsize modCount = env->GetArrayLength(modsArray);
    LOGI("Found %d mods", modCount);
    if (modCount == 0) return;

    jobjectArray sortedArray = (jobjectArray) env->CallStaticObjectMethod(cls, sortModsMid, g_context, modsArray);
    if (!sortedArray) sortedArray = modsArray;

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lua_register(L, "log", lua_log);

    jsize sortedCount = env->GetArrayLength(sortedArray);
    for (jsize i = 0; i < sortedCount; i++) {
        jstring modName = (jstring) env->GetObjectArrayElement(sortedArray, i);
        const char* modNameStr = env->GetStringUTFChars(modName, nullptr);
        LOGI("Loading mod: %s", modNameStr);

        jstring luaCode = (jstring) env->CallStaticObjectMethod(cls, readMainLuaMid, g_context, modName);
        if (luaCode) {
            const char* code = env->GetStringUTFChars(luaCode, nullptr);
            if (luaL_dostring(L, code) != LUA_OK) {
                LOGI("Lua error in mod %s: %s", modNameStr, lua_tostring(L, -1));
                lua_pop(L, 1);
            } else {
                LOGI("Mod %s loaded successfully", modNameStr);
            }
            env->ReleaseStringUTFChars(luaCode, code);
        }
        env->ReleaseStringUTFChars(modName, modNameStr);
    }
    lua_close(L);
}

static void nativeLoadMods(JNIEnv* env, jclass clazz) {
    if (g_context) {
        loadMods(env);
    }
}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("Library loaded successfully.");
    g_vm = vm;

    JNIEnv* env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) return JNI_VERSION_1_6;

    jclass modClass = env->FindClass("com/modloader/loader/MOD");
    if (modClass) {
        JNINativeMethod methods[] = {
            {"nativeLoadMods", "()V", (void*)nativeLoadMods}
        };
        env->RegisterNatives(modClass, methods, 1);
    }

    jobject activity = findCurrentActivity(env);
    if (activity) {
        g_activity = env->NewGlobalRef(activity);
        g_context = env->NewGlobalRef(activity);
        LOGI("Activity found and saved.");
    } else {
        LOGI("Activity not found (maybe too early).");
    }

    if (g_activity) callJavaHi(env, g_activity);

    return JNI_VERSION_1_6;
}