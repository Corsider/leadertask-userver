#pragma once

#include <repositories/goals_repository.hpp>
#include <repositories/goals_repository_component.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>

namespace jwt_auth::goals {

class List final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-goals-list";

  /////////////////
  List(const userver::components::ComponentConfig& config,
       const userver::components::ComponentContext& context);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_body,
      userver::server::request::RequestContext& request_context) const override;

 private:
  jwt_auth::repositories::GoalsRepository& storage_;
};

}  // namespace jwt_auth::goals