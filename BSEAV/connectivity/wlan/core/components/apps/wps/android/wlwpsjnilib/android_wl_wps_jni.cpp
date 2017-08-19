/*
 * Broadcom WPS Enrollee
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 */
#define LOG_TAG "WLWPSClient android_wl_wps_jni.cpp"
#include <utils/Log.h>


#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <utils/threads.h>
#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include <WLWPSClient.h>

// ----------------------------------------------------------------------------
using namespace android;
// ----------------------------------------------------------------------------

struct fields_t 
{
    jfieldID    context;
    jfieldID    listener_context;
    jmethodID   post_event;
};

static fields_t fields;

static Mutex sLock;

struct callback_cookie {
    jclass      wlwpsclient_class;
    jobject     wlwpsclient_ref;
};

callback_cookie* g_cookie = 0;

static void notify(void* cookie, int msg, int ext1, String8 ext2)
{
	LOGE("JNI = message received msg=%d, ext1=%d, ext2=%s", msg, ext1, ext2.string());

	JNIEnv *env = AndroidRuntime::getJNIEnv();

	if (env == NULL)
	{
		LOGE("Callback on dead VM");
		return;
	}

	callback_cookie* c =  g_cookie ; //(callback_cookie*) cookie;  

	env->CallStaticVoidMethod(c->wlwpsclient_class, fields.post_event, c->wlwpsclient_ref, msg, ext1, 0, env->NewStringUTF(ext2.string()));

	LOGE("Callback completed");
}

static sp<WLWPSClient> getWLWPSClient(JNIEnv* env, jobject thiz)
{
	Mutex::Autolock l(sLock);
	WLWPSClient* const p = (WLWPSClient*)env->GetIntField(thiz, fields.context);
	return sp<WLWPSClient>(p);
}

static void
android_wl_jni_native_finalize(JNIEnv *env, jobject thiz)
{
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c != 0) 
	{
		// remove our strong reference created in native setup
		c->decStrong(thiz);
		env->SetIntField(thiz, fields.context, 0);

		callback_cookie *cookie = (callback_cookie *)env->GetIntField(thiz, fields.listener_context);

		if (cookie) 
		{
			env->DeleteGlobalRef(cookie->wlwpsclient_ref);
			env->DeleteGlobalRef(cookie->wlwpsclient_class);
			delete cookie;
			g_cookie = 0;
			env->SetIntField(thiz, fields.listener_context, 0);
		}
	}
} 

static void
android_wl_jni_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
	Mutex::Autolock l(sLock);

	sp<WLWPSClient> c = new WLWPSClient();
	if (c == NULL) {
		jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
		return;
	}

	if (c != 0)
	{
		sp<WLWPSClient> old = (WLWPSClient*)env->GetIntField(thiz, fields.context);
		if (c.get())
		{
			c->incStrong(thiz);
		}
		if (old != 0) 
		{
			old->decStrong(thiz);
		}
		env->SetIntField(thiz, fields.context, (int)c.get());

		callback_cookie* cookie = new callback_cookie;
		g_cookie  = cookie;

		jclass clazz = env->GetObjectClass(thiz);
		if (clazz == NULL) {
			jniThrowException(env, "java/lang/Exception", NULL);
			return;
		}
		cookie->wlwpsclient_class = (jclass)env->NewGlobalRef(clazz);

		cookie->wlwpsclient_ref = env->NewGlobalRef(weak_this);
		env->SetIntField(thiz, fields.listener_context, (int)cookie);

		c->setCallBack(notify, cookie);
	}
	return;
}

static int
android_wl_jni_native_wpsOpen(JNIEnv *env, jobject thiz)
{
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c == NULL ) 
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}
	int status;
	if (NO_ERROR == c->wpsOpen(&status))
		return status;
	return -1;
}


static int
android_wl_jni_native_wpsClose(JNIEnv *env, jobject thiz)
{
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c == NULL ) 
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}
	int status;
	if (NO_ERROR == c->wpsClose(&status))
		return status;
	return -1;
}

static int
android_wl_jni_native_wpsRefreshScanList(JNIEnv *env, jobject thiz, jstring params)
{
	int status = -1;
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c == NULL ) 
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return status;
	}

	const jchar* str = env->GetStringCritical(params, 0);
	String8 params8;
	if (params) {
		params8 = String8(str, env->GetStringLength(params));
		env->ReleaseStringCritical(params, str);
	}
	if (c->wpsRefreshScanList(params8, &status) != NO_ERROR) {
		jniThrowException(env, "java/lang/IllegalArgumentException", "android_wl_jni_native_wpsRefreshScanList failed");
		return status;
	}
	return status;
}

static int
native_wpsGetScanCount(JNIEnv *env, jobject thiz)
{
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c == NULL ) 
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}
	int count;
	if (NO_ERROR == c->wpsGetScanCount(&count))
		return count;
	return -1;
}

static jstring 
native_wpsGetScanSsid(JNIEnv *env, jobject thiz, jint iIndex)
{
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c == NULL ) 
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return 0;
	}
	LOGE("native_wpsGetScanSsid");

	String8 params8;
	if (c->wpsGetScanSsid(iIndex, params8) == NO_ERROR)
	{
		return env->NewStringUTF(params8.string());
	}
	else
	{
		return 0;
	}
}

static jstring 
native_wpsGetScanBssid(JNIEnv *env, jobject thiz, jint iIndex)
{
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c == NULL ) 
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return 0;
	}
	LOGE("native_wpsGetScaBssid");


	String8 params8;
	if (c->wpsGetScanBssid(iIndex, params8) == NO_ERROR)
	{
		return env->NewStringUTF(params8.string());
	}
	else
	{
		return 0;
	}
}

static int 
android_wl_jni_native_wpsEnroll(JNIEnv *env, jobject thiz, jstring params)
{
	int status = -1;
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c == NULL ) 
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return status;
	}

	LOGE("native_wpsEnroll");
	const jchar* str = env->GetStringCritical(params, 0);
	String8 params8;
	if (params) {
		params8 = String8(str, env->GetStringLength(params));
		env->ReleaseStringCritical(params, str);
	}
	if (c->wpsEnroll(params8, &status) != NO_ERROR) {
		jniThrowException(env, "java/lang/IllegalArgumentException", "android_wl_jni_native_wpsEnroll failed");
		return status;
	}
	return status;
}

static jstring 
native_wpsGetSsid(JNIEnv *env, jobject thiz)
{
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c == NULL ) 
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return 0;
	}

	String8 params8;
	if (c->wpsGetSsid(params8) == NO_ERROR)
	{
		return env->NewStringUTF(params8.string());
	}
	else
	{
		return 0;
	}
}

static jstring 
native_wpsGetKeyMgmt(JNIEnv *env, jobject thiz)
{
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c == NULL ) 
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return 0;
	}

	String8 params8;
	if (c->wpsGetKeyMgmt(params8) == NO_ERROR)
	{
		return env->NewStringUTF(params8.string());
	}
	else
	{
		return 0;
	}
}

static jstring 
native_wpsGetKey(JNIEnv *env, jobject thiz)
{
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c == NULL ) 
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return 0;
	}

	String8 params8;
	if (c->wpsGetKey(params8) == NO_ERROR)
	{
		return env->NewStringUTF(params8.string());
	}
	else
	{
		return 0;
	}
}

static jstring 
native_wpsGetEncryption(JNIEnv *env, jobject thiz)
{
	sp<WLWPSClient> c = getWLWPSClient(env, thiz);
	if (c == NULL ) 
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return 0;
	}

	String8 params8;
	if (c->wpsGetEncryption(params8) == NO_ERROR)
	{
		return env->NewStringUTF(params8.string());
	}
	else
	{
		return 0;
	}
}

static const char *classPathName = "com/broadcom/android/wpsguitester/IWLWPSClient";

static JNINativeMethod methods[] = {
	{"native_setup",				"(Ljava/lang/Object;)V",					(void *)android_wl_jni_native_setup},
	{"native_finalize",				"()V",										(void *)android_wl_jni_native_finalize},
	{"native_wpsOpen",				"()I",										(void *)android_wl_jni_native_wpsOpen},
	{"native_wpsClose",				"()I",										(void *)android_wl_jni_native_wpsClose},
	{"native_wpsRefreshScanList",  "(Ljava/lang/String;)I",						(void *)android_wl_jni_native_wpsRefreshScanList},
	{"native_wpsGetScanCount",		"()I",										(void *)native_wpsGetScanCount},
	{"native_wpsGetScanSsid",	    "(I)Ljava/lang/String;",					(void *)native_wpsGetScanSsid},
	{"native_wpsGetScanBssid",	    "(I)Ljava/lang/String;",					(void *)native_wpsGetScanBssid},
	{"native_wpsEnroll",			"(Ljava/lang/String;)I",					(void *)android_wl_jni_native_wpsEnroll},
	{"native_wpsGetSsid",			"()Ljava/lang/String;",						(void *)native_wpsGetSsid},
	{"native_wpsGetKeyMgmt",		"()Ljava/lang/String;",						(void *)native_wpsGetKeyMgmt},
	{"native_wpsGetKey",			"()Ljava/lang/String;",						(void *)native_wpsGetKey},
	{"native_wpsGetEncryption",	    "()Ljava/lang/String;",						(void *)native_wpsGetEncryption},
};               

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
	jclass clazz;

	clazz = env->FindClass(className);
	if (clazz == NULL) {
		LOGE("Native registration unable to find class '%s'", className);
		return JNI_FALSE;
	}

	fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
	if (fields.context == NULL) {
		LOGE("Can't find WLWPSClient.mNativeContext");
		return JNI_FALSE;
	}


	fields. listener_context = env->GetFieldID(clazz, "mListenerContext", "I");
	if (fields.listener_context == NULL) {
		LOGE("Can't find WLWPSClient.mListenerContext");
		return JNI_FALSE;
	}

	fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative",
		"(Ljava/lang/Object;IIILjava/lang/Object;)V");
	if (fields.post_event == NULL) {
		LOGE("Can't find WLWPSClient.postEventFromNative");
		return -1;
	}

	if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
		LOGE("RegisterNatives failed for '%s'", className);
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
static int registerNatives(JNIEnv* env)
{
	if (!registerNativeMethods(env, classPathName,
		methods, sizeof(methods) / sizeof(methods[0]))) {
			return JNI_FALSE;
	}

	return JNI_TRUE;
}


// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */
 
typedef union {
	JNIEnv* env;
	void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	UnionJNIEnvToVoid uenv;
	uenv.venv = NULL;
	jint result = -1;
	JNIEnv* env = NULL;

	LOGI("JNI_OnLoad");

	if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("ERROR: GetEnv failed");
		goto bail;
	}
	env = uenv.env;

	if (registerNatives(env) != JNI_TRUE) {
		LOGE("ERROR: registerNatives failed");
		goto bail;
	}

	result = JNI_VERSION_1_4;

bail:
	return result;
}
