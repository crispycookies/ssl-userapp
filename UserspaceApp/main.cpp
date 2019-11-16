/////////////////////////////////////////////////////////////
// Author: Patrick Wuerflinger
// Filename: main.cpp
// Description: Entry point for the user application
// Revision: 0
/////////////////////////////////////////////////////////////

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <libfpgaregion.h>
#include "SensorSettings.h"
#include "MQTTPublisher.h"
#include "HDC1000.h"

using namespace std::chrono_literals;

// Callback functions for the FPGA region
void ReconfigRequest();
void ReconfigDone();
// Mutex for synchronization 
static std::mutex fpgaMutex;
static FpgaRegion fpga("UserspaceApp", ReconfigRequest, ReconfigDone);

// Callback function for a reconfiguration request
void ReconfigRequest() 
{
	std::cout << "New incoming reconfigRequest." << std::endl;
	std::cout << "Wait to reconfigure savely." << std::endl;
	fpgaMutex.lock();
	std::cout << "FPGA ressources are free now." << std::endl;
	fpga.Release();
	std::cout << "Reconfiguration..." << std::endl;
}

// Callback function for the finished reconfiguration
void ReconfigDone() 
{
	std::cout << "Reconfiguration done" << std::endl;
	std::cout << "FPGA ressources are used again." << std::endl;
	fpga.Acquire();
	fpgaMutex.unlock();
}

int main()
{
	try
	{
		// Initialize necassery variables
		MQTTPublisher publisher(MQTT_SERVER_ADDRESS);
		
		// Create and add sensors
		auto hdc1000 = std::make_shared<HDC1000>(MQTT_HDC1000_NAME, HDC1000_DRIVER_PATH);
		publisher.addSensor(hdc1000);
		//....
		
		// first init to use FPGA ressources
		fpga.Acquire();
		// super loop 
		while (true)
		{
			// lock FPGA ressources 
			fpgaMutex.lock();
			// connect to server
			publisher.connect();
			// perform measurements and publish via MQTT message broker
			publisher.publish(QoS_AT_LEAST_ONCE);
			// disconnect from server
			publisher.disconnect();
			// unlock FPGA ressources for potential reconfiguration
			fpgaMutex.unlock();	
			// block thread for 500 ms
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
	catch(std::string const & error)
	{
		std::cerr << error << std::endl;
		return 1;
	}
	catch(std::bad_alloc const & error)
	{
		std::cerr << "Memory allocation: " << error.what() << std::endl;
		return 1; 
	}
	catch(...)
	{
		std::cerr << "Unhandled Exception" << std::endl;
		return 1; 
	}
	return 0;
}