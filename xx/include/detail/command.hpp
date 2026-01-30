#ifndef XX_COMMAND_HPP
#define XX_COMMAND_HPP

#include <string>
#include <vector>
#include <unordered_map>

struct Command {
	std::string name{};
	std::vector<std::string> cmd{};
	std::unordered_map<std::string, std::string> envs{};
	std::vector<std::pair<std::string, std::string>> constraints{};
};

#endif // XX_COMMAND_HPP
