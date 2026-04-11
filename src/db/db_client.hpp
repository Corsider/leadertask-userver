#pragma once

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

namespace jwt_auth::db {

class DbClient final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "db-client";

  DbClient(const userver::components::ComponentConfig& config,
           const userver::components::ComponentContext& context)
      : ComponentBase(config, context),
        pg_cluster_(
            context.FindComponent<userver::components::Postgres>("postgres")
                .GetCluster()) {}

  userver::storages::postgres::ClusterPtr GetCluster() const {
    return pg_cluster_;
  }

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace jwt_auth::db