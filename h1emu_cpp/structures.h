#ifndef structures
#define structures
#include <string>

#define FIRST_FLAG_SEQ		0
#define UDP_LENGTH			512

/** Supported LoginServer version string */
#define SOE_LS_VERSION		LoginUdp_11
/** Supported GatewayServer version string */
#define SOE_GW_VERSION		ClientProtocol_1080

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
	session_req			= 0x0001,
	session_rep			= 0x0002,
	multi				= 0x0003,
	not_used			= 0x0004,
	disconnect			= 0x0005,
	ping				= 0x0006,
	net_status_req		= 0x0007,
	net_status_rep		= 0x0008,
	chl_data_a			= 0x0009,
	chl_data_b			= 0x000a,
	chl_data_c			= 0x000b,
	chl_data_d			= 0x000c,
	chl_data_frag_a		= 0x000d,
	chl_data_frag_b		= 0x000e,
	chl_data_frag_c		= 0x000f,
	chl_data_frag_d		= 0x0010,
	out_of_order_a		= 0x0011,
	out_of_order_b		= 0x0012,
	out_of_order_c		= 0x0013,
	out_of_order_d		= 0x0014,
	ack_a				= 0x0015,
	ack_b				= 0x0016,
	ack_c				= 0x0017,
	ack_d				= 0x0018,
	multi_a				= 0x0019,
	multi_b				= 0x001a,
	multi_c				= 0x001b,
	multi_d				= 0x001c,
	fatal_error			= 0x001d,
	fatal_error_rep		= 0x001e,

	op_code_count
};

enum e_h1z1_op_code : std::uint8_t
{
	login_req							= 0x1,
	login_rep							= 0x2,
	logout								= 0x3,
	force_disconnect					= 0x4,
	character_create_req				= 0x5,
	character_create_rep				= 0x6,
	character_login_req					= 0x7,
	character_login_rep					= 0x8,
	character_delete_req				= 0x9,
	character_delete_rep				= 0xa,
	character_select_info_req			= 0xb,
	character_select_info_rep			= 0xc,
	server_list_req						= 0xd,
	server_list_rep						= 0xe,
	server_update						= 0xf,
	tunnel_app_packet_client_to_server	= 0x10,
	tunnel_app_packet_server_to_client	= 0x11,
	character_transfer_request			= 0x12,
	character_transfer_reply			= 0x13
};

struct player {
	std::string			id;
	std::string			transient_id;
	std::string			name;

	std::uint32_t		movement_code;

	struct stats {
		std::uint16_t	health;
		std::uint16_t	stamina;
		std::uint16_t	virus;
		std::uint16_t	hunger;
		std::uint16_t	thirst;
	};

	vector3				position;
	vector3				rotation;
	vector3				looking_at;

	player(std::string name) : name(name) {}
};

struct vehicule {
	std::string			id;
	std::string			transient_id;
	std::string			world_id;
	bool				is_managed;
	bool				is_engine_on;
	bool				is_locked;
	vector3				position;
	vector3				rotation;
	vector4				scale;
	std::uint16_t		color;
	std::uint32_t		health;
	std::uint32_t		fuel;
	std::uint32_t		state;
	std::vector<bool>	seat;

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