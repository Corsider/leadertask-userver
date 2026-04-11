#include "tasks_repository.hpp"

#include <userver/storages/postgres/io/chrono.hpp>

namespace jwt_auth::repositories {

namespace pg = userver::storages::postgres;

TasksRepository::TasksRepository(pg::ClusterPtr pg_cluster)
    : pg_cluster_(std::move(pg_cluster)) {}

jwt_auth::tasks::CreateTaskResponseBody TasksRepository::CreateTask(
    std::int64_t goal_id, const std::string& title) {
  auto res = pg_cluster_->Execute(pg::ClusterHostType::kMaster,
                                  R"(
        INSERT INTO tasks (goal_id, title, status)
        VALUES ($1, $2, 'todo')
        RETURNING id, goal_id AS "goalId", title, status
      )",
                                  goal_id, title);

  //    return res.AsSingleRow<jwt_auth::tasks::CreateTaskResponseBody>(
  // pg::kRowTag);
  return res.AsSingleRow<jwt_auth::tasks::CreateTaskResponseBody>(pg::kRowTag);
}

std::optional<jwt_auth::tasks::Task> TasksRepository::GetById(std::int64_t id) {
  auto result = pg_cluster_->Execute(pg::ClusterHostType::kSlave,
                                     R"(
        SELECT id, goal_id AS "goalId", title, status
        FROM tasks WHERE id = $1
      )",
                                     id);

  if (result.IsEmpty()) return std::nullopt;
  return result.AsSingleRow<jwt_auth::tasks::Task>(pg::kRowTag);
}

std::vector<jwt_auth::tasks::Task> TasksRepository::GetByGoal(
    std::int64_t goal_id) {
  auto result = pg_cluster_->Execute(pg::ClusterHostType::kSlave,
                                     R"(
        SELECT id, goal_id AS "goalId", title, status
        FROM tasks WHERE goal_id = $1 ORDER BY created_at
      )",
                                     goal_id);

  return result.AsContainer<std::vector<jwt_auth::tasks::Task>>(pg::kRowTag);
}

std::optional<jwt_auth::tasks::Task> TasksRepository::UpdateStatus(
    std::int64_t task_id, std::int64_t goal_id, const std::string& new_status) {
  if (new_status != "todo" && new_status != "in_progress" &&
      new_status != "done") {
    return std::nullopt;
  }

  auto ress = pg_cluster_->Execute(pg::ClusterHostType::kMaster,
                                     R"(
        UPDATE tasks SET status = $3
        WHERE id = $1 AND goal_id = $2
        returning id, goal_id AS "goalId", title, status
      )",
                                     task_id, goal_id, new_status);

  if (ress.IsEmpty()) return std::nullopt;
  return ress.AsSingleRow<jwt_auth::tasks::Task>(pg::kRowTag);
}

}  // namespace jwt_auth::repositories