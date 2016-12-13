#ifndef ASYN_INVOKE_H
#define ASYN_INVOKE_H

#include <iostream>
#include <msclr\lock.h>
#include <string>
using namespace std;
using namespace System;
using namespace System::Threading;

ref class Asyn_invoke_block
{
private:
	delegate void PrintDelegate(Object ^);
	static void child_invoke(Object ^args)
	{
		String ^str = (String^)args;
		Thread::Sleep(500);
		char buffer[100];
		sprintf(buffer, "%s", str);
		cout << "child run" << Thread::CurrentThread->ManagedThreadId << buffer << endl;
	}
public:
	static void main_invoke()
	{
		PrintDelegate ^printDelegate = gcnew PrintDelegate(child_invoke);
		cout << "main invoke:" << Thread::CurrentThread->ManagedThreadId << endl;
		String ^wori = "hello";
		IAsyncResult ^result = printDelegate->BeginInvoke(wori, nullptr, nullptr);
		cout << "main invoke go on." << Thread::CurrentThread->ManagedThreadId << endl;
		while (!result->IsCompleted) {
			cout << "waiting--" << endl;
			Thread::Sleep(500);
		}
	}
};

public ref class Asyn_invoke_callback
{
private:
	delegate String^ PrintDelegate(String ^);
	static String^ child_invoke(String ^args)
	{
		cout << "child run" << Thread::CurrentThread->ManagedThreadId << endl;
		return "tiankai";
	}
	static void callback_(IAsyncResult ^result)
	{
		PrintDelegate ^printDelegate = (PrintDelegate ^ )result->AsyncState;
		String^ ret = printDelegate->EndInvoke(result);
		char buffer[100];
		sprintf(buffer, "%s", ret);
		cout << "当前线程结束." << buffer << endl;
	}
public:
	static void main_invoke()
	{
		PrintDelegate ^printDelegate = gcnew PrintDelegate(child_invoke);
		AsyncCallback ^callback = gcnew AsyncCallback(callback_);
		cout << "main invoke:" << Thread::CurrentThread->ManagedThreadId << endl;
		String ^wori = "hello";
		printDelegate->BeginInvoke(wori, callback, printDelegate);
		cout << "main invoke go on.." << Thread::CurrentThread->ManagedThreadId << endl;
		while (1);
	}
};
#endif