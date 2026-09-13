#include <doctest/doctest.h>

#include "util/error.hpp"

using binhound::Error;
using binhound::errorCodeName;
using binhound::Result;

TEST_CASE("error: code names")
{
    CHECK(errorCodeName(Error::Code::FileNotFound) == "FileNotFound");
    CHECK(errorCodeName(Error::Code::NotElf) == "NotElf");
    CHECK(errorCodeName(Error::Code::DatabaseMissing) == "DatabaseMissing");
}

TEST_CASE("result: carries value or error")
{
    Result<int> ok = 42;
    CHECK(ok.has_value());
    CHECK(ok.value() == 42);

    Result<int> bad = tl::unexpected(Error{Error::Code::ReadFailed, "boom"});
    CHECK_FALSE(bad.has_value());
    CHECK(bad.error().code == Error::Code::ReadFailed);
    CHECK(bad.error().message == "boom");
}
