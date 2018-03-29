#pragma once

struct JNIEnv;
struct JNIInvokeInterface;
typedef const struct JNIInvokeInterface* JavaVM;

struct JNIInvokeInterface {
    void* reserved[3];

    int (*DestroyJavaVM)(JavaVM* vm);
    int (*AttachCurrentThread)(JavaVM* vm, JNIEnv** env, void* arg);
    int (*DetachCurrentThread)(JavaVM* vm);
    int (*GetEnv)(JavaVM* vm, JNIEnv** env, int ver);
};

struct FakeJNI {

private:
    JNIInvokeInterface interface;
    JavaVM jvm;

public:
    static FakeJNI instance;

    FakeJNI();

    JavaVM* getVM() {
        return &jvm;
    }

};