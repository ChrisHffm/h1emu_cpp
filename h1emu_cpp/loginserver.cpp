#include "loginserver.h"
#include "RC4.h"
#include "CRC32.h"

void loginserver::join(boost::shared_ptr<entity> new_entity)
{
	// todo: send packets to existing entities about the new entity being created
	for (auto entity : entities) {

	}

	entities.push_back(new_entity);
}

void loginserver::leave(boost::shared_ptr<entity> entity) {
	/*todo: 
		procedure to delete the entity from the loginserver correctly
		and let everyone knows.

			entities.erase(entity);

	*/
}

void loginserver::update_entities() {
	std::string clientmessage = boost::lexical_cast<std::string> (entities.size());
	std::for_each(entities.begin(), entities.end(),
		boost::bind(&entity::deliver, _1, clientmessage));
}

void loginserver::update_position(std::string sender_id, vector3 position) {
	for (int i = 0; i < entities.size(); i++) {
		if (entities[i]->get_id() == sender_id) {
			player& data = entities[i]->get_player_data();
			data.position = position;
		}
		else {
			// we let entities know the 'position' variable of 'sender_id' changed
			std::stringstream ss;

			entities[i]->deliver(ss.str());
		}
	}
}

void loginserver::ack(const std::string sender_id, const std::uint16_t sequence_index) {
	
	for (const auto &entity : entities) {
		boost::array<unsigned char, 5> out_buffer_;

		Packet o(out_buffer_.data(), 5);

		o.add_field<std::uint16_t>("soe_packet_id");
		o.add_field<std::uint8_t>("comp_flag");
		o.add_field<std::uint16_t>("sequence");

		o.set_field("soe_packet_id", e_soe_op_code::ack_a);
		o.set_field("comp_flag", 0);
		o.set_field("sequence", sequence_index);

		entity->deliver(std::string(out_buffer_.begin(), out_buffer_.end()));
	}

	//for (int i = 0; i < entities.size(); i++) {
	//	if (entities[i]->get_id() == sender_id) {

	//		boost::array<unsigned char, 5> out_buffer_;

	//		Packet o(out_buffer_.data(), 5);

	//		o.add_field<std::uint16_t>("soe_packet_id");
	//		o.add_field<std::uint8_t>("comp_flag");
	//		o.add_field<std::uint16_t>("sequence");
	//		//o.add_field<std::uint16_t>("crc");

	//		o.set_field("soe_packet_id", e_soe_op_code::ack_a);
	//		o.set_field("comp_flag", 0);
	//		o.set_field("sequence", sequence_index);

	//		entities[i]->deliver(std::string(out_buffer_.begin(), out_buffer_.end()));
	//	}
	//}
}

void loginserver::session_reply(const std::string sender_id, const std::uint32_t session_id) {
	for (int i = 0; i < entities.size(); i++) {
		if (entities[i]->get_id() == sender_id) {

			boost::array<unsigned char, 21> out_buffer_;

			Packet o(out_buffer_.data(), 21, 1);

			o.add_field<std::uint16_t>	("soe_packet_id");
			o.add_field<std::uint32_t>	("session_id");
			o.add_field<std::uint32_t>	("crc_seed");
			o.add_field<std::uint8_t>	("crc_lenght");
			o.add_field<std::uint8_t>	("compression");
			o.add_field<std::uint8_t>	("encryption");
			o.add_field<std::uint32_t>	("udp_lenght");
			o.add_field<std::uint32_t>	("footer");

			// <== set either by the name or index ==>
			o.set_field("soe_packet_id",	e_soe_op_code::session_rep);
			o.set_field("session_id",		session_id);
			o.set_field("crc_seed",			0);
			o.set_field("crc_lenght",		0);
			o.set_field("compression",		1);
			o.set_field("encryption",		0);
			o.set_field("udp_lenght",		512);
			o.set_field("footer",			3);

			entities[i]->deliver(std::string(out_buffer_.begin(), out_buffer_.end()));
		}
	}
}

void loginserver::login_reply(const std::string sender_id) {
	for (int i = 0; i < entities.size(); i++) {
		if (entities[i]->get_id() == sender_id) {

			boost::array<unsigned char, 26> out_buffer_;
			boost::array<unsigned char, 19> h1_buffer_;

			Packet h1(h1_buffer_.data(), 19, 1);

			h1.set_name("h1z1::login_rep");
			h1.add_field<std::uint8_t>("h1z1_packet_id");
			h1.add_field<std::uint8_t>("logged_in");
			h1.add_field<std::uint32_t>("status");
			h1.add_field<std::uint8_t>("is_member");
			h1.add_field<std::uint8_t>("is_internal");
			h1.add_field<std::uint32_t>("sz_1");
			h1.add_field<char*>("namespace", 3);
			h1.add_field<std::uint32_t>("sz_2");

			// set the LoginRep fields
			h1.set_field("h1z1_packet_id",	e_h1z1_op_code::login_rep);
			h1.set_field("logged_in",		1);
			h1.set_field("status",			0x1000000);
			h1.set_field("is_member",		1);
			h1.set_field("is_internal",		1);
			h1.set_field("sz_1",			0x3000000);
			h1.set_field("namespace",		"soe");
			h1.set_field("sz_2",			0);

			std::vector<unsigned char> key = { 0x17, 0xbd, 0x08, 0x6b, 0x1b, 0x94, 0xf0, 0x2f, 0xf0, 0xec, 0x53, 0xd7, 0x63, 0x58, 0x9b, 0x5f };

			SOE::RC4 RC4;
			RC4.Init(key);

			auto data = RC4.Parse((unsigned char*)h1_buffer_.data(), h1_buffer_.size());

			Packet soe(out_buffer_.data(), out_buffer_.size(), 1);
			soe.set_name("soe::data");
			soe.add_field<std::uint16_t>("soe_packet_id");
			soe.add_field<std::uint8_t>("comp_flag");
			soe.add_field<std::uint16_t>("sequence_index");
			soe.add_field<char*>("packed_data", data.size());
			//soe.add_field<std::uint16_t>("crc");

			soe.set_field("soe_packet_id",	e_soe_op_code::chl_data_a);
			soe.set_field("comp_flag", 0);
			soe.set_field("sequence_index", 0);
			soe.set_field("packed_data", data.data(), data.size());
			//soe.set_field("crc", SOE::append_crc(out_buffer_.data(), soe.get_field_offset("crc"), 0));

			entities[i]->deliver(std::string(out_buffer_.begin(), out_buffer_.end()));
		}
	}
}

void loginserver::pong(const std::string sender_id) {
	for (auto entity : entities) {
		if (entity->get_id() == sender_id) {

			boost::array<unsigned char, 3> out_buffer_;

			Packet soe(out_buffer_.data(), 3, 1);

			soe.set_name("soe::ping");
			soe.add_field<std::uint16_t>("soe_packet_id");
			soe.add_field<std::uint8_t>("comp_flag");

			soe.set_field("soe_packet_id",	e_soe_op_code::ping);
			soe.set_field("comp_flag",		0);

			entity->deliver(std::string(out_buffer_.begin(), out_buffer_.end()));
		}
	}
}