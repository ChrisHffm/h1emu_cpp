#include <fstream>
#include <string>
#include <iostream>

#include "server.h"

#include "RC4.h"
#include "CRC32.h"

using boost::asio::ip::udp;

server::server(const std::string& address, std::uint16_t port) :
	io_service(),
	socket_(boost::make_shared<udp::socket>
		(io_service, udp::endpoint(boost::asio::ip::address::from_string(address), port)))
{
	if (socket_->is_open())
	{
		spdlog::info("socket is open");
		start_receive();
	}
	else
	{
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

void handle_reliable_data(uint8_t* buffer, size_t size)
{

}

void handle_packet(uint8_t* buffer, size_t size)
{
	Packet p(buffer, size);

	p.add_field<std::uint16_t>("soe_op_code");
	spdlog::info("handle_packet: {} bytes: {}", p.get_field("soe_op_code"), size);
}

void server::handle_receive(const boost::system::error_code& error,
	std::size_t bytes_transferred)
{
	if (!error || error == boost::asio::error::message_size)
	{
		try
		{
			auto sender_id = std::to_string(remote_endpoint_.port());

			Packet p(recv_buffer_.data(), bytes_transferred);

			p.add_field<std::uint16_t>("soe_op_code");

			switch (p.get_field("soe_op_code"))
			{
			case session_req:
			{
				p.set_name("session_request");
				p.add_field<std::uint32_t>("crc_length");
				p.add_field<std::uint32_t>("conn_id");
				p.add_field<std::uint32_t>("buf_size");
				p.add_field<char*>("protocol", 12);

				new_session.reset(new session(socket_, remote_endpoint_, loginserver_));
				new_session->start();

				loginserver_.session_reply(sender_id, p.get_field("conn_id"));
			}
			break;
			case multi:
			{
				p.set_name("multi");
				p.add_field	<uint16_t>("opcode");

				uint8_t subpacket_num = 0;

				while (p.bytes_left())
				{
					std::string subpacket_data;
					std::string subpacket_str = "subpacket_";
					std::string subpacket_len_str = "subpacket_len_";
					subpacket_str += std::to_string(subpacket_num);
					subpacket_len_str += std::to_string(subpacket_num);

					p.add_field	<uint8_t>(subpacket_len_str);
					p.add_field	<uint8_t*>(subpacket_str, p.get_field(subpacket_len_str));

					subpacket_num++;

					char app_data_frag[UDP_LENGTH];

					p.get_field(subpacket_str, app_data_frag);

					subpacket_data.append(app_data_frag, p.get_field(subpacket_len_str));

					handle_packet((uint8_t*)subpacket_data.data(), p.get_field(subpacket_len_str));
				}
			}
			break;
			case chl_data_a:
			{
				spdlog::info("chl_data_a");

				static std::string app_data;
				static std::uint16_t sequence_count = 0;
				static std::uint16_t next_sequence_index = 0;

				p.add_field<std::uint8_t>("zflag");
				p.add_field<std::uint16_t>("seq_num");

				if (p.get_field("seq_num") == FIRST_FLAG_SEQ)
					p.add_field<std::uint32_t>("full_size");

				auto app_data_length = p.bytes_left();

				p.add_field<unsigned char*>("app_data", app_data_length);

				if (sequence_count == 0)
					sequence_count = p.get_field("full_size") / UDP_LENGTH;

				if (p.get_field("seq_num") == next_sequence_index)
				{
					//loginserver_.ack(sender_id, next_sequence_index);

					char app_data_frag[UDP_LENGTH];

					p.get_field("app_data", app_data_frag);

					app_data.append(app_data_frag, app_data_length);

					next_sequence_index++;
				}

				if (p.get_field("seq_num") == sequence_count)
				{
					SOE::rc4_crypt((unsigned char*)app_data.data(), app_data.length());

					Packet d((unsigned char*)app_data.data(), app_data.size(), 0);

					d.add_field<std::uint8_t>("h1z1_op_code");
					spdlog::info("h1z1_op_code: {}", d.get_field("h1z1_op_code"));
					switch (d.get_field("h1z1_op_code"))
					{
					case login_req:
					{
						d.set_name("login_req");
						d.add_field	<uint32_t>("session_id_len");
						d.add_field	<char*>("session_id", d.get_field("session_id_len"));
						d.add_field	<uint32_t>("fingerprint_len");
						d.add_field	<char*>("fingerprint", d.get_field("fingerprint_len"));
						d.add_field	<uint32_t>("locale");
						d.add_field	<uint32_t>("third_party_auth_ticket");
						d.add_field	<uint32_t>("third_party_user_id");
						d.add_field	<uint32_t>("third_party_id");

						//std::cout << "\n" << d << "\n";

						loginserver_.login_reply(sender_id);
					}
					break;
					default:;
					}

					app_data.clear();
					next_sequence_index = 0;
					sequence_count = 0;
				}
			}
			break;
			case chl_data_frag_a:
			{
				spdlog::info("chl_data_frag_a");

				p.set_name("data_fragmented");

				static std::string app_data;
				static std::uint16_t sequence_count = 0;
				static std::uint16_t next_sequence_index = 0;

				p.add_field<std::uint8_t>("zflag");
				p.add_field<std::uint16_t>("seq_num");

				if (p.get_field("seq_num") == FIRST_FLAG_SEQ)
					p.add_field<std::uint32_t>("full_size");

				/*spdlog::info("zflag:		{}", p.get_field("zflag"));
				spdlog::info("seq_num:	{}", p.get_field("seq_num"));

				if(p.field_exists("full_size"))
					spdlog::info("full_size:	{}", p.get_field("full_size"));*/

				auto app_data_length = p.bytes_left();

				p.add_field<unsigned char*>("app_data", app_data_length);

				if (sequence_count == 0)
					sequence_count = p.get_field("full_size") / UDP_LENGTH;

				if (p.get_field("seq_num") == next_sequence_index)
				{
					//loginserver_.ack(sender_id, next_sequence_index);

					char app_data_frag[UDP_LENGTH];

					p.get_field("app_data", app_data_frag);

					app_data.append(app_data_frag, app_data_length);

					next_sequence_index++;
				}

				if (p.get_field("seq_num") == sequence_count)
				{
					SOE::rc4_crypt((unsigned char*)app_data.data(), app_data.length());

					Packet d((unsigned char*)app_data.data(), app_data.size(), 0);

					d.add_field<std::uint8_t>("h1z1_op_code");
					spdlog::info("h1z1_op_code: {}", d.get_field("h1z1_op_code"));
					switch (d.get_field("h1z1_op_code"))
					{
					case login_req:
					{
						d.set_name("login_req");
						d.add_field	<uint32_t>	("session_id_len");
						d.add_field	<char*>		("session_id", d.get_field("session_id_len"));
						d.add_field	<uint32_t>	("fingerprint_len");
						d.add_field	<char*>		("fingerprint", d.get_field("fingerprint_len"));
						d.add_field	<uint32_t>	("locale");
						d.add_field	<uint32_t>	("third_party_auth_ticket");
						d.add_field	<uint32_t>	("third_party_user_id");
						d.add_field	<uint32_t>	("third_party_id");

						//std::cout << "\n" << d << "\n";

						loginserver_.login_reply(sender_id);
					}
					break;
					default:;
					}

					app_data.clear();
					next_sequence_index = 0;
					sequence_count = 0;
				}
			}
			break;
			case disconnect:
			{
				p.set_name("disconnect");
				p.add_field<std::uint32_t>("session_id");
				p.add_field<std::uint16_t>("disconnect_reason_id");

				spdlog::info("0x{:x} disconnected with reason: {}",
					p.get_field(1),
					p.get_field(2)
				);

				// #todo: erase the session maybe send the disconnect reason to connected clients ?
			}
			break;
			case ping:
			{
				loginserver_.pong(sender_id);
			}
			break;
			default:;
			}
		}
		catch (std::exception& e)
		{
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
