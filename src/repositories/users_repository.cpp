#include "users_repository.hpp"

#include <userver/storages/postgres/exceptions.hpp>
#include <userver/storages/postgres/io/chrono.hpp>

namespace jwt_auth::repositories {

namespace pg = userver::storages::postgres;

UsersRepository::UsersRepository(pg::ClusterPtr pg_cluster)
    : pg_cluster_(std::move(pg_cluster)) {}

std::optional<jwt_auth::users::CreateUserResponseBody>
UsersRepository::CreateUser(const std::string& login,
                            const std::string& password,
                            const std::string& first_name,
                            const std::string& last_name) {
  try   {
    auto result = pg_cluster_->Execute(pg::ClusterHostType::kMaster,
                                       R"(
          INSERT INTO users (login, password, first_name, last_name)
          VALUES ($1, $2, $3, $4)
          RETURNING id
        )",
                                       login, password, first_name, last_name);

    jwt_auth::users::CreateUserResponseBody response;
    response.user_id = result.AsSingleRow<std::int64_t>();
    return response;
  } catch (const pg::UniqueViolation&) {
    return std::nullopt;
  }
}

std::optional<UsersRepository::UserWithPassword>
UsersRepository::GetByLoginWithPassword(const std::string& login) {
  auto resultsss = pg_cluster_->Execute(pg::ClusterHostType::kSlave,
                                     R"(
        SELECT id, login, password, first_name, last_name
        FROM users WHERE login = $1
      )",
                                     login);

  if (resultsss.IsEmpty()) return std::nullopt;

  auto row = resultsss.Front();
  return UserWithPassword{
      .id = row["id"].As<std::int64_t>(),
      .login = row["login"].As<std::string>(),
      .password = row["password"].As<std::string>(), /////////////
      .first_name = row["first_name"].As<std::string>(),
      .last_name = row["last_name"].As<std::string>(),
  };
}

std::optional<jwt_auth::users::User> UsersRepository::GetById(
    std::int64_t user_id) {
  auto result = pg_cluster_->Execute(pg::ClusterHostType::kSlave,
                                     R"(
        SELECT id, login, first_name, last_name
        FROM users WHERE id = $1
      )",
                                     user_id);

  if (result.IsEmpty()) return std::nullopt;
  return result.AsSingleRow<jwt_auth::users::User>(pg::kRowTag);
}

bool UsersRepository::UserExists(std::int64_t user_id) {
  auto result = pg_cluster_->Execute(
      pg::ClusterHostType::kSlave,
      "SELECT EXISTS(SELECT 1 FROM users WHERE id = $1)", user_id);
  return result.AsSingleRow<bool>();
}

std::vector<jwt_auth::users::User> UsersRepository::Search(
    const std::optional<std::string>& login,
    const std::optional<std::string>& first_name_mask,
    const std::optional<std::string>& last_name_mask) {
  auto result = pg_cluster_->Execute(pg::ClusterHostType::kSlave,
                                     R"(
        SELECT id, login, first_name, last_name
        FROM users
        WHERE ($1::TEXT IS NULL OR login = $1)
          AND ($2::TEXT IS NULL OR first_name LIKE '%' || $2 || '%')
          AND ($3::TEXT IS NULL OR last_name LIKE '%' || $3 || '%')
        ORDER BY id
      )",
                                     login, first_name_mask, last_name_mask);

  return result.AsContainer<std::vector<jwt_auth::users::User>>(pg::kRowTag);
}

// std::vector<jwt_auth::users::User> UsersRepository::Search(
//     const std::optional<std::string>& login,
//     const std::optional<std::string>& first_name_mask,
//     const std::optional<std::string>& last_name_mask) {
//   auto result = pg_cluster_->Execute(pg::ClusterHostType::kSlave,
//                                      R"(
//         SELECT id, login, first_name, last_name
//         FROM users
//         WHERE ($1::TEXT IS NULL OR login = $1)
//           AND ($2::TEXT IS NULL OR first_name LIKE '%' || $2 || '%')
//           AND ($3::TEXT IS NULL OR last_name LIKE '%' || $3 || '%')
//         ORDER BY id
//       )",
//                                      login, first_name_mask, last_name_mask);

//   return result.AsContainer<std::vector<jwt_auth::users::User>>(pg::kRowTag);
// }

}  // namespace jwt_auth::repositories