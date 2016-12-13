#include "demo.h"
#include "asyn_invoke.h"
int main()
{
	//Spec_demo ^p_spec = gcnew Spec_demo();
	//cout << p_spec->get_device_cnt() << endl;
	//int ret = p_spec->set_exposure_time(0,0.01);
	//if (ret != AT_SUCCESS) {
	//	cout << "error occur" << endl;
	//}
	////p_spec->serial_do_scan();
	//p_spec->basic_concurrency_do_scans();
	//cout << "ok" << endl;
	Asyn_invoke_callback ^p_run = gcnew Asyn_invoke_callback();
	p_run->main_invoke();
	cout << "over" << endl;
	while (1);
}