#include <ctime>
#include <iostream>
#include <string>
#include "server.h"
#include "RC4.h"

void printBufferHex(const uint8_t* buffer, size_t length) {
	for (size_t i = 0; i < length; i++) {
		printf("%02x ", buffer[i]);
	}
	printf("\n");
}

void handle_packet_test(uint8_t* buffer, size_t size)
{
	Packet p(buffer, size);

	p.add_field<std::uint16_t>("soe_op_code");
	spdlog::info("handle_packet: {:x} bytes: {}", p.get_field("soe_op_code"), size);

	switch (p.get_field("soe_op_code"))
	{
	case chl_data_a:
	{
		static std::string app_data;
		static std::uint16_t sequence_count = 0;
		static std::uint16_t next_sequence_index = 0;

		p.add_field<std::uint16_t>("seq_num");

		auto app_data_length = p.bytes_left();

		p.add_field<unsigned char*>("app_data", app_data_length);

		char app_data_frag[UDP_LENGTH];

		p.get_field("app_data", app_data_frag);

		app_data.append(app_data_frag, app_data_length);
		SOE::rc4_crypt((unsigned char*)app_data.data(), app_data.length());
		
		//std::cout << "\n" << p << "\n";
		printBufferHex((unsigned char*)app_data.data(), app_data.length());


		boost::array<unsigned char, 400> soe_buffer;

		Packet appdata((uint8_t*)app_data.data(), 300, 0);

		appdata.set_name("login_rep");

		appdata.add_field	<uint8_t>("h1z1_packet_id");
		appdata.add_field	<uint8_t>("logged_in");
		appdata.add_field	<uint32_t>("status");
		appdata.add_field	<uint32_t>("result_code");
		appdata.add_field	<uint8_t>("is_member");
		appdata.add_field	<uint8_t>("is_internal");
		appdata.add_field	<uint32_t>("namespace_len");
		appdata.add_field	<char*>("namespace", appdata.get_field("namespace_len"));
		{
			appdata.add_field<char*>("account_features", 100);
			auto& account_features = appdata.sub_packet("account_features");
			account_features.add_field	<uint32_t>("key");
			{
				account_features.add_field<char*>("account_feature", 40);
				auto& account_feature = account_features.sub_packet("account_feature");
				account_feature.add_field	<uint32_t>("id");
				account_feature.add_field	<uint8_t>("active");
				account_feature.add_field	<uint32_t>("remaining_count");
				account_feature.add_field	<uint32_t>("rawdata_len");
				account_feature.add_field	<char*>("rawdata", 2);
			}
		}
		/*{
			appdata.add_field<char*>("error_details", 22);
			auto& error_details = appdata.sub_packet("error_details");
			error_details.add_field	<uint32_t>("unk_dword");
			error_details.add_field	<uint32_t>("name_len");
			error_details.add_field	<char*>("name", error_details.get_field("name_len"));
			error_details.add_field	<uint32_t>("value_len");
			error_details.add_field	<char*>("value", error_details.get_field("value_len"));
		}
		appdata.add_field	<uint32_t>("ip_country_code_len");
		appdata.add_field	<char*>("ip_country_code", appdata.get_field("ip_country_code_len"));
		appdata.add_field	<uint32_t>("application_payload_len");
		appdata.add_field	<char*>("application_payload", appdata.get_field("application_payload_len"));*/

		std::cout << "\n" << appdata << "\n";

	}
	}
}

int main() {

	boost::array<unsigned char, 92> test = { 
	0x00, 0x03, 0x54, 0x00, 0x09, 0x00, 0x00, 0xa9, 0xb7, 0xb9, 0x43, 0xf1, 0x40, 0xa4, 0x04, 0x8c
, 0x13, 0x22, 0x56, 0x65, 0xcc, 0xa8, 0x1a, 0x20, 0x77, 0xb1, 0xb6, 0x29, 0x74, 0xbf, 0xfb, 0x8f
, 0xcd, 0x0a, 0xab, 0xec, 0xed, 0xca, 0xe5, 0x20, 0xa2, 0x87, 0xec, 0x7e, 0x03, 0x79, 0xde, 0x1e
, 0xb4, 0xf6, 0xfb, 0x93, 0x15, 0xdd, 0xde, 0x1f, 0xa6, 0x71, 0xf8, 0x26, 0x79, 0xa4, 0x6f, 0x0a
, 0xf6, 0x26, 0x67, 0x4f, 0xaf, 0xe1, 0x66, 0x75, 0x28, 0x6a, 0x05, 0x58, 0xdc, 0x4f, 0x34, 0x7f
, 0xfd, 0x23, 0xfb, 0x67, 0x8f, 0xb2, 0xce, 0x04, 0x00, 0x15, 0x00, 0x01 };

	Packet p(test.data(), sizeof(test), 1);

	p.set_name("multi_packet");
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

		handle_packet_test((uint8_t*)subpacket_data.data(), p.get_field(subpacket_len_str));
	}


	//std::string ip = "127.0.0.1";
	//std::uint16_t port = 1115;
	//try
	//{
	//	spdlog::info("starting the udp server");

	//	server server(ip, port);
	//	spdlog::info("initiating server on '{}:{}'", ip, port);

	//	server.run();
	//}
	//catch (std::exception& e)
	//{
	//	std::cerr << e.what() << std::endl;
	//}

	return 0;
}