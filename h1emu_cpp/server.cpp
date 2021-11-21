#include "server.h"

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

void server::run() {

	io_service.run();
}

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
		auto sender_id = boost::lexical_cast<std::string>(remote_endpoint_.port());

		Poco::MemoryInputStream in_stream((char*)recv_buffer_.data(), bytes_transferred);
		Poco::BinaryReader packet_in(in_stream, Poco::BinaryReader::NETWORK_BYTE_ORDER);

		std::uint16_t op_code;
		packet_in >> op_code;
		
		switch (op_code)
		{
		case session_req:
		{
			std::uint32_t client_crc_length;
			std::uint32_t client_session_id;
			std::uint32_t client_udp_length;
			std::string client_protocol;

			packet_in >> client_crc_length;
			packet_in >> client_session_id;
			packet_in >> client_udp_length;
			packet_in.readRaw(11, client_protocol);

			spdlog::warn("SessionRequest (crc_lenght: {}, session_id: {}, udp_length: {}, client_protocol: {})",
				client_crc_length,
				client_session_id,
				client_udp_length,
				client_protocol
			);

			// check if the protocol match the one we support.
			if (client_protocol != "LoginUdp_11")
			{
				spdlog::error("couldn't start a session for the given entity (protocol mismatch)");
				break;
			}

			new_session.reset(new session(socket_, remote_endpoint_, zone_));
			new_session->start();

			zone_.session_reply(sender_id, client_session_id);
		}
			break;
		case e_soe_op_code::multi:
			spdlog::info("MultiPacket");
			break;
		case e_soe_op_code::disconnect:
		{
			std::uint32_t	session_id;
			std::uint16_t client_disconnect_reason_id;

			packet_in >> session_id;
			packet_in >> client_disconnect_reason_id;

			spdlog::warn("{} disconnected with reason: {}", sender_id, client_disconnect_reason_id);
		}
			break;
		case e_soe_op_code::ping:
			spdlog::info("Ping");
			break;
		case e_soe_op_code::data:
		{
			spdlog::info("Data");

			static std::string data_sequence;

			std::uint16_t packet_seq;
			std::uint32_t packet_size = bytes_transferred - sizeof(std::uint16_t) - sizeof(std::uint32_t); // packet_size - op_code uint16 size - xor uint32 size
			std::string packet_data;
			std::uint16_t packet_crc;

			packet_in >> packet_seq;
			packet_in.readRaw(packet_size, packet_data);
			packet_in >> packet_crc;
			spdlog::debug("{} -> seq: {} crc: 0x{:X} data_size: {}", sender_id, packet_seq, packet_crc, packet_size);
		}
			break;
		case e_soe_op_code::data_fragmented:
		{
			static std::string data_sequence;

			std::uint16_t packet_seq;
			std::uint32_t packet_size = bytes_transferred - sizeof(std::uint16_t) - sizeof(std::uint32_t); // packet_size - op_code uint16 size - xor uint32 size
			std::string packet_data;
			std::uint16_t packet_crc;

			packet_in >> packet_seq;
			packet_in.readRaw(packet_size, packet_data);
			packet_in >> packet_crc;
			spdlog::warn("{} -> seq: {} crc: 0x{:X} data_size: {}", sender_id, packet_seq, packet_crc, packet_size);

			//check the sequence of the packet
			if (packet_seq == 0)
				data_sequence.clear();

			data_sequence.append(packet_data);

			if (packet_seq == 2)
			{
				Poco::MemoryInputStream data_stream((char*)data_sequence.data(), data_sequence.size());
				Poco::BinaryReader data_in(data_stream, Poco::BinaryReader::NETWORK_BYTE_ORDER);

				std::uint16_t data_op_code;
				data_in >> data_op_code;

				spdlog::warn("data -> op_code: 0x{:X} (size: {})", data_op_code, data_sequence.size());

				Poco::Buffer<char> buffer(1024);
				Poco::Buffer<char> buffer2(1024);
				Poco::MemoryOutputStream ostr(buffer.begin(), sizeof(buffer));
				Poco::MemoryOutputStream ostr2(buffer2.begin(), sizeof(buffer2));

				Poco::BinaryWriter writer(ostr, Poco::BinaryWriter::NETWORK_BYTE_ORDER);
				Poco::BinaryWriter writer2(ostr2, Poco::BinaryWriter::NETWORK_BYTE_ORDER);

				writer << std::uint16_t(0x15);
				writer << std::uint16_t(0);
				writer << std::uint32_t(0xb303);

				zone_.deliver(std::string(buffer.begin(), static_cast<std::string::size_type>(ostr.charsWritten())));

				writer << std::uint16_t(0x15);
				writer << std::uint16_t(1);
				writer << std::uint32_t(0xb303);

				zone_.deliver(std::string(buffer2.begin(), static_cast<std::string::size_type>(ostr2.charsWritten())));


			}

			
		}
			break;
		case e_soe_op_code::out_of_order:
			break;
		case e_soe_op_code::ack_reliable_data:
			break;
		case e_soe_op_code::multi_message:
			spdlog::info("MultiAppPacket");
			break;
		}
			//zone_.update_position(boost::lexical_cast<std::string>(remote_endpoint_.port()) , { x, y });
		//

		start_receive();
	}
}

void server::handle_send(boost::shared_ptr<std::string> message,
	const boost::system::error_code& error,
	std::size_t bytes_transferre)
{
	if (!error) {
		std::cout << *message << " was sent" << std::endl;
	}
	else {
		std::cout << error.message() << std::endl;
	}
}