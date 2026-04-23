#pragma once

#include <optional>
#include <userver/storages/mongo/collection.hpp>
#include <userver/storages/mongo/pool.hpp>
#include <vector>

#include <docs/definitions/goals.hpp>

namespace jwt_auth::repositories {

class GoalsRepository {
 public:
  explicit GoalsRepository(userver::storages::mongo::Collection collection);

  jwt_auth::goals::CreateGoalResponseBody CreateGoal(
      std::int64_t user_id, const std::string& title,
      const std::optional<std::string>& description);

  std::optional<jwt_auth::goals::Goal> GetById(const std::string& id);

  std::vector<jwt_auth::goals::Goal> GetByUser(std::int64_t user_id);

 private:
  userver::storages::mongo::Collection collection_;
};

}  // namespace jwt_auth::repositories