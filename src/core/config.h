#pragma once

#include <string>
#include <map>
#include <mutex>

namespace fml {

class Config {
public:
    static Config& instance();

    void load(const std::string& path);
    void save(const std::string& path);

    std::string get(const std::string& key) const;
    void set(const std::string& key, const std::string& value);
    bool has(const std::string& key) const;
    void remove(const std::string& key);

    const std::map<std::string, std::string>& getAll() const;

private:
    Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::map<std::string, std::string> data_;
    mutable std::mutex mutex_;
    std::string filePath_;
};

} // namespace fml