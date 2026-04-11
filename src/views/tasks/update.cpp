#include "update.hpp"

#include <docs/definitions/goals.hpp>
#include <docs/definitions/tasks.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace jwt_auth::tasks {

namespace {

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
  (void)request_context;

  std::int64_t goal_id{};
  std::int64_t task_id{};

  if (!ParseInt(request.GetPathArg("goalId"), goal_id) ||
      !ParseInt(request.GetPathArg("taskId"), task_id)) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError("invalid path parameters");
  }

  if (!goals_storage_.GetById(goal_id)) {
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