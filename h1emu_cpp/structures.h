#ifndef structures
#define structures
#include <string>

struct vector3 {
	float x = 0; 
	float y = 0;
	float z = 0;
};

struct vector4 {
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 0;
};

enum e_soe_op_code : std::uint16_t
{
	session_req = 0x1,
	session_rep,
	multi,
	disconnect = 0x5,
	ping,
	net_status_req,
	net_status_rep,
	data,
	data_fragmented = 0x0d,
	out_of_order = 0x11,
	ack_reliable_data = 0x15,
	multi_message = 0x19,
	fatal_error = 0x1d,
	fatal_error_rep = 0x1e
};

struct player {
	std::string		id;
	std::string		transient_id;
	std::string		name;

	std::uint32_t	movement_code;

	struct stats {
		std::uint16_t	health;
		std::uint16_t	stamina;
		std::uint16_t	virus;
		std::uint16_t	hunger;
		std::uint16_t	thirst;
	};

	vector3			position;
	vector3			rotation;
	vector3			looking_at;

	player(std::string name) : name(name) {}
};

struct vehicule {
	std::string		id;
	std::string		transient_id;
	std::string		world_id;
	bool            is_managed;
	bool            is_engine_on;
	bool            is_locked;
	vector3			position;
	vector3			rotation;
	vector4         scale;
	std::uint16_t	color;
	std::uint32_t	health;
	std::uint32_t	fuel;
	std::uint32_t	state;
	std::vector<bool> seat;

	vehicule(std::string id) : id(id) {}
};

enum disconnect_reasons : std::uint16_t
{
	disconnect_reason_none,
	disconnect_reason_connection_refused,
	disconnect_reason_too_many_connections,
	disconnect_reason_application,
	disconnect_reason_timeout,
	disconnect_reason_application_released,
	disconnect_reason_socket_error,
	disconnect_reason_socket_error_during_negotiation,
	disconnect_reason_other_side_terminated,
	disconnect_reason_manager_deleted,
	disconnect_reason_connect_error,
	disconnect_reason_connect_fail,
	disconnect_reason_logical_packet_too_short,
	disconnect_reason_logical_packet_too_long,
	disconnect_reason_connect_timeout,
	disconnect_reason_connection_reset,
	disconnect_reason_connection_aborted,
	disconnect_reason_dns_failure,
	disconnect_reason_unable_to_create_socket,
	disconnect_reason_unable_to_configure_socket,
	unknown_reason
};

#endif // !structures