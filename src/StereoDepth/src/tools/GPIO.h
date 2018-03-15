/*
 * GPIO.h
 *
 *  Created on: Jan 5, 2018
 *      Author: Thibault Chappuis
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define LED_ADDR		0x80010000
#define DIP_SW_ADDR		0x80000000
#define PUSH_BTN_ADDR	0x80020000

class GPIO {
public:
	GPIO(const uint addr, const bool input = false);

	virtual ~GPIO();

	uint read() const;

	void write(const uint value);

private:
	const uint PAGE_SIZE = sysconf(_SC_PAGESIZE);

	uint addr;
	bool input;
	uint page_addr;
	uint page_offset;

	int fd;	// file descriptor
	void* ptr;
};

#endif /* SRC_GPIO_H_ */
