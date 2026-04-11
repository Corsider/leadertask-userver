#include "login.hpp"

#include <docs/definitions/users.hpp>

#include <jwt-cpp/jwt.h>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace jwt_auth::users {

namespace {
static constexpr const char* kIssuer = "sample";

userver::formats::json::Value MakeError(std::string message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = std::move(message);
  return builder.ExtractValue();
}
}  // namespace

Login::Login(const userver::components::ComponentConfig& config,
             const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context
                .FindComponent<jwt_auth::repositories::UsersRepositoryComponent>()
                .Get()),
      secret_(config["secret"].As<std::string>()) {}

userver::formats::json::Value Login::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  try {
    const auto body = request_json.As<LoginRequestBody>();

    if (body.login.empty() || body.password.empty()) {
      request.GetHttpResponse().SetStatus( userver::server::http::HttpStatus::kBadRequest );
      return MakeError("login and password are required");
    }

    const auto user = storage_.GetByLoginWithPassword(body.login);
    if (!user || user->password != body.password) {
      request.GetHttpResponse().SetStatus(
          userver::server::http::HttpStatus::kUnauthorized);
      return MakeError("invalid data");
    }
////////////////////// TODO////////////////////////////
    const auto token = ::jwt::create()
                           .set_type("JWT")
                           .set_issuer(kIssuer)
                           .set_subject(std::to_string(user->id))
                           .sign(::jwt::algorithm::hs256{secret_});

    LoginResponseBody resp;
    resp.accessToken = token;
    return userver::formats::json::ValueBuilder{resp}.ExtractValue();

  } catch (const std::exception& exc) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError(exc.what());
  }
}

userver::yaml_config::Schema Login::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<
      userver::server::handlers::HttpHandlerJsonBase>(R"(
type: object
description: Login handler config
additionalProperties: false
properties:
    secret:
        type: string
        description: JWT signing secret
required:
    - secret
)");
}

}  // namespace jwt_auth::users