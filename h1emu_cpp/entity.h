#ifndef ENTITY_H
#define ENTITY_H
#include <string>
#include "structures.h"
#include <boost\shared_ptr.hpp>

class entity {
public: 
	virtual ~entity() {}
	virtual void deliver(std::string message) = 0;
	virtual std::string get_id() = 0;
	virtual player& get_player_data() = 0;
};

#endif