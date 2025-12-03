#include <utility>
#include <string>
#include <stdexcept>
#include <functional>
#include <new>
#include <type_traits>

/// @brief A Result type that can represent either a success (Ok) or an error (Err).
/// @tparam T value type for success
/// @tparam E error type for failure
/// @note Specializations for void T and/or void E are provided.
///
/// It has so much templates so it is hard to read, but it is worth it.
/// methods:
/// - is_ok() -> bool
/// - is_err() -> bool
/// - expect(msg: string) -> T // throws if Err with msg
/// - unwrap() -> T // throws if Err
/// - unwrap_or(default: T) -> T // returns default if Err
/// - unwrap_or_else(func: function(E) -> T) -> T // calls func if Err
/// - unwrap_unchecked() -> T // no checks, undefined behavior if Err
/// - expect_err(msg: string) -> E // throws if Ok with msg
/// - unwrap_err() -> E // throws if Ok
/// - unwrap_err_unchecked() -> E // no checks, undefined behavior if Ok
/// - take() -> T // moves out T if Ok, throws if Err
/// - take_err() -> E // moves out E if Err, throws if Ok
///
/// Factory methods:
/// - Ok(value: T) -> Result<T, E>
/// - Ok() -> Result<void, E> // for void T
/// - Err(error: E) -> Result<T, E>
/// - Err() -> Result<T, void> // for void E
///
/// Design inspired by Rust's Result type.
/// https://doc.rust-lang.org/std/result/
template<typename T, typename E>
class Result {
  struct OkTag {};
  struct ErrTag {};

  template<typename TP, typename EP>
  struct Payload {
    union {
      TP ok_value;
      EP err_value;
    };
    bool is_ok;

    Payload(OkTag, const TP& v) : ok_value(v), is_ok(true) {}
    Payload(OkTag, TP&& v) : ok_value(std::move(v)), is_ok(true) {}
    Payload(ErrTag, const EP& v) : err_value(v), is_ok(false) {}
    Payload(ErrTag, EP&& v) : err_value(std::move(v)), is_ok(false) {}

    ~Payload() {
      if (is_ok) ok_value.~TP();
      else err_value.~EP();
    }

    Payload(const Payload& other) : is_ok(other.is_ok) {
      if (is_ok) new (&ok_value) TP(other.ok_value);
      else new (&err_value) EP(other.err_value);
    }

    Payload(Payload&& other) : is_ok(other.is_ok) {
      if (is_ok) new (&ok_value) TP(std::move(other.ok_value));
      else new (&err_value) EP(std::move(other.err_value));
    }

    TP val() {
      return ok_value;
    }

    EP err() {
      return err_value;
    }
  };

  template<typename TP>
  struct Payload<TP, void> {
    union { TP ok_value; };
    bool is_ok;

    Payload(OkTag, const TP& v) : ok_value(v), is_ok(true) {}
    Payload(OkTag, TP&& v) : ok_value(std::move(v)), is_ok(true) {}
    Payload(ErrTag) : is_ok(false) {}

    ~Payload() {
      if (is_ok) ok_value.~TP();
    }

    Payload(const Payload& other) : is_ok(other.is_ok) {
      if (is_ok) new (&ok_value) TP(other.ok_value);
    }

    Payload(Payload&& other) : is_ok(other.is_ok) {
      if (is_ok) new (&ok_value) TP(std::move(other.ok_value));
    }

    TP val() {
      return ok_value;
    }

    void err() {}
  };

  template<typename EP>
  struct Payload<void, EP> {
    union { EP err_value; };
    bool is_ok;

    Payload(OkTag) : is_ok(true) {}
    Payload(ErrTag, const EP& v) : err_value(v), is_ok(false) {}
    Payload(ErrTag, EP&& v) : err_value(std::move(v)), is_ok(false) {}

    ~Payload() {
      if (!is_ok) err_value.~EP();
    }

    Payload(const Payload& other) : is_ok(other.is_ok) {
      if (!is_ok) new (&err_value) EP(other.err_value);
    }

    Payload(Payload&& other) : is_ok(other.is_ok) {
      if (!is_ok) new (&err_value) EP(std::move(other.err_value));
    }

    void val() {}

    EP err() {
      return err_value;
    }
  };

  template<>
  struct Payload<void, void> {
    bool is_ok;
    Payload(OkTag) : is_ok(true) {}
    Payload(ErrTag) : is_ok(false) {}

    void val() {}
    void err() {}
  };

  explicit Result(Payload<T, E>&& payload) : payload_(std::move(payload)) {}

 public:
  Result(const Result<T, E>&) = default;
  Result(Result<T, E>&&) = default;
  Result<T, E>& operator=(const Result<T, E>&) = default;
  Result<T, E>& operator=(Result<T, E>&&) = default;
  ~Result() = default;

  // Ok factories
  template <typename U = T>
  static std::enable_if_t<!std::is_void_v<U>, Result<T, E>> Ok(const U& value) {
    return Result<T, E>(Payload<T, E>(OkTag{}, value));
  }

  template <typename U = T>
  static std::enable_if_t<!std::is_void_v<U>, Result<T, E>> Ok(U&& value) {
     return Result<T, E>(Payload<T, E>(OkTag{}, std::move(value)));
  }

  template <typename U = T>
  static std::enable_if_t<std::is_void_v<U>, Result<T, E>> Ok() {
    return Result<T, E>(Payload<T, E>(OkTag{}));
  }

  // Err factories
  template <typename U = E>
  static std::enable_if_t<!std::is_void_v<U>, Result<T, E>> Err(const U& error) {
    return Result<T, E>(Payload<T, E>(ErrTag{}, error));
  }

  template <typename U = E>
  static std::enable_if_t<!std::is_void_v<U>, Result<T, E>> Err(U&& error) {
    return Result<T, E>(Payload<T, E>(ErrTag{}, std::move(error)));
  }

  template <typename U = E>
  static std::enable_if_t<std::is_void_v<U>, Result<T, E>> Err() {
    return Result<T, E>(Payload<T, E>(ErrTag{}));
  }

  [[nodiscard]] bool is_ok() const {
    return payload_.is_ok;
  }

  [[nodiscard]] bool is_err() const {
    return !payload_.is_ok;
  }

  [[nodiscard]] T expect(const std::string& msg) const {
    if (!payload_.is_ok) {
      throw std::runtime_error(msg);
    }
    return payload_.val();
  }

  template <typename U = T>
  [[nodiscard]] std::enable_if_t<!std::is_void_v<U>, const U&> unwrap() const {
    if (!payload_.is_ok) {
      throw std::runtime_error("Called unwrap on an Err value");
    }
    return payload_.val();
  }

  template <typename U = T>
  [[nodiscard]] std::enable_if_t<!std::is_void_v<U>, U> unwrap_or(const U& default_value) const {
    if (payload_.is_ok) {
      return payload_.val();
    }
    return default_value;
  }

  template<typename U = T, typename V = E>
  [[nodiscard]] std::enable_if_t<!std::is_void_v<U>, U> unwrap_or_else(const std::function<U(const V&)>& func) const {
    if (payload_.is_ok) {
      return payload_.val();
    }
    return func(payload_.err());
  }

  template<typename U = T, typename V = E>
  [[nodiscard]] std::enable_if_t<std::is_void_v<U>, U> unwrap_or_else(const std::function<U()>& func) const {
    if (payload_.is_ok) {
      return payload_.val();
    }
    return func();
  }

  template <typename U = T>
  [[nodiscard]] std::enable_if_t<!std::is_void_v<U>, U> unwrap_unchecked() const noexcept {
    return payload_.val();
  }

  template <typename U = E>
  [[nodiscard]] std::enable_if_t<!std::is_void_v<U>, const U&> unwrap_err() const {
    if (payload_.is_ok) {
      throw std::runtime_error("Called unwrap_err on an Ok value");
    }
    return payload_.err();
  }

  [[nodiscard]] E expect_err(const std::string& msg) const {
    if (payload_.is_ok) {
      throw std::runtime_error(msg);
    }
    return payload_.err();
  }

  [[nodiscard]] E unwrap_err_unchecked() const noexcept {
    return payload_.err();
  }

  // take
  template <typename U = T>
  [[nodiscard]] std::enable_if_t<!std::is_void_v<U>, U&&> take() && {
    if (!payload_.is_ok) {
      throw std::runtime_error("Called take on an Err value");
    }
    return std::move(payload_.val());
  }

  // take_err
  template <typename U = E>
  [[nodiscard]] std::enable_if_t<!std::is_void_v<U>, U&&> take_err() && {
    if (payload_.is_ok) {
      throw std::runtime_error("Called take_err on an Ok value");
    }
    return std::move(payload_.err());
  }

 private:
  Payload<T, E> payload_;
};
