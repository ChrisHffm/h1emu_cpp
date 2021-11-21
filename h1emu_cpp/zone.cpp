#include "zone.h"

// zone::join - handle the creation of the session for the new connected entity
void zone::join(boost::shared_ptr<entity> new_entity)
{
	spdlog::info("started a session for {}", new_entity->get_id());

	// todo: send packets to existing entities about the new entity being created
	for (auto entity : entities) {

	}

	entities.push_back(new_entity);
}

void zone::leave(boost::shared_ptr<entity> entity) {
	/*todo: 
		procedure to delete the entity from the zone correctly
		and let everyone knows.

			entities.erase(entity);

	*/
}

void zone::deliver(std::string message) {
	for (auto entity : entities) {
		entity->deliver(message);
	}
}

void zone::update_entities() {
	std::string clientmessage = boost::lexical_cast<std::string> (entities.size());
	std::for_each(entities.begin(), entities.end(),
		boost::bind(&entity::deliver, _1, clientmessage));
}

void zone::update_position(std::string sender_id, vector3 position) {
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

void zone::update_rotation(std::string sender_id, vector3 rotation) {
	for (int i = 0; i < entities.size(); i++) {
		// set the sender new 'rotation' data
		if (entities[i]->get_id() == sender_id) {
			player& data = entities[i]->get_player_data();
			data.rotation = rotation;
		}
		else {
			// we let entities know the 'rotation' variable of 'sender_id' changed
			std::stringstream ss;

			entities[i]->deliver(ss.str());
		}
	}
}

//
void zone::update_looking_at(const std::string sender_id, vector3 looking_at) {
	for (int i = 0; i < entities.size(); i++) {
		// set the sender new 'looking_at' data
		if (entities[i]->get_id() == sender_id) {
			player& data = entities[i]->get_player_data();
			data.looking_at = looking_at;
		}
		else {
			// we let entities know the 'looking_at' variable of 'sender_id' changed
			std::stringstream ss;

			entities[i]->deliver(ss.str());
		}
	}
}

void zone::session_reply(const std::string sender_id, const std::uint32_t session_id) {
	for (int i = 0; i < entities.size(); i++) {
		if (entities[i]->get_id() == sender_id) {
			Poco::Buffer<char> buffer(512);

			Poco::MemoryOutputStream ostr(buffer.begin(), sizeof(buffer));
			Poco::BinaryWriter writer(ostr, Poco::BinaryWriter::NETWORK_BYTE_ORDER);

			writer << e_soe_op_code::net_status_rep;	//  op_code
			writer << session_id;						//	session_id
			writer << std::uint32_t(0);					//	crc_seed
			writer << std::uint8_t(2);					//	crc_lenght;
			writer << std::uint8_t(0);					//	use_compression
			writer << std::uint8_t(0);					//	use_encryption
			writer << std::uint32_t(512);				//	server_udp_size;
			writer << std::uint8_t(3);

			entities[i]->deliver(std::string(buffer.begin(), static_cast<std::string::size_type>(ostr.charsWritten())));

		}
	}
}