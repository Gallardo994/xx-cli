#ifndef XXLIB_PARSER_HPP
#define XXLIB_PARSER_HPP

#include "detail/command.hpp"
#include <string>
#include <expected>

namespace xxlib::parser {
	std::expected<std::string, std::string> read_file(const std::string& path);
	std::expected<std::vector<Command>, std::string> parse_buffer(const std::string& buffer);
} // namespace xxlib::parser

#endif // XXLIB_PARSER_HPP
