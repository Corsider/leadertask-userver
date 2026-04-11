#pragma once

#include <userver/server/handlers/http_handler_json_base.hpp>

#include <repositories/goals_repository.hpp>
#include <repositories/goals_repository_component.hpp>
#include <repositories/users_repository.hpp>
#include <repositories/users_repository_component.hpp>

namespace jwt_auth::goals {

class Create final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-goals-create";

  Create(const userver::components::ComponentConfig& config,
         const userver::components::ComponentContext& context);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_body,
      userver::server::request::RequestContext& request_context) const override;

 private:
  jwt_auth::repositories::GoalsRepository& storage_;
  jwt_auth::repositories::UsersRepository& usersStorage_;
};

}  // namespace jwt_auth::goals