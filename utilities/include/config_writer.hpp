//
// Created by JellyfishKnight on 25-3-2.
//

#ifndef FILE_WRITER_HPP
#define FILE_WRITER_HPP

#include <filesystem>
#include <fstream>
#include <optional>
#include "json.hpp"
#include <utility>
#include "logger.hpp"

using json = nlohmann::json;

// 检查类型是否可以被序列化为JSON
template<typename T>
concept JSONSerializable = requires(json& j, const T& t) {
  { to_json(j, t) } -> std::same_as<void>;
};

// 检查类型是否可以从JSON反序列化
template<typename T>
concept JSONDeserializable = requires(const json& j, T& t) {
  { from_json(j, t) } -> std::same_as<void>;
};

// 检查类型是否同时支持序列化和反序列化
template<typename T>
concept JSONReflectable = JSONSerializable<T> && JSONDeserializable<T>;


class ConfigWriter {
public:
  explicit ConfigWriter(std::string file_path) : file_path_(std::move(file_path))
  {
    // create file if not exists
    if (!std::filesystem::exists(file_path_)) {
      std::ofstream file(file_path_);
      if (!file.is_open()) {
        LOG_ERROR("错误： 无法创建wifi缓存文件！");
        return;
      }
    }
  }

  template<typename T>
    requires JSONReflectable<T>
  bool write_config(T t)
  {
    std::ofstream out_file(file_path_);
    if (!out_file.is_open())
    {
      return false;
    }
    json json_format_t = t;
    out_file << json_format_t;
    if (out_file.fail())
    {
      return false;
    }
    return true;
  }

  template<typename T>
    requires JSONReflectable<T>
  T get_config()
  {
    std::ifstream in_file(file_path_);
    if (!in_file.is_open())
    {
      return T{};
    }
    json json_obj;
    try
    {
      in_file.seekg(std::ios_base::beg);
      in_file >> json_obj;
      if (json_obj.empty())
      {
        return T{};
      }
      return json_obj.get<T>();
    } catch (const std::exception& e)
    {
      LOG_ERROR("错误： 读取配置文件失败: {}", e.what());
      return T{};
    }
  }

private:
  std::string file_path_;
};



#endif //FILE_WRITER_HPP
