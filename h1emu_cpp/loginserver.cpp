#include "loginserver.h"
#include "RC4.h"
#include "CRC32.h"


void loginserver::join(boost::shared_ptr<entity> new_entity)
{
	// todo: send packets to existing entities about the new entity being created
	for (auto entity : entities)
	{
	}

	entities.push_back(new_entity);
}

void loginserver::leave(boost::shared_ptr<entity> entity)
{
	/*todo: 
		procedure to delete the entity from the loginserver correctly
		and let everyone knows.

			entities.erase(entity);

	*/
}

void loginserver::update_entities()
{
	auto clientmessage = boost::lexical_cast<std::string>(entities.size());
	std::for_each(entities.begin(), entities.end(),
	              boost::bind(&entity::deliver, _1, clientmessage));
}

void loginserver::update_position(std::string sender_id, vector3 position)
{
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i]->get_id() == sender_id)
		{
			player& data = entities[i]->get_player_data();
			data.position = position;
		}
		else
		{
			// we let entities know the 'position' variable of 'sender_id' changed
			std::stringstream ss;

			entities[i]->deliver(ss.str());
		}
	}
}

void loginserver::ack(const std::string sender_id, const std::uint16_t sequence_index)
{
	for (const auto& entity : entities)
	{
		boost::array<unsigned char, 5> out_buffer_;

		Packet o(out_buffer_.data(), 5);

		o.add_field<std::uint16_t>("soe_packet_id");
		o.add_field<std::uint8_t>("comp_flag");
		o.add_field<std::uint16_t>("sequence");

		o.set_field("soe_packet_id", ack_a);
		o.set_field("comp_flag", 0);
		o.set_field("sequence", sequence_index);

		std::cout << o << "\n";

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

void loginserver::session_reply(const std::string sender_id, const std::uint32_t conn_id)
{
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i]->get_id() == sender_id)
		{
			boost::array<unsigned char, 21> out_buffer_;

			Packet o(out_buffer_.data(), 21, 1);

			o.set_name("session_reply");
			o.add_field<std::uint16_t>("soe_packet_id");
			o.add_field<std::uint32_t>("conn_id");
			o.add_field<std::uint32_t>("crc_seed");
			o.add_field<std::uint8_t>("crc_lenght");
			o.add_field<std::uint8_t>("zflag");
			o.add_field<std::uint8_t>("enc_flag");
			o.add_field<std::uint32_t>("buf_size");
			o.add_field<std::uint32_t>("soe_proto_version");

			// <== set either by the name or index ==>
			o.set_field("soe_packet_id", session_rep);
			o.set_field("conn_id", conn_id);
			o.set_field("crc_seed", 0);
			o.set_field("crc_lenght", 0);
			o.set_field("zflag", 0);
			o.set_field("enc_flag", 1);
			o.set_field("buf_size", 512);
			o.set_field("soe_proto_version", 3);

			//std::cout << o << "\n";

			entities[i]->deliver(std::string(out_buffer_.begin(), out_buffer_.end()));
		}
	}
}

void loginserver::login_reply(const std::string& sender_id)
{
	for (auto& entity : entities)
	{
		if (entity->get_id() == sender_id)
		{
			boost::array<unsigned char, 400> soe_buffer;
			boost::array<unsigned char, 200> appdata_buffer;

			Packet appdata(appdata_buffer.data(), 200, 1);

			appdata.set_name("login_rep");

			appdata.add_field	<uint8_t>	("h1z1_packet_id");
			appdata.add_field	<uint8_t>	("logged_in");
			appdata.add_field	<uint32_t>	("status");
			appdata.add_field	<uint32_t>	("result_code");
			appdata.add_field	<uint8_t>	("is_member");
			appdata.add_field	<uint8_t>	("is_internal");
			appdata.add_field	<uint32_t>	("namespace_len");
			appdata.add_field	<char*>		("namespace", 3);
			{
				appdata.add_field<char*>("account_features", 21);
				auto& account_features = appdata.sub_packet("account_features");
				account_features.add_field	<uint32_t>	("key");
				{
					account_features.add_field<char*>("account_feature", 17);
					auto& account_feature = account_features.sub_packet("account_feature");
					account_feature.add_field	<uint32_t>	("id");
					account_feature.add_field	<uint8_t>	("active");
					account_feature.add_field	<uint32_t>	("remaining_count");
					account_feature.add_field	<uint32_t>	("rawdata_len");
					account_feature.add_field	<char*>		("rawdata", 4);
				}
			}
			{
				appdata.add_field<char*>("error_details", 22);
				auto& error_details = appdata.sub_packet("error_details");
				error_details.add_field	<uint32_t>	("unk_dword");
				error_details.add_field	<uint32_t>	("name_len");
				error_details.add_field	<char*>		("name", 4);
				error_details.add_field	<uint32_t>	("value_len");
				error_details.add_field	<char*>		("value", 4);
			}
			appdata.add_field	<uint32_t>	("ip_country_code_len");
			appdata.add_field	<char*>		("ip_country_code", 2);
			appdata.add_field	<uint32_t>	("application_payload_len");
			appdata.add_field	<char*>		("application_payload", 3);

			// set the LoginRep fields
			appdata.set_field("h1z1_packet_id",			login_rep);
			appdata.set_field("logged_in",				0);
			appdata.set_field("status",					1);
			appdata.set_field("result_code",			1);
			appdata.set_field("is_member",				0);
			appdata.set_field("is_internal",			0);
			appdata.set_field("namespace_len",			3);
			appdata.set_field("namespace",				"soe");

			appdata.sub_packet("account_features").set_field("key", 1);
			appdata.sub_packet("account_features").sub_packet("account_feature").set_field("id", 2);
			appdata.sub_packet("account_features").sub_packet("account_feature").set_field("active", true);
			appdata.sub_packet("account_features").sub_packet("account_feature").set_field("remaining_count", 2);
			appdata.sub_packet("account_features").sub_packet("account_feature").set_field("rawdata_len", 4);
			appdata.sub_packet("account_features").sub_packet("account_feature").set_field("rawdata", "test");
			
			appdata.sub_packet("error_details").set_field("unk_dword", 0);
			appdata.sub_packet("error_details").set_field("name_len", 4);
			appdata.sub_packet("error_details").set_field("name", "None");
			appdata.sub_packet("error_details").set_field("value_len", 4);
			appdata.sub_packet("error_details").set_field("value", "None");

			appdata.set_field("ip_country_code_len", 2);
			appdata.set_field("ip_country_code", "US");
			appdata.set_field("application_payload_len", 2);
			appdata.set_field("application_payload", "US");

			//std::cout << appdata << "\n";

			// encrypt the appdata content
			SOE::rc4_crypt(appdata_buffer.data(), appdata_buffer.size());

			// prepare soe packet
			Packet soe(soe_buffer.data(), soe_buffer.size(), 1);
			soe.set_name("soe::data");
			soe.add_field<std::uint16_t>("soe_packet_id");
			soe.add_field<char*>("packed_data", appdata_buffer.size());

			soe.set_field("soe_packet_id", multi);

			soe.set_field("packed_data", appdata_buffer.data(), appdata_buffer.size());

			//std::cout << soe << "\n";

			entity->deliver(std::string(soe_buffer.begin(), soe_buffer.end()));
		}
	}
}

void loginserver::pong(const std::string sender_id)
{
	for (const auto& entity : entities)
	{
		if (entity->get_id() == sender_id)
		{
			boost::array<unsigned char, 3> out_buffer_;

			Packet soe(out_buffer_.data(), 3, 1);

			soe.set_name("soe::ping");
			soe.add_field<std::uint16_t>("soe_packet_id");
			soe.add_field<std::uint8_t>("comp_flag");

			soe.set_field("soe_packet_id", ping);
			soe.set_field("comp_flag", 0);

			entity->deliver(std::string(out_buffer_.begin(), out_buffer_.end()));
		}
	}
}