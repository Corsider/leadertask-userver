#include "goals_repository.hpp"

#include <userver/formats/bson.hpp>
#include <userver/formats/bson/value_builder.hpp>
#include <userver/storages/mongo/collection.hpp>
#include <userver/storages/mongo/options.hpp>
#include <userver/utils/assert.hpp>

namespace jwt_auth::repositories {

namespace bson = userver::formats::bson;
namespace mongo = userver::storages::mongo;

GoalsRepository::GoalsRepository(mongo::Collection collection)
    : collection_(std::move(collection)) {}

jwt_auth::goals::CreateGoalResponseBody GoalsRepository::CreateGoal(
    std::int64_t user_id, const std::string& title,
    const std::optional<std::string>& description) {
  bson::Oid oid;
  bson::Document doc;
  ///////////////////////////////////////////////
  if(description){
    doc = bson::MakeDoc( "_id", oid, "title", title, "userId", user_id,
                        "description", *description);
  } else {
    doc = bson::MakeDoc( "_id", oid, "title", title, "userId", user_id,
                        "description", nullptr);
  }

  collection_.InsertOne(std::move(doc));

  jwt_auth::goals::CreateGoalResponseBody body;
  body.id = oid.ToString();
  body.title = title;
  body.description = description.value_or("") ;
  body.userId = user_id;
  return body;
}

std::optional<jwt_auth::goals::Goal> GoalsRepository::GetById(
    const std::string& id) {
  auto doc_opt = collection_.FindOne(bson::MakeDoc("_id", bson::Oid(id)));
  if (!doc_opt) {
    return std::nullopt;
  }
  const auto& doc = *doc_opt;

  jwt_auth::goals::Goal goal;
  goal.id = doc["_id"].As<bson::Oid>().ToString();
  goal.title = doc["title"].As<std::string>();
  goal.description =
      (doc.HasMember("description") && !doc["description"].IsNull())
          ? doc["description"].As<std::string>()
          : "";
  goal.userId = doc["userId"].As<std::int64_t>();
  return goal;
}

std::vector<jwt_auth::goals::Goal> GoalsRepository::GetByUser(
    std::int64_t user_id) {
  std::vector<jwt_auth::goals::Goal> result;

  auto cursorrr = collection_.Find(
      bson::MakeDoc("userId", user_id),
      mongo::options::Sort{{"_id", mongo::options::Sort::kDescending}});

  for (const auto& doc : cursorrr) {
    jwt_auth::goals::Goal goal;
    goal.id = doc["_id"].As<bson::Oid>().ToString();
    goal.title = doc["title"].As<std::string>();
    // TODO
    goal.description =
        (doc.HasMember("description") && !doc["description"].IsNull())
            ? doc["description"].As<std::string>()
            : "";
    goal.userId = doc["userId"].As<std::int64_t>();
    result.push_back(std::move(goal));
  }
  return result;
}

}  // namespace jwt_auth::repositories