#include "signup.hpp"

#include <docs/definitions/users.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

namespace jwt_auth::users {

namespace {
userver::formats::json::Value MakeError(std::string message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = std::move(message);
  return builder.ExtractValue();
}
}  // namespace

Signup::Signup(const userver::components::ComponentConfig& config,
               const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context), storage_(context
                .FindComponent<jwt_auth::repositories::UsersRepositoryComponent>()
                .Get()) {}

userver::formats::json::Value Signup::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  try {
    // const auto body = request_json;
    // const auto body = request_json.As<CreateUserRequestBody>();

    const auto body = request_json.As<CreateUserRequestBody>();
    if (body.login.empty() || body.password.empty() || body.firstName.empty() ||
        body.lastName.empty()) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kBadRequest);
      return MakeError(
          "login, password, first_name, last_name  required");
    }

    const auto user_id = storage_.CreateUser(body.login, body.password,
                                             body.firstName, body.lastName);

    if (!user_id) {
      request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kConflict);
      return MakeError("user already exists");
    }

    CreateUserResponseBody response;
    response.user_id = user_id->user_id; //////////////// todo fix
    return userver::formats::json::ValueBuilder{response}.ExtractValue();

  } catch (const std::exception& exc) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError(exc.what());
  }
}

}  // namespace jwt_auth::users