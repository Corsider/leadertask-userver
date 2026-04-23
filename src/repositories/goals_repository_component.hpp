#pragma once

#include <userver/components/component.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/pool.hpp>

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
                  .FindComponent<userver::components::Mongo>("mongo")
                  .GetPool()
                  ->GetCollection("goals")) {}

  GoalsRepository& Get() { return repo_; }

 private:
  GoalsRepository repo_;
};

}  // namespace jwt_auth::repositories