#include "search.hpp"

#include <docs/definitions/users.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/server/http/http_status.hpp>
#include <chrono>
#include <mutex>
#include <unordered_map>

namespace jwt_auth::users {

namespace {
using UserId = std::int64_t;

struct TokenBucket {
    double tokens = 2.0;
    std::chrono::steady_clock::time_point last_refill = std::chrono::steady_clock::now();
    static constexpr double max_tokens = 2.0;
    static constexpr double refill_rate = 2.0;
};

std::unordered_map<UserId, TokenBucket> rate_limits;
std::mutex rate_limits_mutex;

bool CheckRateLimit(UserId user_id, int& remaining) {
    std::lock_guard<std::mutex> lock(rate_limits_mutex);
    auto now = std::chrono::steady_clock::now();
    auto& bucket = rate_limits[user_id];
    
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - bucket.last_refill).count();
    bucket.tokens = std::min(bucket.max_tokens, bucket.tokens + elapsed * bucket.refill_rate);
    bucket.last_refill = now;
    
    remaining = static_cast<int>(bucket.tokens);
    if (bucket.tokens >= 1.0) {
        bucket.tokens -= 1.0;
        remaining--;
        return true;
    }
    return false;
}

userver::formats::json::Value MakeError(std::string msg) {
  userver::formats::json::ValueBuilder b;
  b["error"] = std::move(msg);
  return b.ExtractValue();
}
}  // namespace

Search::Search(const userver::components::ComponentConfig& config,
               const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context), storage_(context
                .FindComponent<jwt_auth::repositories::UsersRepositoryComponent>()
                .Get()) {}

userver::formats::json::Value Search::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext& request_context) const {
  UserId user_id;
  try {
    user_id = request_context.GetUserData<UserId>();
  } catch (...) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kUnauthorized);
    return MakeError("user_id not foundtoken");
  }

  int remaining = 0;
  if (!CheckRateLimit(user_id, remaining)) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kTooManyRequests);
    request.GetHttpResponse().SetHeader(std::string_view("X-RateLimit-Limit"), "2");
    request.GetHttpResponse().SetHeader(std::string_view("X-RateLimit-Remaining"), std::to_string(remaining));
    request.GetHttpResponse().SetHeader(std::string_view("X-RateLimit-Reset"), "1");
    return MakeError("Rate limit exceeded");
  }

  const auto login = request.GetArg("login");
  const auto firstMask = request.GetArg("firstNameMask");
  const auto lastMask = request.GetArg("lastNameMask");

  const bool hasLogin = !login.empty();
  const bool hasFirst = !firstMask.empty();
  const bool hasLast = !lastMask.empty();

  if (!hasLogin && !hasFirst && !hasLast) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError("at least one filter required");
  }

  auto results = storage_.Search(
      hasLogin ? std::optional<std::string>(login) : std::nullopt,
      hasFirst ? std::optional<std::string>(firstMask) : std::nullopt,
      hasLast ? std::optional<std::string>(lastMask) : std::nullopt);

  SearchResponseBody resp;
  resp.reserve(results.size());

  for (const auto& u : results) {
    User dto;
    dto.id = u.id;
    dto.login = u.login;
    dto.first_name = u.first_name;
    dto.last_name = u.last_name;

    resp.push_back(std::move(dto));
  }

  return userver::formats::json::ValueBuilder(resp).ExtractValue();
  // userver::formats::json::ValueBuilder builder;
  // builder = userver::formats::json::Type::kArray;

  // for (const auto& u : results) {
  //   userver::formats::json::ValueBuilder item;
  //   item["id"] = u.id;
  //   item["login"] = u.login;
  //   item["first_name"] = u.firstName;
  //   item["last_name"] = u.lastName;

  //   builder.PushBack(item.ExtractValue());
  // }

  // return builder.ExtractValue();
}

}  // namespace jwt_auth::users