#pragma once

#include <optional>
#include <userver/storages/postgres/cluster.hpp>
#include <vector>

#include <docs/definitions/goals.hpp>

namespace jwt_auth::repositories {

class GoalsRepository {
 public:
  explicit GoalsRepository(userver::storages::postgres::ClusterPtr pg_cluster);

  jwt_auth::goals::CreateGoalResponseBody CreateGoal(
      std::int64_t user_id, const std::string& title,
      const std::optional<std::string>& description);

  std::optional<jwt_auth::goals::Goal> GetById(std::int64_t id);

  std::vector<jwt_auth::goals::Goal> GetByUser(std::int64_t user_id);

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace jwt_auth::repositories