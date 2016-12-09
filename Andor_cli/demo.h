#ifndef ANDOR_DEMO_H
#define ANDOR_DEMO_H
#include "atcore.h"
#include <iostream>
#include <msclr\lock.h>
using namespace std;
using namespace System;
using namespace System::Threading;

#define EXTRACTLOWPACKED(SourcePtr) ( (SourcePtr[0] << 4) + (SourcePtr[1] & 0xF) )
#define EXTRACTHIGHPACKED(SourcePtr) ( (SourcePtr[2] << 4) + (SourcePtr[1] >> 4) )

public ref class Spec_demo
{
public:
	Spec_demo()
	{
		AT_InitialiseLibrary();
		p_device_cnt = new AT_64;
		AT_GetInt(AT_HANDLE_SYSTEM, L"Device Count", p_device_cnt);
		handles = new int[*p_device_cnt];
		buffers = new unsigned char*[*p_device_cnt];
		for (int i = 0; i < *p_device_cnt; i++) {
			int ret_code = AT_Open(i, handles+i); //打开各设备
			if (ret_code != AT_SUCCESS) {
				cout << "Error condition, could not initialise camera" << endl << endl;
			}
			AT_64 *p_ImageSizeBytes=new AT_64;
			AT_GetInt(handles[i], L"Image Size Bytes", p_ImageSizeBytes);
			int BufferSize = static_cast<int>(*p_ImageSizeBytes);
			buffers[i] = new unsigned char[BufferSize];
			delete p_ImageSizeBytes;
		}
	}
	~Spec_demo()
	{
		for (int i = 0; i < *p_device_cnt; i++) {
			int ret_code = AT_Close(handles[i]);
			if (ret_code != AT_SUCCESS) {
				cout << "Error condition, could not close camera" << endl << endl;
			}
		}
		delete p_device_cnt;
		AT_FinaliseLibrary();
	}
	//通过Handle设置光谱仪
	int set_exposure_time(int index,double exposure_tie)
	{
		AT_H Hndl = get_handle(index);
		return AT_SetFloat(Hndl, L"Exposure Time", exposure_tie);
	}
	//通过Handle读取光谱仪。这里需要先设置pixel encoding,然后安装规则从buffer里取数据
	//这里要加上数据的处理，而不会简单输出
	void do_scan(int device_index)
	{
		AT_H Hndl = get_handle(device_index);
		cout << Hndl << endl;
		//Get the number of bytes required to store one frame
		AT_64 *p_ImageSizeBytes=new AT_64;
		AT_GetInt(Hndl, L"Image Size Bytes", p_ImageSizeBytes);
		int BufferSize = static_cast<int>(*p_ImageSizeBytes);
		cout << "buffer size" << BufferSize << endl;
		unsigned char* UserBuffer = new unsigned char[BufferSize];
		AT_QueueBuffer(Hndl, UserBuffer, BufferSize);
		AT_Command(Hndl, L"Acquisition Start");
		unsigned char* Buffer;
		if (AT_WaitBuffer(Hndl, &Buffer, &BufferSize, 10000) == AT_SUCCESS) {
			cout << "For Test: Print out of first 20 pixels " << endl;
			//Unpack the 12 bit packed data
			for (int i = 0; i < 3 * 10; i += 3) {
				AT_64 LowPixel = EXTRACTLOWPACKED(Buffer);
				AT_64 HighPixel = EXTRACTHIGHPACKED(Buffer);
				cout << HighPixel << " " << LowPixel << " ";
				Buffer += 3;
			}
			cout << endl;
			delete[] UserBuffer;
		}
		else {
			cout << "Timeout occurred check the log file ..." << endl << endl;
		}
		AT_Command(Hndl, L"Acquisition Stop");
		AT_Flush(Hndl);
		delete p_ImageSizeBytes;
	}
	void serial_do_scans() //串行读一次
	{
		cout << "serial do scan " << endl;
		for (int i = 0; i < *p_device_cnt; i++) {
			do_scan(i);
		}
	}
	//多线程并发读取所有光谱数据
	void basic_concurrency_do_scans()
	{
		lockx = gcnew Object(); //用于锁一些状态
		drawed_flag = false; //绘图完成标志
		//task和received_msg用于多个读取线程同步
		task = 0; //已完成的任务数
		received_msg = 0; //有多少线程收到了绘图完成的消息

		for (int i = 0; i < *p_device_cnt; i++) //为每个通道创建一个线程
		{
			ParameterizedThreadStart ^pStart = gcnew ParameterizedThreadStart(do_scan_thread);
			Thread ^thread = gcnew Thread(pStart);
			thread->Start(i);
		}
		//绘图线程
		ParameterizedThreadStart ^pStart = gcnew ParameterizedThreadStart(display_data);
		Thread ^thread = gcnew Thread(pStart);
		thread->Start(this);
	}
	static void do_scan_thread(Object ^arg)
	{
		int index = (int)arg;
		AT_H Hndl = get_handle(index);
		int BufferSize = sizeof(buffers[index]);

		while (true)
		{
			AT_QueueBuffer(Hndl, buffers[index], BufferSize);
			AT_Command(Hndl, L"Acquisition Start");
			unsigned char* Buffer;
			if (AT_WaitBuffer(Hndl, &Buffer, &BufferSize, 10000) == AT_SUCCESS) {
				cout << "thread " <<index << " scan done...waiting.."<< endl;
			}
			else {
				cout << "Timeout occurred check the log file ..." << endl << endl;
			}
			AT_Command(Hndl, L"Acquisition Stop");
			AT_Flush(Hndl);

			Monitor::Enter(lockx);
			task++;
			Monitor::Exit(lockx);

			while (!drawed_flag) Thread::Sleep(1);

			Monitor::Enter(lockx);
			received_msg++;
			Monitor::Exit(lockx);

			while (received_msg < *p_device_cnt) Thread::Sleep(1);
			drawed_flag = false;
		}
		task = 0;
		received_msg = 0;
		drawed_flag = false;
	}
	static void display_data(Object ^args)
	{
		while (true)
		{
			if (task < *p_device_cnt) { Thread::Sleep(1); }
			else
			{
				for (int i = 0; i < *p_device_cnt; i++) {
					cout << "For Test: Print out of first 20 pixels " << endl;
					unsigned char *p_buffer = buffers[i];
					for (int i = 0; i < 3 * 10; i += 3) {
						AT_64 LowPixel = EXTRACTLOWPACKED(p_buffer);
						AT_64 HighPixel = EXTRACTHIGHPACKED(p_buffer);
						cout << HighPixel << " " << LowPixel << " ";
						p_buffer += 3;
					}
					cout << endl;
				}
				task = 0;
				received_msg = 0;
				drawed_flag = true;
				cout << "draw once." << endl << endl;
				Thread::Sleep(500);
			}
		}
		task = 0;
		received_msg = 0;
		drawed_flag = false;
	}
	int get_device_cnt()
	{
		return *p_device_cnt;
	}
	static AT_H get_handle(int index)
	{
		return handles[index];
	}
private:
	static AT_64 *p_device_cnt;
	static AT_H *handles;
	static unsigned char** buffers; //记录所有buffer，每个设备使用一个
	static Object ^lockx;
	static int task;
	static int received_msg;
	static bool drawed_flag;
};
#endif ANDOR_DEMO_H