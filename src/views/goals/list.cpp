#include "list.hpp"

#include <docs/definitions/goals.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
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

List::List(const userver::components::ComponentConfig& config,
           const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context), storage_(context
                .FindComponent<jwt_auth::repositories::GoalsRepositoryComponent>()
                .Get()) {}

userver::formats::json::Value List::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& /*request_body*/,
    userver::server::request::RequestContext& request_context) const {
  UserId user_id;
  try {
    user_id = request_context.GetUserData<UserId>();
  } catch (const std::exception&) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kUnauthorized);
    return MakeError("user_id not found in token");
  }

  const auto goals = storage_.GetByUser(user_id);

  GetGoalsResponseBody resp;
  resp.reserve(goals.size());

  for (const auto& goal : goals) {
    Goal dto;
    dto.id = goal.id;
    dto.title = goal.title;
    dto.description = goal.description;
    dto.userId = goal.userId;
    resp.push_back(std::move(dto));
  }
  return userver::formats::json::ValueBuilder(resp).ExtractValue();
}

}  // namespace jwt_auth::goals