#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct GoalRecord {
  std::int64_t id{};
  std::string title;
  std::string description;
  std::int64_t userId{};
};

class GoalsStorage {
 public:
  GoalRecord CreateGoal(std::string title, std::string description,
                        std::int64_t user_id) {
    std::lock_guard lock(mutex_);

    const std::int64_t id = next_id_++;
    GoalRecord goal{
        .id = id,
        .title = std::move(title),
        .description = std::move(description),
        .userId = user_id,
    };
    goals_by_id_.emplace(id, goal);
    return goal;
  }

  std::optional<GoalRecord> GetById(std::int64_t id) const {
    std::lock_guard lock(mutex_);
    //
    const auto it = goals_by_id_.find(id);
    if (it == goals_by_id_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  std::vector<GoalRecord> GetGoalsByUser(std::int64_t user_id) const {
    std::lock_guard lock(mutex_);
    std::vector<GoalRecord> result;

    for (const auto& [id, goal] : goals_by_id_) {
      if (goal.userId == user_id) {
        result.push_back(goal);
      }
    }
    return result;
  }

 private:
  mutable std::mutex mutex_;
  std::int64_t next_id_{1};
  std::unordered_map<std::int64_t, GoalRecord> goals_by_id_;
};

inline GoalsStorage& GetGoalsStorage() {
  static GoalsStorage storage;
  return storage;
}