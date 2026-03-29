#include <views/hello/handler.hpp>

#include <docs/definitions/hello.hpp>

#include <greeting.hpp>

namespace jwt_auth::hello {

std::string Handler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  static constexpr std::string_view kArgName = "name";

  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  auto request_json = userver::formats::json::FromString(request.RequestBody());

  auto request_dom = request_json.As<HelloRequestBody>();
  if (request_dom.name == HelloRequestBody::kFieldDefaultname &&
      request.HasArg(kArgName)) {
    request_dom.name = request.GetArg(kArgName);
  }
  // r
  auto response_dom = SayHelloTo(std::move(request_dom));

  auto response_json =
      userver::formats::json::ValueBuilder{response_dom}.ExtractValue();
  return userver::formats::json::ToString(response_json);
}

}  // namespace jwt_auth::hello