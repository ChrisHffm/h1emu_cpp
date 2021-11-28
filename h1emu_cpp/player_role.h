#ifndef PLAYER_ROLE_H
#define PLAYER_ROLE_H
#include <string>
#include <vector>

class player_role {
public:
	void add_role(const std::string& role_name);

	void remove_role(const std::string& role_name);

	bool role_exist(const std::string& role_name);

	void reload_plugin();

	void set_role(const std::string& user_id, const std::string& role_name);

	void init();

private:

	std::vector<std::string> roles_;
};

#endif