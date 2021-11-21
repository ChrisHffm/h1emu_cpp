#ifndef ZONE_H
#define ZONE_H

#include <iostream>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <spdlog/spdlog.h>
#include <Poco/BinaryReader.h>
#include <Poco/BinaryWriter.h>
#include <set>
#include "entity.h"
#include "structures.h"

class zone
{
public: 
	void join(boost::shared_ptr<entity> entity);

	void leave(boost::shared_ptr<entity> entity);

	void deliver(const std::string message);

	void update_position(const std::string sender_id, vector3 position);

	void update_rotation(const std::string sender_id, vector3 rotation);

	void update_looking_at(const std::string sender_id, vector3 looking_at);

	void send_text_message(const std::string sender_id, const std::string message);

	void session_reply(const std::string sender_id, const std::uint32_t session_id);


private:
	void update_entities();

	std::vector<boost::shared_ptr<entity>> entities;
};

#endif // !ZONE_H