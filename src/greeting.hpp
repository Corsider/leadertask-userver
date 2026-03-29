#pragma once

#include <string>
#include <string_view>

#include <docs/definitions/hello.hpp>

namespace jwt_auth::hello {

HelloResponseBody SayHelloTo(HelloRequestBody&& body);

}