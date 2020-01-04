#include "jni.h"
#include "jni_md.h"
#include <iostream>
#include <tchar.h>
#include "windows.h"

using namespace std;

bool startJVM();

// jstring转char*
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

// char *转jstring
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
//设置输出流
//bool setStream(JNIEnv *env, const char* fileName, const char* method);

//启动java虚拟机

bool startJVM() {
    //获取jvm动态库的路径
    TCHAR* jvmPath = _T((char *)"G://jre1.8.0_231//bin//server//jvm.dll");

    //java虚拟机启动时接收的参数，每个参数单独一项
    int nOptionCount = 2;
    JavaVMOption vmOption[2];
    //设置JVM最大允许分配的堆内存，按需分配
    vmOption[0].optionString = (char *)"-Xmx256M";
    //设置classpath
    //vmOption[1].optionString = (char *)"-Djava.class.path=./Demo.jar";
    vmOption[1].optionString = (char *)"-Djava.class.path=./HelloWorld.jar";


    JavaVMInitArgs vmInitArgs;
    vmInitArgs.version = JNI_VERSION_1_8;
    vmInitArgs.options = vmOption;
    vmInitArgs.nOptions = nOptionCount;
    //忽略无法识别jvm的情况
    vmInitArgs.ignoreUnrecognized = JNI_TRUE;

    //设置启动类，注意分隔符为"/"
    const char startClass[] = "test/HelloWorld";
    //const char startClass[] = "demo/WinFile";

    //启动方法，一般是main函数，当然可以设置成其他函数
    const char startMethod[] = "sayHello";
    //const char startMethod[] = "sayHello";

    //加载JVM,注意需要传入的字符串为LPCWSTR,指向一个常量Unicode字符串的32位指针，相当于const wchar_t*
    HINSTANCE jvmDLL = LoadLibrary(jvmPath);
    if (jvmDLL == NULL) {
        cout << "加载JVM动态库错误" + ::GetLastError() << endl;
        return false;
    }

    //初始化jvm物理地址
    JNICREATEPROC jvmProcAddress = (JNICREATEPROC)GetProcAddress(jvmDLL, "JNI_CreateJavaVM");
    if (jvmDLL == NULL) {
        FreeLibrary(jvmDLL);
        cout << "加载JVM动态库错误" + ::GetLastError() << endl;
        return false;
    }

    //创建JVM
    JNIEnv *env;
    JavaVM *jvm;
    jint jvmProc = (jvmProcAddress)(&jvm, (void **)&env, &vmInitArgs);
    if (jvmProc < 0 || jvm == NULL || env == NULL) {
        FreeLibrary(jvmDLL);
        cout << "创建JVM错误" + ::GetLastError() << endl;
        return false;
    }

    //加载启动类
    jclass mainclass = env->FindClass(startClass);
    if (env->ExceptionCheck() == JNI_TRUE || mainclass == NULL) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        FreeLibrary(jvmDLL);
        cout << "加载启动类失败" << endl;
        return false;
    }

    
    //jclass global_class = (jclass)env->NewGlobalRef(mainclass);
    //加载启动方法
    jmethodID methedID = env->GetMethodID(mainclass, startMethod, "()V");
    //jmethodID methedID = env->GetStaticMethodID(mainclass, startMethod, "(Ljava/lang/String;)Ljava/lang/String;");
    if (env->ExceptionCheck() == JNI_TRUE || methedID == NULL) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        FreeLibrary(jvmDLL);
        cout << "加载启动方法失败" << endl;
        return false;
    }

    cout << "开始执行" << endl;
    std::string strNew = "我是来自C++的string！";
    jstring strr = WindowsTojstring(env, strNew.c_str());
    //实例化该类
    jobject jobject = env->AllocObject(mainclass);//分配新 Java 对象而不调用该对象的任何构造函数。返回该对象的引用  如果其他的可能会用到NewObject
    env->CallVoidMethod(jobject, methedID, NULL);
    //jstring strResult = (jstring)env->CallStaticObjectMethod(mainclass, methedID, strr);
    //char* pBuf = new char[1024];
    //memset(pBuf, 0, 1024);
    //char *q = pBuf;
    //q = jstringToWindows(env,strResult);
    //cout << q << endl;
    cout << "执行结束" << endl;

    //jvm释放
    jvm->DestroyJavaVM();
    FreeLibrary(jvmDLL);
    return true;
}
// 步骤按照上面来  注释都写好了  注意找到java
//总结 上面实验了三种情况：
//1.直接调用main方法 java  main是static的，
//(1)直接加载启动方法GetStaticMethodID(mainclass, startMethod, "(Ljava/lang/String;)Ljava/lang/String;");
// (2)env->CallStaticObjectMethod(mainclass, methedID, NULL);  注意因为java返回值不一样CallStaticObjectMethod  CallStaticVoidMethod等可以选择使用
//2.调用带参数的static方法，其实与上面一样
// (1) 同上，唯一的区别就是第三个参数不能为null，是java需要的参数，同时jstring传递时注意互转java c++
//3.调用类成员方法  复杂一点  暂时没搞懂啥意思
// (1)加载启动方法都是一样的
// (2)因为是成员变量需要先实例化一个对象出来  jobject jobject = env->AllocObject(mainclass);//分配新 Java 对象而不调用该对象的任何构造函数。返回该对象的引用  如果其他的可能会用到NewObject
// (3) env->CallVoidMethod(jobject, methedID, NULL);然后用这个对象去启动方法
// (4)带参数应该也差不多，NULL换成参数

// JNI还是比较复杂的  有需要的可以多看看

// 本文参考：https://blog.csdn.net/sf0407/article/details/53924174
//           https://blog.csdn.net/toyauko/article/details/83109325


// C++调java大致如此，将java封装成jar包，(可以给出class，方便别人查看和使用),r然后在c++中启动jvm虚拟机，调用方法，看起来还是比较复杂的
// java掉c++相对简单许多，c++编译成dll/so库，java加载，使用就可以了，但是需要注意c++方法的名字，要符合jni规范，要出注意java调用地方的包名和类名
// 这是以前在java调c++的参考(ffmpeg在java中调用，当时编译ffmpeg.so可愁死我啦，破公司没有外网环境，linux也没有，还是在windows上用eclipse环境ndk编出的so)
// 当时使用ffmpeg也比较巧妙，改了ffmpeg.cpp中的一个读取命令行参数的方法为main方法，然后java穿命令过来就行了
//https://www.cnblogs.com/dengpeng1004/p/8745186.html