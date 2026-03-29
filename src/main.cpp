#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>

#include <userver/utils/daemon_run.hpp>

#include <auth/jwt_auth_factory.hpp>
#include <goals_storage.hpp>
#include <tasks_storage.hpp>
#include <users_storage.hpp>
#include <views/goals/create.hpp>
#include <views/goals/list.hpp>
#include <views/hello/handler.hpp>
#include <views/tasks/create.hpp>
#include <views/tasks/list.hpp>
#include <views/tasks/update.hpp>
#include <views/users/login.hpp>
#include <views/users/search.hpp>
#include <views/users/signup.hpp>

int main(int argc, char* argv[]) {
  userver::server::handlers::auth::RegisterAuthCheckerFactory<
      auth::jwt::JwtAuthCheckerFactory>();
  auto component_list =
      userver::components::MinimalServerComponentList()
          .Append<userver::server::handlers::Ping>()
          .Append<userver::components::TestsuiteSupport>()
          .AppendComponentList(userver::clients::http::ComponentList())
          .Append<userver::clients::dns::Component>()
          .Append<userver::server::handlers::TestsControl>()
          .Append<userver::congestion_control::Component>()
          .Append<jwt_auth::hello::Handler>()
          .Append<jwt_auth::users::Login>()
          .Append<jwt_auth::users::Signup>()
          .Append<jwt_auth::users::Search>()
          .Append<jwt_auth::goals::Create>()
          .Append<jwt_auth::goals::List>()
          .Append<jwt_auth::tasks::Create>()
          .Append<jwt_auth::tasks::UpdateTaskStatus>()
          .Append<jwt_auth::tasks::ListTasks>()
          .Append<auth::jwt::JwtAuthComponent>();

  return userver::utils::DaemonMain(argc, argv, component_list);
}