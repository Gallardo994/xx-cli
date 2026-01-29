#ifndef XXLIB_PARSER_HPP
#define XXLIB_PARSER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <expected>

namespace xxlib {
	struct Command {
		std::string name{};
		std::vector<std::string> cmd{};
		std::unordered_map<std::string, std::string> envs{};
		std::vector<std::pair<std::string, std::string>> constraints{};
	};

	namespace parser {
		std::expected<std::string, std::string> read_file(const std::string& path);
		std::expected<std::vector<Command>, std::string> parse_buffer(const std::string& buffer);
	} // namespace parser
} // namespace xxlib

#endif // XXLIB_PARSER_HPP
