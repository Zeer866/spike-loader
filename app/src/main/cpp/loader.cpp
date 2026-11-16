#include <jni.h>
#include <android/log.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <string>

#define TAG "SpikeLoader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static bool endsWithSo(const char* name) {
    size_t len = strlen(name);
    return len > 3 && strcmp(name + len - 3, ".so") == 0;
}

// Nombre del paquete del proceso actual (ej: com.robtopx.geometryjump)
static std::string getPackageName() {
    FILE* f = fopen("/proc/self/cmdline", "r");
    if (!f) return "";
    char buf[256] = {0};
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    std::string s(buf);
    size_t p = s.find(':');
    if (p != std::string::npos) s = s.substr(0, p);
    return s;
}

static void makeDirs(const std::string& path) {
    for (size_t i = 1; i < path.size(); i++) {
        if (path[i] == '/') mkdir(path.substr(0, i).c_str(), 0771);
    }
    mkdir(path.c_str(), 0771);
}

static bool copyFile(const std::string& from, const std::string& to) {
    FILE* in = fopen(from.c_str(), "rb");
    if (!in) return false;
    FILE* out = fopen(to.c_str(), "wb");
    if (!out) { fclose(in); return false; }
    char buf[65536];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, out);
    fclose(in);
    fclose(out);
    chmod(to.c_str(), 0755);
    return true;
}

// Copia cada .so a la carpeta interna del juego y lo carga desde ahi
static int loadMods(const std::string& modsDir, const std::string& internalDir) {
    DIR* d = opendir(modsDir.c_str());
    if (!d) { LOGE("No se pudo abrir %s", modsDir.c_str()); return 0; }

    makeDirs(internalDir);
    int count = 0;

    while (dirent* e = readdir(d)) {
        if (!endsWithSo(e->d_name)) continue;

        std::string src = modsDir + "/" + e->d_name;
        std::string dst = internalDir + "/" + e->d_name;

        if (!copyFile(src, dst)) { LOGE("No se pudo copiar %s", e->d_name); continue; }

        void* h = dlopen(dst.c_str(), RTLD_NOW);
        if (!h) { LOGE("Fallo %s: %s", e->d_name, dlerror()); continue; }

        // Si el mod exporta spike_mod_init(), se llama
        auto init = (void (*)())dlsym(h, "spike_mod_init");
        if (init) init();

        LOGI("Mod cargado: %s", e->d_name);
        count++;
    }
    closedir(d);
    return count;
}

// Se ejecuta solo cuando la libreria se carga
__attribute__((constructor))
static void onLibraryLoad() {
    std::string pkg = getPackageName();
    if (pkg.empty() || pkg == "com.spikeloader") return;

    std::string modsDir = "/storage/emulated/0/Android/media/" + pkg + "/spikeloader/mods";
    std::string internalDir = "/data/data/" + pkg + "/files/spikeloader";

    makeDirs(modsDir);
    LOGI("Spike Loader iniciado en %s", pkg.c_str());
    int n = loadMods(modsDir, internalDir);
    LOGI("%d mod(s) cargados", n);
}

// Lo usa el launcher para contar los mods de la carpeta
extern "C" JNIEXPORT jint JNICALL
Java_com_spikeloader_MainActivity_nativeCountMods(JNIEnv* env, jobject, jstring jdir) {
    const char* dir = env->GetStringUTFChars(jdir, nullptr);
    int count = 0;
    if (DIR* d = opendir(dir)) {
        while (dirent* e = readdir(d)) if (endsWithSo(e->d_name)) count++;
        closedir(d);
    }
    env->ReleaseStringUTFChars(jdir, dir);
    return count;
}
