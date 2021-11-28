//#include "player_role.h"
//#include <fstream>
//
//
//void player_role::add_role(const std::string& role_name)
//{
//	if (!role_exist(role_name))
//		this->roles_.push_back(role_name);
//}
//
//void player_role::remove_role(const std::string& role_name)
//{
//	if (!role_exist(role_name)) {
//		this->roles_.erase(std::remove(roles_.begin(), roles_.end(), role_name), roles_.end());
//
//		//todo: remove players's role  
//	}
//}
//
//bool player_role::role_exist(const std::string& role_name)
//{
//	bool role_found;
//	std::find_if(this->roles_.begin(), this->roles_.end(), [&](const std::string& role) {
//		role_found = (role == role_name);
//	});
//
//	return role_found;
//}
//
//void player_role::reload_plugin()
//{
//}
//
//void player_role::set_role(const std::string& user_id, const std::string& role_name)
//{
//	/*
//	 auto user = h1z1->get_user(user_id);
//
//	 user.role = ...
//	 */
//}
//
//void player_role::init()
//{
//	std::ifstream ifs("plugin/player_role/config.json");
//
//	// later parse the file
//}
