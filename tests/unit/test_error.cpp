#include "doctest.hpp"

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

TEST_CASE("error: all code names")
{
    CHECK(binhound::errorCodeName(binhound::Error::Code::FileTooLarge) == "FileTooLarge");
    CHECK(binhound::errorCodeName(binhound::Error::Code::ReadFailed) == "ReadFailed");
    CHECK(binhound::errorCodeName(binhound::Error::Code::Truncated) == "Truncated");
    CHECK(binhound::errorCodeName(binhound::Error::Code::BadEndianness) == "BadEndianness");
    CHECK(binhound::errorCodeName(binhound::Error::Code::UnsupportedClass) == "UnsupportedClass");
}
