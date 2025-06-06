#include <vector>
#include <optional>

#include <gtest/gtest.h>

#include "nix/util/error.hh"
#include "nix/util/json-utils.hh"

namespace nix {

/* Test `to_json` and `from_json` with `std::optional` types.
 * We are specifically interested in whether we can _nest_ optionals in STL
 * containers so we that we can leverage existing adl_serializer templates. */

TEST(to_json, optionalInt) {
    std::optional<int> val = std::make_optional(420);
    ASSERT_EQ(nlohmann::json(val), nlohmann::json(420));
    val = std::nullopt;
    ASSERT_EQ(nlohmann::json(val), nlohmann::json(nullptr));
}

TEST(to_json, vectorOfOptionalInts) {
    std::vector<std::optional<int>> vals = {
        std::make_optional(420),
        std::nullopt,
    };
    ASSERT_EQ(nlohmann::json(vals), nlohmann::json::parse("[420,null]"));
}

TEST(to_json, optionalVectorOfInts) {
    std::optional<std::vector<int>> val = std::make_optional(std::vector<int> {
        -420,
        420,
    });
    ASSERT_EQ(nlohmann::json(val), nlohmann::json::parse("[-420,420]"));
    val = std::nullopt;
    ASSERT_EQ(nlohmann::json(val), nlohmann::json(nullptr));
}

TEST(from_json, optionalInt) {
    nlohmann::json json = 420;
    std::optional<int> val = json;
    ASSERT_TRUE(val.has_value());
    ASSERT_EQ(*val, 420);
    json = nullptr;
    json.get_to(val);
    ASSERT_FALSE(val.has_value());
}

TEST(from_json, vectorOfOptionalInts) {
    nlohmann::json json = { 420, nullptr };
    std::vector<std::optional<int>> vals = json;
    ASSERT_EQ(vals.size(), 2u);
    ASSERT_TRUE(vals.at(0).has_value());
    ASSERT_EQ(*vals.at(0), 420);
    ASSERT_FALSE(vals.at(1).has_value());
}

TEST(valueAt, simpleObject) {
    auto simple = R"({ "hello": "world" })"_json;

    ASSERT_EQ(valueAt(getObject(simple), "hello"), "world");

    auto nested = R"({ "hello": { "world": "" } })"_json;

    ASSERT_EQ(valueAt(valueAt(getObject(nested), "hello"), "world"), "");
}

TEST(valueAt, missingKey) {
    auto json = R"({ "hello": { "nested": "world" } })"_json;

    auto & obj = getObject(json);

    ASSERT_THROW(valueAt(obj, "foo"), Error);
}

TEST(getObject, rightAssertions) {
    auto simple = R"({ "object": {} })"_json;

    ASSERT_EQ(getObject(valueAt(getObject(simple), "object")), (nlohmann::json::object_t {}));

    auto nested = R"({ "object": { "object": {} } })"_json;

    auto nestedObject = getObject(valueAt(getObject(nested), "object"));

    ASSERT_EQ(nestedObject, getObject(nlohmann::json::parse(R"({ "object": {} })")));
    ASSERT_EQ(getObject(valueAt(getObject(nestedObject), "object")), (nlohmann::json::object_t {}));
}

TEST(getObject, wrongAssertions) {
    auto json = R"({ "object": {}, "array": [], "string": "", "int": 0, "boolean": false })"_json;

    auto & obj = getObject(json);

    ASSERT_THROW(getObject(valueAt(obj, "array")), Error);
    ASSERT_THROW(getObject(valueAt(obj, "string")), Error);
    ASSERT_THROW(getObject(valueAt(obj, "int")), Error);
    ASSERT_THROW(getObject(valueAt(obj, "boolean")), Error);
}

TEST(getArray, rightAssertions) {
    auto simple = R"({ "array": [] })"_json;

    ASSERT_EQ(getArray(valueAt(getObject(simple), "array")), (nlohmann::json::array_t {}));
}

TEST(getArray, wrongAssertions) {
    auto json = R"({ "object": {}, "array": [], "string": "", "int": 0, "boolean": false })"_json;

    ASSERT_THROW(getArray(valueAt(json, "object")), Error);
    ASSERT_THROW(getArray(valueAt(json, "string")), Error);
    ASSERT_THROW(getArray(valueAt(json, "int")), Error);
    ASSERT_THROW(getArray(valueAt(json, "boolean")), Error);
}

TEST(getString, rightAssertions) {
    auto simple = R"({ "string": "" })"_json;

    ASSERT_EQ(getString(valueAt(getObject(simple), "string")), "");
}

TEST(getString, wrongAssertions) {
    auto json = R"({ "object": {}, "array": [], "string": "", "int": 0, "boolean": false })"_json;

    ASSERT_THROW(getString(valueAt(json, "object")), Error);
    ASSERT_THROW(getString(valueAt(json, "array")), Error);
    ASSERT_THROW(getString(valueAt(json, "int")), Error);
    ASSERT_THROW(getString(valueAt(json, "boolean")), Error);
}

TEST(getIntegralNumber, rightAssertions) {
    auto simple = R"({ "int": 0, "signed": -1 })"_json;

    ASSERT_EQ(getUnsigned(valueAt(getObject(simple), "int")), 0u);
    ASSERT_EQ(getInteger<int8_t>(valueAt(getObject(simple), "int")), 0);
    ASSERT_EQ(getInteger<int8_t>(valueAt(getObject(simple), "signed")), -1);
}

TEST(getIntegralNumber, wrongAssertions) {
    auto json = R"({ "object": {}, "array": [], "string": "", "int": 0, "signed": -256, "large": 128, "boolean": false })"_json;

    ASSERT_THROW(getUnsigned(valueAt(json, "object")), Error);
    ASSERT_THROW(getUnsigned(valueAt(json, "array")), Error);
    ASSERT_THROW(getUnsigned(valueAt(json, "string")), Error);
    ASSERT_THROW(getUnsigned(valueAt(json, "boolean")), Error);
    ASSERT_THROW(getUnsigned(valueAt(json, "signed")), Error);

    ASSERT_THROW(getInteger<int8_t>(valueAt(json, "object")), Error);
    ASSERT_THROW(getInteger<int8_t>(valueAt(json, "array")), Error);
    ASSERT_THROW(getInteger<int8_t>(valueAt(json, "string")), Error);
    ASSERT_THROW(getInteger<int8_t>(valueAt(json, "boolean")), Error);
    ASSERT_THROW(getInteger<int8_t>(valueAt(json, "large")), Error);
    ASSERT_THROW(getInteger<int8_t>(valueAt(json, "signed")), Error);
}

TEST(getBoolean, rightAssertions) {
    auto simple = R"({ "boolean": false })"_json;

    ASSERT_EQ(getBoolean(valueAt(getObject(simple), "boolean")), false);
}

TEST(getBoolean, wrongAssertions) {
    auto json = R"({ "object": {}, "array": [], "string": "", "int": 0, "boolean": false })"_json;

    ASSERT_THROW(getBoolean(valueAt(json, "object")), Error);
    ASSERT_THROW(getBoolean(valueAt(json, "array")), Error);
    ASSERT_THROW(getBoolean(valueAt(json, "string")), Error);
    ASSERT_THROW(getBoolean(valueAt(json, "int")), Error);
}

TEST(optionalValueAt, existing) {
    auto json = R"({ "string": "ssh-rsa" })"_json;

    ASSERT_EQ(optionalValueAt(json, "string"), std::optional { "ssh-rsa" });
}

TEST(optionalValueAt, empty) {
    auto json = R"({})"_json;

    ASSERT_EQ(optionalValueAt(json, "string"), std::nullopt);
}

TEST(getNullable, null) {
    auto json = R"(null)"_json;

    ASSERT_EQ(getNullable(json), nullptr);
}

TEST(getNullable, empty) {
    auto json = R"({})"_json;

    auto * p = getNullable(json);

    ASSERT_NE(p, nullptr);
    ASSERT_EQ(*p, R"({})"_json);
}

} /* namespace nix */
