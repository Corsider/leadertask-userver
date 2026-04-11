#pragma once

#include <userver/components/component.hpp>
#include <userver/storages/postgres/component.hpp>

#include <repositories/tasks_repository.hpp>

namespace jwt_auth::repositories {

class TasksRepositoryComponent final
    : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "tasks-repository";

  TasksRepositoryComponent(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context)
      : ComponentBase(config, context),
        repo_(context
                  .FindComponent<userver::components::Postgres>("postgres")
                  .GetCluster()) {}

  TasksRepository& Get() { return repo_; }

 private:
  TasksRepository repo_;
};

}  // namespace jwt_auth::repositories