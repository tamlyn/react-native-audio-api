#pragma once

#include <string>
#include <variant>
#include <optional>
#include <map>
#include <utility>
#include <cstdint>

namespace audioapi {

using MetaValue = std::variant<int64_t, double, std::string, bool>;
using MetaMap   = std::map<std::string, MetaValue>;

template<typename T>
class ReturnStatus {
 public:
  enum class Status {
    Success = 0,
    Error,
  };

  static ReturnStatus Success(T value, const std::string &message = "") {
    ReturnStatus s;
    s.status_  = Status::Success;
    s.message_ = message;
    s.value_   = std::move(value);
    return s;
  }

  static ReturnStatus Error(const std::string &message) {
    ReturnStatus s;
    s.status_  = Status::Error;
    s.message_ = message;
    return s;
  }

  Status getStatus() const { return status_; }
  bool isSuccess() const { return status_ == Status::Success; }
  bool isError() const { return status_ == Status::Error; }

  const std::string &getMessage() const { return message_; }

  T &getValue() { return *value_; }
  const T &getValue() const { return *value_; }

  bool hasValue() const { return value_.has_value(); }

  MetaMap &getMeta() { return meta_; }
  const MetaMap &getMeta() const { return meta_; }

  template<typename U>
  void setMetaValue(const std::string &key, U &&value) {
    meta_[key] = MetaValue(std::forward<U>(value));
  }

  template<typename U>
  const U *getMetaValue(const std::string &key) const {
    auto it = meta_.find(key);
    if (it == meta_.end()) return nullptr;
    return std::get_if<U>(&it->second);
  }

 private:
  Status status_{Status::Error};
  std::string message_;
  std::optional<T> value_;
  MetaMap meta_;
};

template<>
class ReturnStatus<void> {
 public:
  enum class Status {
    Success = 0,
    Error,
  };

  static ReturnStatus Success(const std::string &message = "") {
    ReturnStatus s;
    s.status_  = Status::Success;
    s.message_ = message;
    return s;
  }

  static ReturnStatus Error(const std::string &message) {
    ReturnStatus s;
    s.status_  = Status::Error;
    s.message_ = message;
    return s;
  }

  Status getStatus() const { return status_; }
  bool isSuccess() const { return status_ == Status::Success; }
  bool isError() const { return status_ == Status::Error; }

  const std::string &getMessage() const { return message_; }

  MetaMap &getMeta() { return meta_; }
  const MetaMap &getMeta() const { return meta_; }

  template<typename U>
  void setMetaValue(const std::string &key, U &&value) {
    meta_[key] = MetaValue(std::forward<U>(value));
  }

  template<typename U>
  const U *getMetaValue(const std::string &key) const {
    auto it = meta_.find(key);
    if (it == meta_.end()) return nullptr;
    return std::get_if<U>(&it->second);
  }

 private:
  Status status_{Status::Error};
  std::string message_;
  MetaMap meta_;
};

} // namespace audioapi
