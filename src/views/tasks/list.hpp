#pragma once

#include <userver/server/handlers/http_handler_json_base.hpp>

#include <goals_storage.hpp>
#include <tasks_storage.hpp>

namespace jwt_auth::tasks {

class ListTasks final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-goals-list-tasks";

  ListTasks(const userver::components::ComponentConfig& config,
            const userver::components::ComponentContext& context);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_body,
      userver::server::request::RequestContext& request_context) const override;

 private:
  TasksStorage& tasks_storage_;
  GoalsStorage& goals_storage_;
};
}  // namespace jwt_auth::tasks