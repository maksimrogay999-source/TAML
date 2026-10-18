#include <jni.h>
#include <android/log.h>
#include <string>

// Подключаем LuaJIT
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#define LOG_TAG "TAML"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static int lua_log(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    LOGI("Lua: %s", msg);
    return 0;
}

extern "C" void callJavaHi(JNIEnv* env) {
    jclass cls = env->FindClass("com/modloader/loader/MOD");
    if (!cls) return;
    jmethodID mid = env->GetStaticMethodID(cls, "sayHi", "()V");
    if (mid) env->CallStaticVoidMethod(cls, mid);
}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("Library loaded successfully.");

    lua_State* L = luaL_newstate();
    if (!L) {
        LOGI("Error: failed to create Lua state");
        return JNI_VERSION_1_6;
    }
    luaL_openlibs(L);

    lua_register(L, "log", lua_log);

    const char* script = "log('Lua from TAML is working!')";
    if (luaL_dostring(L, script) != LUA_OK) {
        LOGI("Lua error: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    lua_close(L);
    JNIEnv* env = nullptr;
    jint result = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    callJavaHi(env);

    return JNI_VERSION_1_6;
}