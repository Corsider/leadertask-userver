#pragma once

#include <userver/server/handlers/http_handler_json_base.hpp>

#include <users_storage.hpp>

namespace jwt_auth::users {

class Search final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-users-search";

  Search(const userver::components::ComponentConfig& config,
         const userver::components::ComponentContext& context);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value&,
      userver::server::request::RequestContext&) const override;

 private:
  UsersStorage& storage_;
};

}  // namespace jwt_auth::users