#include "update.hpp"

#include <docs/definitions/goals.hpp>
#include <docs/definitions/tasks.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <chrono>
#include <mutex>
#include <unordered_map>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace jwt_auth::tasks {

namespace {
using UserId = std::int64_t;

struct TokenBucket {
    double tokens = 2.0;
    std::chrono::steady_clock::time_point last_refill = std::chrono::steady_clock::now();
    static constexpr double max_tokens = 2.0;
    static constexpr double refill_rate = 2.0;
};

std::unordered_map<UserId, TokenBucket> rate_limits;
std::mutex rate_limits_mutex;

bool CheckRateLimit(UserId user_id, int& remaining) {
    std::lock_guard<std::mutex> lock(rate_limits_mutex);
    auto now = std::chrono::steady_clock::now();
    auto& bucket = rate_limits[user_id];
    
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - bucket.last_refill).count();
    bucket.tokens = std::min(bucket.max_tokens, bucket.tokens + elapsed * bucket.refill_rate);
    bucket.last_refill = now;
    
    remaining = static_cast<int>(bucket.tokens);
    if (bucket.tokens >= 1.0) {
        bucket.tokens -= 1.0;
        remaining--;
        return true;
    }
    return false;
}

userver::formats::json::Value MakeError(std::string msg) {
  userver::formats::json::ValueBuilder b;
  b["error"] = std::move(msg);
  return b.ExtractValue();
}

bool ParseInt(std::string_view value, std::int64_t& out) {
  try {
    std::size_t pos = 0;
    const auto parsed = std::stoll(std::string(value), &pos);
    if (pos != value.size()) {
      return false;
    }
    out = static_cast<std::int64_t>(parsed);
    return true;
    // Todod
  } catch (...) {
    return false;
  }
}

// userver::formats::json::Value TaskToResop(const TaskRecord& task) {
//   userver::formats::json::ValueBuilder b;
//   b["id"] = task.id;
//   b["goalId"] = task.goalId;
//   b["title"] = task.title;
//   b["status"] = std::string(TaskStatusToString(task.status));
//   return b.ExtractValue();
// }

}  // namespace

UpdateTaskStatus::UpdateTaskStatus(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      goals_storage_(context
                .FindComponent<jwt_auth::repositories::GoalsRepositoryComponent>()
                .Get()),
      tasks_storage_(context
                .FindComponent<jwt_auth::repositories::TasksRepositoryComponent>()
                .Get()) {}

userver::formats::json::Value UpdateTaskStatus::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_body,
    userver::server::request::RequestContext& request_context) const {
  UserId user_id;
  try {
    user_id = request_context.GetUserData<UserId>();
  } catch (...) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kUnauthorized);
    return MakeError("user_id not found  token");
  }


  ////////////////////
  int remaining = 0;
  if (!CheckRateLimit(user_id, remaining)) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kTooManyRequests);
    request.GetHttpResponse().SetHeader(std::string_view("X-RateLimit-Limit"), "2");
    request.GetHttpResponse().SetHeader(std::string_view("X-RateLimit-Remaining"), std::to_string(remaining));
    request.GetHttpResponse().SetHeader(std::string_view("X-RateLimit-Reset"), "1");
    return MakeError("Rate limit exceeded");
  }

  std::int64_t goal_id{};
  std::int64_t task_id{};

  if (!ParseInt(request.GetPathArg("goalId"), goal_id) ||
      !ParseInt(request.GetPathArg("taskId"), task_id)) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError("invalid path parameters");
  }

  const auto goal_id_str = request.GetPathArg("goalId");
  if (!goals_storage_.GetById(goal_id_str)) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kNotFound);
    return MakeError("goal not found");
  }

  UpdateTaskStatusRequest body;
  try {
    body = request_body.As<UpdateTaskStatusRequest>();
  } catch (const std::exception&) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError("invalid request body");
  }

  const auto new_status = body.status;
  // TODO

  const auto updated_task = tasks_storage_.UpdateStatus(
      goal_id, task_id, new_status);
  if (!updated_task) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kNotFound);
    return MakeError("task not found");
  }

  request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);

  CreateTaskResponseBody resp;
  resp.id = updated_task->id;
  resp.goalId = updated_task->goalId;
  resp.title = updated_task->title;
  resp.status = updated_task->status;
  return userver::formats::json::ValueBuilder{resp}.ExtractValue();
}

}  // namespace jwt_auth::tasks