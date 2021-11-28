#include <vector>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/container/vector/vector10.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/type_traits/remove_reference.hpp>
using namespace boost::fusion;

struct struct_station
{
	std::uint8_t	id;
	vector3<float>	position;
	std::string		station_name;
};

class gas_station {
public:
	void add_station(vector3<float> position, const std::string& station_name);

	void rem_station(const std::string& station_name);


	void init();

private:

	std::vector<struct_station> stations_;
};