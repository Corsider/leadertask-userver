#pragma once

#include <optional>
#include <userver/storages/postgres/cluster.hpp>
#include <vector>

#include <docs/definitions/tasks.hpp>

namespace jwt_auth::repositories {

class TasksRepository {
 public:
  explicit TasksRepository(userver::storages::postgres::ClusterPtr pg_cluster);

  jwt_auth::tasks::CreateTaskResponseBody CreateTask(std::int64_t goal_id,
                                                     const std::string& title);

  std::optional<jwt_auth::tasks::Task> GetById(std::int64_t id);

  std::vector<jwt_auth::tasks::Task> GetByGoal(std::int64_t goal_id);

  std::optional<jwt_auth::tasks::Task> UpdateStatus(
      std::int64_t task_id, std::int64_t goal_id,
      const std::string& new_status);

  // inline TaskStatus TaskStatusFromString(std::string st) {
  //   if (st == "todo") {
  //     return TaskStatus::TASK_STATUS_TODO;
  //   } else if (st == "in_progress") {
  //     return TaskStatus::TASK_STATUS_IN_PROGRESS;
  //   } else if (st == "done") {
  //     return TaskStatus::TASK_STATUS_DONE;
  //   } else {
  //     throw std::invalid_argument("invalid task statuss " + st);
  //   }
  // }
 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace jwt_auth::repositories