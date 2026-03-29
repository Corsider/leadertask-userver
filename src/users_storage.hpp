#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct UserRecord {
  int64_t id{};
  std::string login;
  std::string password;
  std::string firstName;
  std::string lastName;
};

// struct UserRecord {
//   int64_t id{};
//   std::string login;
//   std::string password;
//   std::string firstName;
//   std::string lastName;
// };

class UsersStorage {
 public:
  std::optional<int64_t> CreateUser(const std::string& login,
                                    const std::string& password,
                                    const std::string& firstName,
                                    const std::string& lastName) {
    std::lock_guard lock(mutex_);

    if (users_by_login_.contains(login)) {
      return std::nullopt;
    }

    const int64_t id = next_id_++;
    UserRecord user{
        .id = id,
        .login = login,
        .password = password,
        .firstName = firstName,
        .lastName = lastName,
    };

    users_by_login_.emplace(login, user);
    users_by_id_.emplace(id, user);
    return id;
  }

  std::optional<UserRecord> GetByLogin(std::string_view login) const {
    std::lock_guard lock(mutex_);

    const auto it = users_by_login_.find(std::string(login));
    if (it == users_by_login_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  std::optional<UserRecord> GetById(int64_t user_id) const {
    std::lock_guard lock(mutex_);
    const auto it = users_by_id_.find(user_id);
    if (it == users_by_id_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  bool UserExists(int64_t user_id) const {
    std::lock_guard lock(mutex_);
    return users_by_id_.contains(user_id);
  }

  //////////////
  std::vector<UserRecord> Search(
      const std::optional<std::string>& login,
      const std::optional<std::string>& first_mask,
      const std::optional<std::string>& last_mask) const {
    std::lock_guard<std::mutex> lock(mutex_);
    //////////////////////////////////////////////////////
    std::vector<UserRecord> res;

    for (const auto& [_, user] : users_by_login_) {
      bool ok = true;
      if (login && user.login != *login) {
        ///
        ok = false;
      }
      if (first_mask && user.firstName.find(*first_mask) == std::string::npos) {
        ok = false;
      }
      if (last_mask && user.lastName.find(*last_mask) == std::string::npos) {
        ok = false;
      }
      if (ok) {
        res.push_back(user);
      }
    }
    return res;
  }

 private:
  mutable std::mutex mutex_;
  int64_t next_id_{1};
  std::unordered_map<std::string, UserRecord> users_by_login_;
  std::unordered_map<int64_t, UserRecord> users_by_id_;
};

inline UsersStorage& GetUsersStorage() {
  static UsersStorage storage;
  return storage;
}
