#include "goals_repository.hpp"

#include <userver/storages/postgres/io/chrono.hpp>

namespace jwt_auth::repositories {

namespace pg = userver::storages::postgres;

GoalsRepository::GoalsRepository(pg::ClusterPtr pg_cluster)
    : pg_cluster_(std::move(pg_cluster)) {}

jwt_auth::goals::CreateGoalResponseBody GoalsRepository::CreateGoal(
    std::int64_t user_id, const std::string& title,
    const std::optional<std::string>& description) {
  auto result = pg_cluster_->Execute(pg::ClusterHostType::kMaster,
                                     R"(
        INSERT INTO goals (user_id, title, description)
        VALUES ($1, $2, $3)
        RETURNING id, user_id AS "userId", title, description
      )",
                                     user_id, title, description);

  return result.AsSingleRow<jwt_auth::goals::CreateGoalResponseBody>(
      pg::kRowTag);
}

std::optional<jwt_auth::goals::Goal> GoalsRepository::GetById(std::int64_t id) {
  auto result = pg_cluster_->Execute(pg::ClusterHostType::kSlave,
                                     R"(
        SELECT id, title, description, user_id AS "userId"
        FROM goals WHERE id = $1
      )",
                                     id);

  ////////// TODOD
  if (result.IsEmpty()) {
    return std::nullopt;
  }
  return result.AsSingleRow<jwt_auth::goals::Goal>(pg::kRowTag);
}

std::vector<jwt_auth::goals::Goal> GoalsRepository::GetByUser(
    std::int64_t user_id) {
  auto result = pg_cluster_->Execute(pg::ClusterHostType::kSlave,
                                     R"(
        SELECT id, title, description, user_id AS "userId"
        FROM goals WHERE user_id = $1 ORDER BY created_at DESC
      )",
                                     user_id);

  return result.AsContainer<std::vector<jwt_auth::goals::Goal>>(pg::kRowTag);
}

}  // namespace jwt_auth::repositories