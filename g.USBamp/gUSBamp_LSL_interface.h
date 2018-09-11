#ifndef GUSBAMP_LSL_INTERFACE_H
#define GUSBAMP_LSL_INTERFACE_H
#include <cstdint>
#include <vector>


class gUSBamp_LSL_interface {
public:
	explicit gUSBamp_LSL_interface(int32_t device_param);
	~gUSBamp_LSL_interface();
	bool getData(std::vector<int32_t>& buffer);
	bool getStatus() { return true; }
private:
	int32_t counter;
};

#endif // GUSBAMP_LSL_INTERFACE_H
