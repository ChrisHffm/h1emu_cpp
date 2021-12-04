#pragma once
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/make_shared.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <spdlog/spdlog.h>
#include <iostream>
#include <exception>
#include "loginserver.h"
#include "entity.h"
#include "session.h"

class server
{
public:
	server(const std::string& address, std::uint16_t port);
	
	void run();

private:
	void start_receive();

	void handle_receive(const boost::system::error_code& error,
		std::size_t /*bytes_transferred*/);

	void handle_send(boost::shared_ptr<std::string> /*message*/,
		const boost::system::error_code& /*error*/,
		std::size_t /*bytes_transferred*/);

	boost::asio::io_service io_service;
	boost::shared_ptr<boost::asio::ip::udp::socket> socket_;
	boost::asio::ip::udp::endpoint remote_endpoint_;
	boost::array<unsigned char, UDP_LENGTH> recv_buffer_;

	//The next connection to be accepted
	boost::shared_ptr<session> new_session;

	loginserver loginserver_;
	//gatewayserver gatewayserver__;
};