#include <fstream>
#include <string>
#include <iostream>

#include "server.h"

#include "RC4.h"
#include "CRC32.h"

using boost::asio::ip::udp;

server::server(std::string address, std::uint16_t port) :
	io_service(),
	socket_(boost::make_shared<udp::socket>
		(io_service, udp::endpoint(boost::asio::ip::address::from_string(address), port)))
{
	if (socket_->is_open()) {
		spdlog::info("socket is open");
		start_receive();
	}
	else {
		spdlog::error("socket is closed!");
	}
}

void server::run() { io_service.run(); }

void server::start_receive()
{
	socket_->async_receive_from(
		boost::asio::buffer(recv_buffer_), remote_endpoint_,
		boost::bind(&server::handle_receive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void server::handle_receive(const boost::system::error_code& error,
	std::size_t bytes_transferred)
{
	if (!error || error == boost::asio::error::message_size)
	{
		try {
			auto sender_id = boost::lexical_cast<std::string>(remote_endpoint_.port());

			Packet p(recv_buffer_.data(), bytes_transferred);

			p.add_field<std::uint16_t>("soe_op_code");

			// <=== handle the data we receive accorded to the op code ===>
			switch (p.get_field("soe_op_code"))
			{
			case e_soe_op_code::session_req: {

				p.set_name("session_req");
				p.add_field<std::uint32_t>("crc_length");
				p.add_field<std::uint32_t>("session_id");
				p.add_field<std::uint32_t>("udp_length");
				p.add_field<char*>("protocol", 12);

				std::cout << "\n" << p << "\n";

				new_session.reset(new session(socket_, remote_endpoint_, loginserver_));
				new_session->start();

				loginserver_.session_reply(sender_id, p.get_field("session_id"));
			}
			break;
			case e_soe_op_code::chl_data_frag_a: {

				p.set_name("data_fragmented");

				static std::string		app_data;
				static std::uint16_t	sequence_count = 0;
				static std::uint16_t	next_sequence_index = 0;

				p.add_field<std::uint8_t>	("comp_flag");
				p.add_field<std::uint16_t>	("sequence_index");

				if (p.get_field("sequence_index") == FIRST_FLAG_SEQ)
					p.add_field<std::uint32_t>("full_size");

				auto app_data_length = p.bytes_left() /*- 2*/;

				p.add_field<unsigned char*>("app_data", app_data_length);
				//p.add_field<std::uint16_t>	("crc");

				if (sequence_count == 0)
					sequence_count = p.get_field("full_size") / UDP_LENGTH;

				if (p.get_field("sequence_index") == next_sequence_index) {

					loginserver_.ack(sender_id, next_sequence_index);

					char app_data_frag[UDP_LENGTH];

					p.get_field("app_data", app_data_frag);

					app_data.append(app_data_frag, app_data_length);

					next_sequence_index++;
				}

				if (p.get_field("sequence_index") == sequence_count) {
					
					std::vector<unsigned char> key = { 0x17, 0xbd, 0x08, 0x6b, 0x1b, 0x94, 0xf0, 0x2f, 0xf0, 0xec, 0x53, 0xd7, 0x63, 0x58, 0x9b, 0x5f };

					SOE::RC4 RC4;
					RC4.Init(key);

					auto data = RC4.Parse((unsigned char*)app_data.data(), app_data.length());

					Packet d(data.data(), data.size());

					d.add_field<std::uint8_t>("h1z1_op_code");
					spdlog::info("opcode: {}", d.get_field("h1z1_op_code"));
					switch (d.get_field("h1z1_op_code"))
					{
					case e_h1z1_op_code::login_req:
					{
						loginserver_.login_reply(sender_id);
					}
					break;
					}

					app_data.clear();
					next_sequence_index = 0;
					sequence_count = 0;
				}
			}
			break;
			case e_soe_op_code::disconnect: {

				p.set_name("disconnect");
				p.add_field<std::uint32_t>	("session_id");
				p.add_field<std::uint16_t>	("disconnect_reason_id");

				spdlog::info("0x{:x} disconnected with reason: {}",
					p.get_field(1),
					p.get_field(2)
				);

				// #todo: erase the session maybe send the disconnect reason to connected clients ?
			}
			break;
			case e_soe_op_code::ping: {

				loginserver_.pong(sender_id);

			}
			break;
			}
		}
		catch (std::exception& e) {
			spdlog::critical("{}",
				e.what());
		}

		start_receive();
	}
}

void server::handle_send(boost::shared_ptr<std::string> message,
	const boost::system::error_code& error,
	std::size_t bytes_transferred)
{
	//todo: :)
}