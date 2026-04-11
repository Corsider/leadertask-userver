#pragma once

#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/yaml_config/schema.hpp>

#include <repositories/users_repository_component.hpp>
#include <repositories/users_repository.hpp>

namespace jwt_auth::users {

class Login final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-login";

  Login(const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext& context) const override;

  static userver::yaml_config::Schema GetStaticConfigSchema();

 private:
  jwt_auth::repositories::UsersRepository& storage_;
  std::string secret_;
};

}  // namespace jwt_auth::users