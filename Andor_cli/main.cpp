#include "demo.h"

int main()
{
	Spec_demo ^p_spec = gcnew Spec_demo();
	cout << p_spec->get_device_cnt() << endl;
	int ret = p_spec->set_exposure_time(0,0.01);
	if (ret != AT_SUCCESS) {
		cout << "error occur" << endl;
	}
	//p_spec->serial_do_scan();
	p_spec->basic_concurrency_do_scans();
	cout << "ok" << endl;
	while (1);
}