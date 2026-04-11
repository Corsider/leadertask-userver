#include "search.hpp"

#include <docs/definitions/users.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/server/http/http_status.hpp>

namespace jwt_auth::users {

namespace {
userver::formats::json::Value MakeError(std::string msg) {
  userver::formats::json::ValueBuilder b;
  b["error"] = std::move(msg);
  return b.ExtractValue();
}
}  // namespace

Search::Search(const userver::components::ComponentConfig& config,
               const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context), storage_(context
                .FindComponent<jwt_auth::repositories::UsersRepositoryComponent>()
                .Get()) {}

userver::formats::json::Value Search::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto login = request.GetArg("login");
  const auto firstMask = request.GetArg("firstNameMask");
  const auto lastMask = request.GetArg("lastNameMask");

  const bool hasLogin = !login.empty();
  const bool hasFirst = !firstMask.empty();
  const bool hasLast = !lastMask.empty();

  if (!hasLogin && !hasFirst && !hasLast) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError("at least one filter required");
  }

  auto results = storage_.Search(
      hasLogin ? std::optional<std::string>(login) : std::nullopt,
      hasFirst ? std::optional<std::string>(firstMask) : std::nullopt,
      hasLast ? std::optional<std::string>(lastMask) : std::nullopt);

  SearchResponseBody resp;
  resp.reserve(results.size());

  for (const auto& u : results) {
    User dto;
    dto.id = u.id;
    dto.login = u.login;
    dto.first_name = u.first_name;
    dto.last_name = u.last_name;

    resp.push_back(std::move(dto));
  }

  return userver::formats::json::ValueBuilder(resp).ExtractValue();
  // userver::formats::json::ValueBuilder builder;
  // builder = userver::formats::json::Type::kArray;

  // for (const auto& u : results) {
  //   userver::formats::json::ValueBuilder item;
  //   item["id"] = u.id;
  //   item["login"] = u.login;
  //   item["first_name"] = u.firstName;
  //   item["last_name"] = u.lastName;

  //   builder.PushBack(item.ExtractValue());
  // }

  // return builder.ExtractValue();
}

}  // namespace jwt_auth::users