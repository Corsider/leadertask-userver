#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
//#include <string_view>
#include <unordered_map>
#include <vector>

#include <docs/definitions/tasks.hpp>

enum class TaskStatus {
  TASK_STATUS_TODO,
  TASK_STATUS_IN_PROGRESS,
  TASK_STATUS_DONE
};

inline std::string TaskStatusToString(TaskStatus st) {
  switch (st) {
    case TaskStatus::TASK_STATUS_TODO:
      return "todo";
    case TaskStatus::TASK_STATUS_IN_PROGRESS:
      return "in_progress";
    case TaskStatus::TASK_STATUS_DONE:
      return "done";
  }
  return "todo";
}

inline TaskStatus TaskStatusFromString(std::string st) {
  if (st == "todo") {
    return TaskStatus::TASK_STATUS_TODO;
  } else if (st == "in_progress") {
    return TaskStatus::TASK_STATUS_IN_PROGRESS;
  } else if (st == "done") {
    return TaskStatus::TASK_STATUS_DONE;
  } else {
    throw std::invalid_argument("invalid task statuss " + st);
  }
}

// TaskStatus ConvertStatus(jwt_auth::tasks::UpdateTaskStatusRequest::Status st)
// {
//   switch (st) {
//     case jwt_auth::tasks::UpdateTaskStatusRequest::Status::kTodo:
//       return TaskStatus::TASK_STATUS_TODO;
//     case jwt_auth::tasks::UpdateTaskStatusRequest::Status::kInProgress:
//       return TaskStatus::TASK_STATUS_IN_PROGRESS;
//     case jwt_auth::tasks::UpdateTaskStatusRequest::Status::kDone:
//       return TaskStatus::TASK_STATUS_DONE;
//   }
//   throw std::runtime_error("unknown status");
// }

struct TaskRecord {
  std::int64_t id{};
  std::int64_t goalId{};
  std::string title;
  std::string status{TaskStatusToString(TaskStatus::TASK_STATUS_TODO)};
};

class TasksStorage {
 public:
  TaskRecord CreateTask(std::int64_t goal_id, std::string title) {
    std::lock_guard lock(mutex_);

    const std::int64_t id = next_id_++;
    TaskRecord task{.id = id,
                    .goalId = goal_id,
                    .title = std::move(title),
                    .status = TaskStatusToString(TaskStatus::TASK_STATUS_TODO)};

    tasks_by_id_.emplace(id, task);
    tasks_by_goal_[goal_id].push_back(task);
    return task;
  }

  std::optional<TaskRecord> GetById(std::int64_t id) const {
    std::lock_guard lock(mutex_);
    const auto it = tasks_by_id_.find(id);
    if (it == tasks_by_id_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  std::vector<TaskRecord> GetByGoal(std::int64_t goal_id) const {
    std::lock_guard lock(mutex_);
    const auto it = tasks_by_goal_.find(goal_id);
    if (it == tasks_by_goal_.end()) return {};
    return it->second;
  }

  /////////////////////////////////////////////////////////////////////////////////////
  std::optional<TaskRecord> UpdateTaskStatus(std::int64_t goal_id,
                                             std::int64_t task_id,
                                             TaskStatus new_st) {
    std::lock_guard lock(mutex_);

    auto it = tasks_by_id_.find(task_id);
    if (it == tasks_by_id_.end()) {
      return std::nullopt;
    }

    if (it->second.goalId != goal_id) {
      return std::nullopt;
    }

    it->second.status = TaskStatusToString(new_st);
    const TaskRecord updated = it->second;

    auto goal_it = tasks_by_goal_.find(goal_id);
    if (goal_it != tasks_by_goal_.end()) {
      for (auto& task : goal_it->second) {
        if (task.id == task_id) {
          task.status = TaskStatusToString(new_st);
          break;
        }
      }
    }

    /////////// t
    return updated;
  }

 private:
  mutable std::mutex mutex_;
  std::int64_t next_id_{1};

  std::unordered_map<std::int64_t, TaskRecord> tasks_by_id_;
  std::unordered_map<std::int64_t, std::vector<TaskRecord>> tasks_by_goal_;
};

inline TasksStorage& GetTasksStorage() {
  static TasksStorage storage;
  return storage;
}