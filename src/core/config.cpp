#include "config.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace fml {

Config& Config::instance() {
    static Config instance;
    return instance;
}

void Config::load(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    filePath_ = path;

    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    size_t pos = 0;
    while (pos < content.size()) {
        size_t keyStart = content.find('"', pos);
        if (keyStart == std::string::npos) break;
        keyStart++;

        size_t keyEnd = content.find('"', keyStart);
        if (keyEnd == std::string::npos) break;

        std::string key = content.substr(keyStart, keyEnd - keyStart);

        size_t colonPos = content.find(':', keyEnd + 1);
        if (colonPos == std::string::npos) break;

        size_t valStart = content.find('"', colonPos + 1);
        if (valStart == std::string::npos) break;
        valStart++;

        size_t valEnd = content.find('"', valStart);
        if (valEnd == std::string::npos) break;

        std::string value = content.substr(valStart, valEnd - valStart);

        data_[key] = value;
        pos = valEnd + 1;
    }
}

void Config::save(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!path.empty()) {
        filePath_ = path;
    }

    std::ofstream file(filePath_);
    if (!file.is_open()) return;

    file << "{" << std::endl;
    bool first = true;
    for (const auto& [key, value] : data_) {
        if (!first) {
            file << "," << std::endl;
        }
        file << "  \"" << key << "\": \"" << value << "\"";
        first = false;
    }
    file << std::endl << "}" << std::endl;
}

std::string Config::get(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = data_.find(key);
    if (it != data_.end()) {
        return it->second;
    }
    return "";
}

void Config::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    data_[key] = value;
}

bool Config::has(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return data_.find(key) != data_.end();
}

void Config::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    data_.erase(key);
}

const std::map<std::string, std::string>& Config::getAll() const {
    return data_;
}

} // namespace fml