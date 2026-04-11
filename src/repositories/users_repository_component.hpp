#pragma once

#include <userver/components/component.hpp>
#include <userver/storages/postgres/component.hpp>

#include <repositories/users_repository.hpp>

namespace jwt_auth::repositories {

class UsersRepositoryComponent final
    : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "users-repository";

  UsersRepositoryComponent(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context)
      : ComponentBase(config, context),
        repo_(context
                  .FindComponent<userver::components::Postgres>("postgres")
                  .GetCluster()) {}

  UsersRepository& Get() { return repo_; }

 private:
  UsersRepository repo_;
};

}  // namespace jwt_auth::repositories