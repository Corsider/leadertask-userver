#pragma once

#include <userver/components/component.hpp>
#include <userver/storages/postgres/component.hpp>

#include <repositories/goals_repository.hpp>

namespace jwt_auth::repositories {

class GoalsRepositoryComponent final
    : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "goals-repository";

  GoalsRepositoryComponent(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context)
      : ComponentBase(config, context),
        repo_(context
                  .FindComponent<userver::components::Postgres>("postgres")
                  .GetCluster()) {}

  GoalsRepository& Get() { return repo_; }

 private:
  GoalsRepository repo_;
};

}  // namespace jwt_auth::repositories