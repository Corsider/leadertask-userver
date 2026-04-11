#include "list.hpp"

#include <docs/definitions/tasks.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
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

ListTasks::ListTasks(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      tasks_storage_(context
                .FindComponent<jwt_auth::repositories::TasksRepositoryComponent>()
                .Get()),
      goals_storage_(context
                .FindComponent<jwt_auth::repositories::GoalsRepositoryComponent>()
                .Get()) {}

userver::formats::json::Value ListTasks::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext& request_context) const {
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

  try {
    goal_id = std::stoll(goal_id_str);
  } catch (...) {
    request.GetHttpResponse().SetStatus( userver::server::http::HttpStatus::kBadRequest);
    return MakeError("invalid goalId");
  }


// TODO
  const auto goal = goals_storage_.GetById(goal_id);
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

  const auto tasks = tasks_storage_.GetByGoal(goal_id);



  // ListTasksResponseBody res;
  ListTasksResponseBody resp;
  resp.reserve(tasks.size());

  for (const auto& task : tasks) {
    Task dto;
    dto.id = task.id;
    dto.goalId = task.goalId;
    dto.title = task.title;
    dto.status = task.status;
    resp.push_back(std::move(dto));
  }
  return userver::formats::json::ValueBuilder(resp).ExtractValue();
}
}  // namespace jwt_auth::tasks