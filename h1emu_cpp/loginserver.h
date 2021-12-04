#ifndef LOGINSERVER_H
#define LOGINSERVER_H

#include <iostream>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <spdlog/spdlog.h>

#include "packet_handling.h"

#include <set>
#include "entity.h"
#include "structures.h"

class loginserver
{
public: 
	void ack(const std::string /*sender_id*/, const std::uint16_t /*sequence*/);

	void session_reply(const std::string /*sender_id*/, const std::uint32_t /*session_id*/);

	void pong(const std::string /*sender_id*/);

	void login_reply(const std::string& /*sender_id*/);

	void join(boost::shared_ptr<entity> /*entity*/);

	void leave(boost::shared_ptr<entity> /*entity*/);

	void update_position(const std::string /*sender_id*/, vector3 /*position*/);

private:
	void update_entities();

	std::vector<boost::shared_ptr<entity>> entities;
};

#endif // !LOGINSERVER_H