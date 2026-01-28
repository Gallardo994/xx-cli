#include "detail/parser.hpp"
#include "third_party/toml.hpp"

namespace xxlib::parser {
    std::expected<std::string, std::string> read_file(const std::string& path) {
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                return std::unexpected("Failed to open file: " + path);
            }
            
            file.seekg(0, std::ios::end);
            std::streamsize size = file.tellg();
            if (size > 1024 * 1024) {
                return std::unexpected("File size exceeds 1 MB limit: " + path);
            }
            file.seekg(0, std::ios::beg);

            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            return content;
        } catch (const std::exception& e) {
            return std::unexpected(std::string("Error reading file: ") + e.what());
        }
    }
    
    std::expected<std::vector<Command>, std::string> parse_buffer(const std::string& buffer) {
        return std::unexpected("Not implemented yet");
    };
}
