#include "create.hpp"

#include <docs/definitions/goals.hpp>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <chrono>
#include <mutex>
#include <unordered_map>

namespace jwt_auth::goals {

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
    
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now-bucket.last_refill).count();
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

Create::Create(const userver::components::ComponentConfig& config,
               const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context
                .FindComponent<jwt_auth::repositories::GoalsRepositoryComponent>()
                .Get()),
      usersStorage_(context
                .FindComponent<jwt_auth::repositories::UsersRepositoryComponent>()
                .Get()) {}

userver::formats::json::Value Create::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_body,
    userver::server::request::RequestContext& request_context) const {
  const auto body = request_body.As<CreateGoalRequestBody>();

  if (body.title.empty()) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return MakeError("title is required");
  }

  // TODO handle
  UserId user_id;
  try {
    user_id = request_context.GetUserData<UserId>();
  } catch (const std::exception&) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kUnauthorized);
    return MakeError("user_id not found in token");
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

  // if (!usersStorage_.UserExists(user_id)) {
  //   request.GetHttpResponse().SetStatus(
  //       userver::server::http::HttpStatus::kBadRequest);
  //   return MakeError("user doesnt not exist");
  // }

  const auto goal =
      storage_.CreateGoal(user_id, body.title, body.description.value_or(""));

  request.GetHttpResponse().SetStatus(
      userver::server::http::HttpStatus::kCreated);

  CreateGoalResponseBody resp;
  resp.id = goal.id;
  resp.title = goal.title;
  resp.description = goal.description;
  resp.userId = goal.userId;
  return userver::formats::json::ValueBuilder{resp}.ExtractValue();
}

}  // namespace jwt_auth::goals