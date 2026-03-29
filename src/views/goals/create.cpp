#include "create.hpp"

#include <docs/definitions/goals.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

namespace jwt_auth::goals {

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
      storage_(GetGoalsStorage()),
      usersStorage_(GetUsersStorage()) {}

userver::formats::json::Value Create::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_body,
    userver::server::request::RequestContext& request_context) const {
  const auto body = request_body.As<CreateGoalRequestBody>();

  if (body.title.empty()) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError("title is required");
  }

  // TODO handle
  UserId user_id;
  try {
    user_id = request_context.GetUserData<UserId>();
  } catch (const std::exception&) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kUnauthorized);
    return MakeError("user_id not found in token");
  }

  if (!GetUsersStorage().UserExists(user_id)) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError("user doesnt not exist");
  }

  const auto goal =
      storage_.CreateGoal(body.title, body.description.value_or(""), user_id);

  request.GetHttpResponse().SetStatus(
      userver::server::http::HttpStatus::kCreated);

  CreateGoalResponseBody resp;
  resp.id = goal.id;
  resp.title = goal.title;
  resp.description = goal.description;
  resp.userId = goal.userId;
  return userver::formats::json::ValueBuilder{resp}.ExtractValue();
}

}  // namespace jwt_auth::goals