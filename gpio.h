/*
 * GPIO.h  Created on: 29 Apr 2014
 * Copyright (c) 2014 Derek Molloy (www.derekmolloy.ie)
 * Made available for the book "Exploring BeagleBone"
 * See: www.exploringbeaglebone.com
 * Licensed under the EUPL V.1.1
 *
 * This Software is provided to You under the terms of the European
 * Union Public License (the "EUPL") version 1.1 as published by the
 * European Union. Any use of this Software, other than as authorized
 * under this License is strictly prohibited (to the extent such use
 * is covered by a right of the copyright holder of this Software).
 *
 * This Software is provided under the License on an "AS IS" basis and
 * without warranties of any kind concerning the Software, including
 * without limitation merchantability, fitness for a particular purpose,
 * absence of defects or errors, accuracy, and non-infringement of
 * intellectual property rights other than copyright. This disclaimer
 * of warranty is an essential part of the License and a condition for
 * the grant of any rights to this Software.
 *
 * For more details, see http://www.derekmolloy.ie/
 */

#ifndef GPIO_H_
#define GPIO_H_
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<cstdlib>
#include<cstdio>
#include<fcntl.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<pthread.h>
using std::string;
using std::ofstream;

#define GPIO_PATH "/sys/class/gpio/"

namespace exploringBB {

typedef int (*CallbackType)(int);
enum GPIO_DIRECTION{ INPUT, OUTPUT };
enum GPIO_VALUE{ LOW=0, HIGH=1 };
enum GPIO_EDGE{ NONE, RISING, FALLING, BOTH };

class GPIO {
private:
	int number, debounceTime;
	string name, path;

public:
	GPIO(int number) {
	this->number = number;
	this->debounceTime = 0;
	this->togglePeriod=100;
	this->toggleNumber=-1; //infinite number
	this->callbackFunction = NULL;
	this->threadRunning = false;

	ostringstream s;
	s << "gpio" << number;
	this->name = string(s.str());
	this->path = GPIO_PATH + this->name + "/";
//	this->exportGPIO();
	// need to give Linux time to set up the sysfs structure
	usleep(250000); // 250ms delay
}
 //constructor will export the pin
	virtual int getNumber() { return number; }

	// General Input and Output Settings
	virtual int setDirection(GPIO_DIRECTION dir)
	{
   	switch(dir){
   	case INPUT: return this->write(this->path, "direction", "in");
      	break;
   	case OUTPUT:return this->write(this->path, "direction", "out");
      	break;
   	}
   	return -1;
	}
	virtual GPIO_DIRECTION getDirection()
	{
	string input = this->read(this->path, "direction");
	if (input == "in") return INPUT;
	else return OUTPUT;
	}
	
	virtual int setValue(GPIO_VALUE value)
	{
   	switch(value){
   	case HIGH: return this->write(this->path, "value", "1");
      	break;
   	case LOW: return this->write(this->path, "value", "0");
      	break;
   	}
   	return -1;
	}

	virtual int toggleOutput();
	virtual GPIO_VALUE getValue()
	{
	string input = this->read(this->path, "value");
	if (input == "0") return LOW;
	else return HIGH;
	}

	virtual int setActiveLow(bool isLow){
   	if(isLow) return this->write(this->path, "active_low", "1");
   	else return this->write(this->path, "active_low", "0");
	}  //low=1, high=0
	virtual int:setActiveHigh(){
   	return this->setActiveLow(false);
	}
 //default
	//software debounce input (ms) - default 0
	virtual void setDebounceTime(int time) { this->debounceTime = time; }

	// Advanced OUTPUT: Faster write by keeping the stream alive (~20X)
	virtual int streamOpen()
	{
	stream.open((path + "value").c_str());
	return 0;
	}
	virtual int streamWrite(GPIO_VALUE value)
	{
	stream << value << std::flush;
	return 0;
	}
	virtual int streamClose()
	{
	stream.close();
	return 0;
	}

	virtual int toggleOutput()
	{
	this->setDirection(OUTPUT);
	if ((bool) this->getValue()) this->setValue(LOW);
	else this->setValue(HIGH);
   	 return 0;
	}
 //threaded invert output every X ms.
	virtual int toggleOutput(int numberOfTimes, int time);
	virtual void changeToggleTime(int time) { this->togglePeriod = time; }
	virtual void toggleCancel() { this->threadRunning = false; }

	// Advanced INPUT: Detect input edges; threaded and non-threaded
	virtual int setEdgeType(GPIO_EDGE value)
		{
   	switch(value){
   	case NONE: return this->write(this->path, "edge", "none");
   	   break;
   	case RISING: return this->write(this->path, "edge", "rising");
   	   break;
   	case FALLING: return this->write(this->path, "edge", "falling");
   	   break;
   	case BOTH: return this->write(this->path, "edge", "both");
   	   break;
   		}
	virtual GPIO_EDGE getEdgeType()
	{
	string input = this->read(this->path, "edge");
	if (input == "rising") return RISING;
	else if (input == "falling") return FALLING;
	else if (input == "both") return BOTH;
	else return NONE;
	}
	virtual int waitForEdge(); // waits until button is pressed
	virtual int waitForEdge(CallbackType callback); // threaded with callback
	virtual void waitForEdgeCancel() { this->threadRunning = false; }

	virtual ~GPIO();  //destructor will unexport the pin

private:
	int write(string path, string filename, string value);
	int write(string path, string filename, int value);
	string read(string path, string filename);
//	int exportGPIO();
//	int unexportGPIO();
	ofstream stream;
	pthread_t thread;
	CallbackType callbackFunction;
	bool threadRunning;
	int togglePeriod;  //default 100ms
	int toggleNumber;  //default -1 (infinite)
	friend void* threadedPoll(void *value);
	friend void* threadedToggle(void *value);
};

void* threadedPoll(void *value);
void* threadedToggle(void *value);

} /* namespace exploringBB */

#endif /* GPIO_H_ */
