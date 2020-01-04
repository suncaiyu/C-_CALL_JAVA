#include "jni.h"
#include "jni_md.h"
#include <iostream>
#include <tchar.h>
#include "windows.h"

using namespace std;

bool startJVM();

// jstringתchar*
char* jstringToWindows(JNIEnv *env, jstring jstr)
{
    int length = env->GetStringLength(jstr);
    const jchar* jcstr = env->GetStringChars(jstr, 0);
    char* rtn = (char*)malloc(length * 2 + 1);
    int size = 0;
    size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)jcstr, length, rtn, (length * 2 + 1), NULL, NULL);
    if (size <= 0)
        return NULL;
    env->ReleaseStringChars(jstr, jcstr);
    rtn[size] = 0;
    return rtn;
}

// char *תjstring
jstring WindowsTojstring(JNIEnv *env, const char* str)
{
    jstring rtn = 0;
    int slen = strlen(str);
    unsigned short* buffer = 0;
    if (slen == 0)
        rtn = env->NewStringUTF(str);
    else
    {
        int length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str, slen, NULL, 0);
        buffer = (unsigned short*)malloc(length * 2 + 1);
        if (MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str, slen, (LPWSTR)buffer, length) > 0)
            rtn = env->NewString((jchar*)buffer, length);
    }
    if (buffer)
        free(buffer);
    return rtn;
}

int _tmain(int argc, _TCHAR* argv[])
{
    cout << "haha" << endl;
    startJVM();
    return 0;
}

typedef jint(JNICALL *JNICREATEPROC)(JavaVM **, void **, void *);
//���������
//bool setStream(JNIEnv *env, const char* fileName, const char* method);

//����java�����

bool startJVM() {
    //��ȡjvm��̬���·��
    TCHAR* jvmPath = _T((char *)"G://jre1.8.0_231//bin//server//jvm.dll");

    //java���������ʱ���յĲ�����ÿ����������һ��
    int nOptionCount = 2;
    JavaVMOption vmOption[2];
    //����JVM����������Ķ��ڴ棬�������
    vmOption[0].optionString = (char *)"-Xmx256M";
    //����classpath
    //vmOption[1].optionString = (char *)"-Djava.class.path=./Demo.jar";
    vmOption[1].optionString = (char *)"-Djava.class.path=./HelloWorld.jar";


    JavaVMInitArgs vmInitArgs;
    vmInitArgs.version = JNI_VERSION_1_8;
    vmInitArgs.options = vmOption;
    vmInitArgs.nOptions = nOptionCount;
    //�����޷�ʶ��jvm�����
    vmInitArgs.ignoreUnrecognized = JNI_TRUE;

    //���������࣬ע��ָ���Ϊ"/"
    const char startClass[] = "test/HelloWorld";
    //const char startClass[] = "demo/WinFile";

    //����������һ����main��������Ȼ�������ó���������
    const char startMethod[] = "sayHello";
    //const char startMethod[] = "sayHello";

    //����JVM,ע����Ҫ������ַ���ΪLPCWSTR,ָ��һ������Unicode�ַ�����32λָ�룬�൱��const wchar_t*
    HINSTANCE jvmDLL = LoadLibrary(jvmPath);
    if (jvmDLL == NULL) {
        cout << "����JVM��̬�����" + ::GetLastError() << endl;
        return false;
    }

    //��ʼ��jvm�����ַ
    JNICREATEPROC jvmProcAddress = (JNICREATEPROC)GetProcAddress(jvmDLL, "JNI_CreateJavaVM");
    if (jvmDLL == NULL) {
        FreeLibrary(jvmDLL);
        cout << "����JVM��̬�����" + ::GetLastError() << endl;
        return false;
    }

    //����JVM
    JNIEnv *env;
    JavaVM *jvm;
    jint jvmProc = (jvmProcAddress)(&jvm, (void **)&env, &vmInitArgs);
    if (jvmProc < 0 || jvm == NULL || env == NULL) {
        FreeLibrary(jvmDLL);
        cout << "����JVM����" + ::GetLastError() << endl;
        return false;
    }

    //����������
    jclass mainclass = env->FindClass(startClass);
    if (env->ExceptionCheck() == JNI_TRUE || mainclass == NULL) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        FreeLibrary(jvmDLL);
        cout << "����������ʧ��" << endl;
        return false;
    }

    
    //jclass global_class = (jclass)env->NewGlobalRef(mainclass);
    //������������
    jmethodID methedID = env->GetMethodID(mainclass, startMethod, "()V");
    //jmethodID methedID = env->GetStaticMethodID(mainclass, startMethod, "(Ljava/lang/String;)Ljava/lang/String;");
    if (env->ExceptionCheck() == JNI_TRUE || methedID == NULL) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        FreeLibrary(jvmDLL);
        cout << "������������ʧ��" << endl;
        return false;
    }

    cout << "��ʼִ��" << endl;
    std::string strNew = "��������C++��string��";
    jstring strr = WindowsTojstring(env, strNew.c_str());
    //ʵ��������
    jobject jobject = env->AllocObject(mainclass);//������ Java ����������øö�����κι��캯�������ظö��������  ��������Ŀ��ܻ��õ�NewObject
    env->CallVoidMethod(jobject, methedID, NULL);
    //jstring strResult = (jstring)env->CallStaticObjectMethod(mainclass, methedID, strr);
    //char* pBuf = new char[1024];
    //memset(pBuf, 0, 1024);
    //char *q = pBuf;
    //q = jstringToWindows(env,strResult);
    //cout << q << endl;
    cout << "ִ�н���" << endl;

    //jvm�ͷ�
    jvm->DestroyJavaVM();
    FreeLibrary(jvmDLL);
    return true;
}
// ���谴��������  ע�Ͷ�д����  ע���ҵ�java
//�ܽ� ����ʵ�������������
//1.ֱ�ӵ���main���� java  main��static�ģ�
//(1)ֱ�Ӽ�����������GetStaticMethodID(mainclass, startMethod, "(Ljava/lang/String;)Ljava/lang/String;");
// (2)env->CallStaticObjectMethod(mainclass, methedID, NULL);  ע����Ϊjava����ֵ��һ��CallStaticObjectMethod  CallStaticVoidMethod�ȿ���ѡ��ʹ��
//2.���ô�������static��������ʵ������һ��
// (1) ͬ�ϣ�Ψһ��������ǵ�������������Ϊnull����java��Ҫ�Ĳ�����ͬʱjstring����ʱע�⻥תjava c++
//3.�������Ա����  ����һ��  ��ʱû�㶮ɶ��˼
// (1)����������������һ����
// (2)��Ϊ�ǳ�Ա������Ҫ��ʵ����һ���������  jobject jobject = env->AllocObject(mainclass);//������ Java ����������øö�����κι��캯�������ظö��������  ��������Ŀ��ܻ��õ�NewObject
// (3) env->CallVoidMethod(jobject, methedID, NULL);Ȼ�����������ȥ��������
// (4)������Ӧ��Ҳ��࣬NULL���ɲ���

// JNI���ǱȽϸ��ӵ�  ����Ҫ�Ŀ��Զ࿴��

// ���Ĳο���https://blog.csdn.net/sf0407/article/details/53924174
//           https://blog.csdn.net/toyauko/article/details/83109325


// C++��java������ˣ���java��װ��jar����(���Ը���class��������˲鿴��ʹ��),rȻ����c++������jvm����������÷��������������ǱȽϸ��ӵ�
// java��c++��Լ���࣬c++�����dll/so�⣬java���أ�ʹ�þͿ����ˣ�������Ҫע��c++���������֣�Ҫ����jni�淶��Ҫ��ע��java���õط��İ���������
// ������ǰ��java��c++�Ĳο�(ffmpeg��java�е��ã���ʱ����ffmpeg.so�ɳ����������ƹ�˾û������������linuxҲû�У�������windows����eclipse����ndk�����so)
// ��ʱʹ��ffmpegҲ�Ƚ��������ffmpeg.cpp�е�һ����ȡ�����в����ķ���Ϊmain������Ȼ��java���������������
//https://www.cnblogs.com/dengpeng1004/p/8745186.html