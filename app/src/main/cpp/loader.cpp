#include <jni.h>
#include <android/log.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string>
#include <cstring>

#define TAG "SpikeLoader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static int loadMods(const std::string& dir) {
    DIR* d = opendir(dir.c_str());
    if (!d) { LOGE("No se pudo abrir %s", dir.c_str()); return 0; }

    int count = 0;
    while (dirent* e = readdir(d)) {
        size_t len = strlen(e->d_name);
        if (len < 4 || strcmp(e->d_name + len - 3, ".so") != 0) continue;

        std::string path = dir + "/" + e->d_name;
        void* h = dlopen(path.c_str(), RTLD_NOW);
        if (!h) { LOGE("Fallo %s: %s", path.c_str(), dlerror()); continue; }

        LOGI("Mod cargado: %s", e->d_name);
        count++;
    }
    closedir(d);
    return count;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_spikeloader_MainActivity_nativeCountMods(JNIEnv* env, jobject, jstring jdir) {
    const char* dir = env->GetStringUTFChars(jdir, nullptr);
    int n = loadMods(dir);
    env->ReleaseStringUTFChars(jdir, dir);
    return n;
}
