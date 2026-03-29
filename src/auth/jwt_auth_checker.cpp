#include "jwt_auth_checker.hpp"

#include <jwt-cpp/jwt.h>

#include <userver/http/common_headers.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace auth::jwt {

namespace {
static constexpr std::string_view kSecret = "secret";
static constexpr std::string_view kAlgorithm = "Bearer ";
static constexpr const char* kServiceName = "sample";
using UserId = std::int64_t;

}  // namespace

JwtChecker::JwtChecker(const std::string& secret) : secret_(secret) {}

JwtChecker::AuthCheckResult JwtChecker::CheckAuth(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const std::string_view auth_headr =
      request.GetHeader(userver::http::headers::kAuthorization);

  if (auth_headr.empty()) {
    return AuthCheckResult{
        AuthCheckResult::Status::kTokenNotFound,
        {},
        "Missing 'Authorization' header",
        userver::server::handlers::HandlerErrorCode::kUnauthorized};
  }

  if (!auth_headr.starts_with(kAlgorithm)) {
    return AuthCheckResult{
        AuthCheckResult::Status::kInvalidToken,
        {},
        "Invalid authorization type, expected 'Bearer'",
        userver::server::handlers::HandlerErrorCode::kUnauthorized};
  }

  const std::string_view token = auth_headr.substr(kAlgorithm.length());
  try {
    const std::string token_str{token};

    auto decoded = ::jwt::decode(token_str);

    auto verifier = ::jwt::verify()
                        .allow_algorithm(::jwt::algorithm::hs256{secret_})
                        .with_issuer(kServiceName);

    verifier.verify(decoded);

    /////
    const auto subject = decoded.get_subject();
    if (subject.empty()) {
      return AuthCheckResult{
          AuthCheckResult::Status::kInvalidToken,
          {},
          "Token subjet is empty",
          userver::server::handlers::HandlerErrorCode::kUnauthorized};
    }

    UserId user_id{};
    try {
      user_id = std::stoll(subject);
    } catch (const std::exception&) {
      return AuthCheckResult{
          AuthCheckResult::Status::kInvalidToken,
          {},
          "Token subject is not integer ser_id",
          userver::server::handlers::HandlerErrorCode::kUnauthorized};
    }

    request_context.SetUserData<UserId>(user_id);
    return {};

  } catch (const ::jwt::error::token_verification_exception& exc) {
    return AuthCheckResult{
        AuthCheckResult::Status::kInvalidToken,
        {},
        "Token verif failed: " + std::string{exc.what()}};
  } catch (const std::exception& exc) {
    return AuthCheckResult{
        AuthCheckResult::Status::kForbidden,
        {},
        "Token proces error: " + std::string{exc.what()}};
  }
}

JwtAuthComponent::JwtAuthComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context) {
  authorizer_ = std::make_shared<JwtChecker>(config[kSecret].As<std::string>());
}

JwtCheckerPtr JwtAuthComponent::Get() const { return authorizer_; }

userver::yaml_config::Schema JwtAuthComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<LoggableComponentBase>(R"(
type: object
description: JWT Auth Checker Component
additionalProperties: false
properties:
    secret:
        type: string
        description: secret key for JWT validation
)");
}

}  // namespace auth::jwt