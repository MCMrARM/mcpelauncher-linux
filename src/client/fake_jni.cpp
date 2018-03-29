#include "fake_jni.h"

FakeJNI FakeJNI::instance;

FakeJNI::FakeJNI() {
    interface.DestroyJavaVM = [](JavaVM*) {
        return -1;
    };
    interface.AttachCurrentThread = [](JavaVM* vm, JNIEnv** env, void* arg) {
        return 0;
    };
    interface.DetachCurrentThread = [](JavaVM* vm) {
        return 0;
    };
    interface.GetEnv = [](JavaVM* vm, JNIEnv** env, int ver) {
        return 0;
    };
    jvm = &interface;
}