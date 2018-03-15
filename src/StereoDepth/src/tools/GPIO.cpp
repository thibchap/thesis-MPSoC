/*
 * GPIO.cpp
 *
 *  Created on: Jan 5, 2018
 *      Author: Thibault Chappuis
 */

#include "GPIO.h"

using namespace std;

GPIO::GPIO(const uint addr, const bool input) : addr(addr), input(input), ptr(NULL)
{
	if(input) 	fd = open("/dev/mem", O_RDONLY);
	else 		fd = open("/dev/mem", O_RDWR);

	if(fd < 1) {
		cerr << "[GPIO] Error, can't open file descriptor /dev/mem" << endl;
		return;
	}
	page_addr = (addr & ~(PAGE_SIZE-1));
	page_offset = addr - page_addr;

	cout << "page_addr 0x" << hex << page_addr << dec << endl;
	cout << "page_offset 0x" << hex << page_offset << dec << endl;

	if(input) 	ptr = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd, page_addr);
	else		ptr = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);

	if((long long)ptr == -1) {
		cerr << "[GPIO] Error, can't map memory address 0x" << hex << addr << dec << endl;
	}
}

GPIO::~GPIO() {
	// nothing
}

uint GPIO::read() const {
	if((long long)ptr == -1 or input == false)
		return 0;

	return ((uint*) ptr)[page_offset];
}

void GPIO::write(const uint value) {
	if((long long)ptr == -1 or input == true)
		return;

	((uint*) ptr)[page_offset] = value;
}
