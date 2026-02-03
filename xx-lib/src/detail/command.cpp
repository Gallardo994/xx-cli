#include "detail/command.hpp"

#include <numeric>
#include <string>
#include <spdlog/spdlog.h>

namespace xxlib {
	namespace command {
		template <typename T> std::string join_vector(const std::vector<T>& vec, const std::string& separator) {
			if (vec.empty()) {
				return "";
			}

			return std::accumulate(std::next(vec.begin()), vec.end(), vec[0], [&separator](const std::string& a, const T& b) {
				return a + separator + b;
			});
		}

		std::string join_cmd(const Command& command) {
			return join_vector(command.cmd, " ");
		}

		std::string join_constraints(const Command& command) {
			if (command.constraints.empty()) {
				return "";
			}

			std::vector<std::string> parts;
			parts.reserve(command.constraints.size());

			for (const auto& [key, value] : command.constraints) {
				parts.push_back(key + "=" + value);
			}

			return join_vector(parts, ", ");
		}
	} // namespace command
} // namespace xxlib
