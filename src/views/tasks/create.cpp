#include "create.hpp"

#include <docs/definitions/tasks.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

namespace jwt_auth::tasks {

namespace {
using UserId = std::int64_t;

userver::formats::json::Value MakeError(std::string msg) {
  userver::formats::json::ValueBuilder b;
  b["error"] = std::move(msg);
  return b.ExtractValue();
}

}  // namespace

Create::Create(const userver::components::ComponentConfig& config,
               const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      tasks_(context
                .FindComponent<jwt_auth::repositories::TasksRepositoryComponent>()
                .Get()),
      goals_(context
                .FindComponent<jwt_auth::repositories::GoalsRepositoryComponent>()
                .Get()) {}

userver::formats::json::Value Create::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_body,
    userver::server::request::RequestContext& request_context) const {
  const auto body = request_body.As<CreateTaskRequestBody>();

  if (body.title.empty()) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError("title is required");
  }

  UserId user_id;
  try {
    user_id = request_context.GetUserData<UserId>();
  } catch (...) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kUnauthorized);
    return MakeError("user_id not found in token");
  }

  const auto goal_id_str = request.GetPathArg("goalId");
  std::int64_t goal_id;

  LOG_WARNING() << "goalId " << goal_id_str;

  // TODO
  try {
    goal_id = std::stoll(goal_id_str);
  } catch (...) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError("invalid goalId");
  }

  const auto goal = goals_.GetById(goal_id);
  if (!goal) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kNotFound);
    return MakeError("goal not found in your account");
  }

  if (goal->userId != user_id) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kNotFound);
    return MakeError("goal not found");
  }

  // TODO
  const auto task = tasks_.CreateTask(goal_id, body.title);

  request.GetHttpResponse().SetStatus(
      userver::server::http::HttpStatus::kCreated);

  CreateTaskResponseBody resp;
  resp.id = task.id;
  resp.goalId = task.goalId;
  resp.title = task.title;
  resp.status = task.status;
  return userver::formats::json::ValueBuilder{resp}.ExtractValue();
}

}  // namespace jwt_auth::tasks