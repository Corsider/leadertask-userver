#pragma once

#include <userver/server/handlers/http_handler_json_base.hpp>

#include <goals_storage.hpp>
#include <users_storage.hpp>

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
  GoalsStorage& storage_;
  UsersStorage& usersStorage_;
};

}  // namespace jwt_auth::goals