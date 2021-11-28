#include <fstream>
#include "gas_station.h"


void gas_station::init()
{
	std::ifstream ifs("plugin/gas_station/config.json");

	// later parse the file
}

void gas_station::add_station(vector3<float> position, const std::string& station_name)
{

}

void gas_station::rem_station(const std::string& station_name)
{

}
