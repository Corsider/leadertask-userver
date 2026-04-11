#pragma once

#include <optional>
#include <string>
#include <userver/storages/postgres/cluster.hpp>
#include <vector>

#include <docs/definitions/users.hpp>

namespace jwt_auth::repositories {

class UsersRepository {
 public:
  explicit UsersRepository(userver::storages::postgres::ClusterPtr pg_cluster);

  std::optional<jwt_auth::users::CreateUserResponseBody> CreateUser(
      const std::string& login, const std::string& password,
      const std::string& first_name, const std::string& last_name);

  struct UserWithPassword {
    std::int64_t id;
    std::string login;
    std::string password;
    std::string first_name;
    std::string last_name;
  };
  std::optional<UserWithPassword> GetByLoginWithPassword(
      const std::string& login);

  std::optional<jwt_auth::users::User> GetById(std::int64_t user_id);

  bool UserExists(std::int64_t user_id);

  std::vector<jwt_auth::users::User> Search(
      const std::optional<std::string>& login,
      const std::optional<std::string>& first_name_mask,
      const std::optional<std::string>& last_name_mask);

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace jwt_auth::repositories