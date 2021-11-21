#ifndef SESSION_H
#define SESSION_H

#include <iostream>
#include <deque>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "entity.h"
#include "zone.h"
#include "structures.h"

class session : public entity
, public boost::enable_shared_from_this<session> {
public: 
	// construct a connection with the given io service
	session(boost::shared_ptr<boost::asio::ip::udp::socket> socket, 
		boost::asio::ip::udp::endpoint endpoint, zone& zone);

	boost::asio::ip::udp::socket& socket(); 

	void start();

private:
	void deliver(std::string message);

	player& get_player_data();

	void start_receive();

	void handle_receive(const boost::system::error_code& error,
		std::size_t /*bytes_transferred*/);

	void handle_send(std::string /*message*/,
		const boost::system::error_code& /*error*/,
		std::size_t /*bytes_transferred*/);

	std::string get_id();

	boost::shared_ptr<boost::asio::ip::udp::socket> socket_;

	boost::asio::ip::udp::endpoint remote_endpoint_;

	std::deque<std::string> message_queue;

	/// The buffer for incoming data.
	boost::array<char, 1024> buff_;

	zone& zone_;

	player player_;
};

#endif