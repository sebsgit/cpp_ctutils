#include "CTJson.h"
#include <iostream>

using namespace ctjson;

static constexpr const char jsonNameVal[] = "  \"age\" : 42 ";
static constexpr const char whitespaces_str[] = " trim   ";
static constexpr const char source[] = "x: 7\ny: 'test'";

static constexpr const char tokenTest_CurlyBracket[] = "{\"age\" : 42}";

static void testTokenizerIterations() noexcept
{
    using Input = String<tokenTest_CurlyBracket, 0, sizeof(tokenTest_CurlyBracket) - 1>;
    using FirstIter = ReadToken<Input>;
    using FirstToken = typename FirstIter::Token;
    using RestString = typename FirstIter::Rest;

    static_assert(FirstToken::equals("{"), "");
    static_assert(RestString::equals("\"age\" : 42}"), "");

    using SecondIter = ReadToken<RestString>;
    using SecondToken = typename SecondIter::Token;
    using RestString2 = typename SecondIter::Rest;

    static_assert(SecondToken::equals("\"age\""), "");
    static_assert(RestString2::equals(" : 42}"), "");

    using Iter3 = ReadToken<RestString2>;
    using Token3 = typename Iter3::Token;
    using RestString3 = typename Iter3::Rest;
    static_assert(Token3::equals(":"), "");
    static_assert(RestString3::equals(" 42}"), "");
}

static void testTokenizer()
{
    using Input = String<tokenTest_CurlyBracket, 0, sizeof(tokenTest_CurlyBracket) - 1>;
    using TokenList = JSONTokenizer<Input>;

    using Token0 = TokenList::Token;
    static_assert(Token0::equals("{"), "");

    using Token1 = TokenList::Next::Token;
    static_assert(Token1::equals("\"age\""), "");

    using Token2 = TokenList::Next::Next::Token;
    static_assert(Token2::equals(":"), "");

    using Token3 = TokenList::Next::Next::Next::Token;
    static_assert(Token3::equals("42"), "");

    using Token4 = TokenList::Next::Next::Next::Next::Token;
    static_assert(Token4::equals("}"), "");

    using EndMarker = TokenList::Next::Next::Next::Next::Next::Token;
    static_assert(std::is_same_v<EndMarker, std::false_type>, "");
}

static void testJSONNVParser() noexcept
{
    using Input = String<jsonNameVal, 0, sizeof(jsonNameVal) - 1>;
    using TokenList = JSONTokenizer<Input>;

    using Parser = JSONNameValuePairParser<TokenList>;
    using NV = typename Parser::Result;

    static_assert(Parser::success);

    using name = typename NV::name;
    using value = typename NV::value;

    static_assert(name::size() == 5, "");
    static_assert(name::find("age") == 1, "");
    static_assert(value::String::size() == 2, "");
    static_assert(value::String::toInt() == 42, "");

    using TokensLeft = Parser::NextTokens;
    static_assert(JSONTokenSpecs<TokensLeft>::type == TokenType::Invalid);
}

static constexpr const char tokenTest_IntArray[] = "[10, 20, 30]";

static void testIntArrayParse()
{
    using Input = String<tokenTest_IntArray, 0, sizeof(tokenTest_IntArray) - 1>;
    using TokenList = JSONTokenizer<Input>;

    using ParseArrayImpl = ParseJSONArray<TokenList>;
    using Result = ParseArrayImpl::Result;

    static_assert(Result::length == 3);
    static_assert(Result::At<0>::String::toInt() == 10);
    static_assert(Result::At<1>::String::toInt() == 20);
    static_assert(Result::At<2>::String::toInt() == 30);

    using TokensLeft = ParseArrayImpl::NextTokens;
    static_assert(TokensLeft::type == TokenType::Invalid);
    static_assert(std::is_same_v<TokensLeft::Token, std::false_type>);
}

static constexpr const char tokenTest_StringArray[] = "[\"one\", \"two\"]";

static void testStringArrayParse()
{
    using Input = String<tokenTest_StringArray, 0, sizeof(tokenTest_StringArray) - 1>;
    using TokenList = JSONTokenizer<Input>;

    using ParseArrayImpl = ParseJSONArray<TokenList>;
    using Result = ParseArrayImpl::Result;

    static_assert(Result::length == 2);
    static_assert(Result::At<0>::String::equals("\"one\""));
    static_assert(Result::At<1>::String::equals("\"two\""));

    using TokensLeft = ParseArrayImpl::NextTokens;
    static_assert(TokensLeft::type == TokenType::Invalid);
    static_assert(std::is_same_v<TokensLeft::Token, std::false_type>);
}

static constexpr const char tokenTest_ArraySyntaxError_CloseBracket[] = "[10, 10}";
static void testArrayParse_SyntaxError_0()
{
    using Input = String<tokenTest_ArraySyntaxError_CloseBracket, 0, sizeof(tokenTest_ArraySyntaxError_CloseBracket) - 1>;
    using TokenList = JSONTokenizer<Input>;

    using ParseArrayImpl = ParseJSONArray<TokenList>;

    static_assert(!ParseArrayImpl::success);
    using TokensLeft = ParseArrayImpl::NextTokens;
    static_assert(TokensLeft::type == TokenType::ArrayOpen);
}

static constexpr const char tokenTest_ObjectDef_simple[] = "{ \"age\" : 42 }";
static void testObjectParse()
{
    using Input = String<tokenTest_ObjectDef_simple, 0, sizeof(tokenTest_ObjectDef_simple) - 1>;
    using TokenList = JSONTokenizer<Input>;
    static_assert(JSONTokenSpecs<TokenList>::type == TokenType::DictOpen);
    static_assert(JSONTokenSpecs<TokenList::Next>::type == TokenType::String);
    static_assert(JSONTokenSpecs<TokenList::Next::Next>::type == TokenType::Colon);
    static_assert(JSONTokenSpecs<TokenList::Next::Next::Next>::type == TokenType::Number);
    static_assert(JSONTokenSpecs<TokenList::Next::Next::Next::Next>::type == TokenType::DictClose);

    {
        using NVTokens = TokenList::Next;
        using NVParser = JSONNameValuePairParser<NVTokens>;
        static_assert(NVParser::success);

        using TokensLeft = NVParser::NextTokens;
        static_assert(JSONTokenSpecs<TokensLeft>::type == TokenType::DictClose);
    }

    using Parser = JSONObjectParser<TokenList>;
    using Result = Parser::Result;

    static_assert(Parser::success);
    static_assert(Result::length == 1);
    using Pair = Result::type;
    static_assert(Pair::name::equals("\"age\""));
    static_assert(Pair::value::String::toInt() == 42);

    using TokensLeft = Parser::NextTokens;
    static_assert(TokensLeft::type == TokenType::Invalid);
    static_assert(std::is_same_v<TokensLeft::Token, std::false_type>);

    constexpr auto json_obj { JSONDeclarator<Result>::createObject() };
    static_assert (json_obj.value() == 42, "");
    std::cout << json_obj.name().toString() << ':' << json_obj.value() << '\n';
}

static constexpr const char tokenTest_ObjectDef_simple_two[] = "{ \"age\" : 42, \"name\" : \"Joe\" }";
static void testObjectParseTwo()
{
    using Input = String<tokenTest_ObjectDef_simple_two, 0, sizeof(tokenTest_ObjectDef_simple_two) - 1>;
    using TokenList = JSONTokenizer<Input>;

    using Parser = JSONObjectParser<TokenList>;
    using Result = Parser::Result;

    static_assert(Parser::success);
    static_assert(Result::length == 2);

    using Pair = Result::type;
    static_assert(Pair::name::equals("\"age\""));
    static_assert(Pair::value::String::toInt() == 42);

    using Pair2 = Result::next::type;
    static_assert(Pair2::name::equals("\"name\""));
    static_assert(Pair2::value::String::equals("\"Joe\""));

    using TokensLeft = Parser::NextTokens;
    static_assert(TokensLeft::type == TokenType::Invalid);
    static_assert(std::is_same_v<TokensLeft::Token, std::false_type>);

    constexpr auto json_obj { JSONDeclarator<Result>::createObject() };
    static_assert (json_obj.node().value() == 42);
    static_assert (std::is_same_v<std::decay_t<decltype(json_obj.next().value())>, StringView>);
    std::cout << "multi-value dict: \n";
    std::cout << json_obj.node().name().toString() << ':' << json_obj.node().value() << '\n';
    std::cout << json_obj.next().name().toString() << ':' << json_obj.next().value().toString() << '\n';
    std::cout << "---\n";
}

static constexpr const char tokenTest_ObjectDef_array[] = "{ \"values\" : [1, 2, 3] }";
static void testObjectParseValueArray()
{
    using Input = String<tokenTest_ObjectDef_array, 0, sizeof(tokenTest_ObjectDef_array) - 1>;
    using TokenList = JSONTokenizer<Input>;

    using Parser = JSONObjectParser<TokenList>;
    using Result = Parser::Result;

    static_assert(Parser::success);
    static_assert(Result::length == 1);

    using Pair = Result::type;
    static_assert(Pair::name::equals("\"values\""));
    using Values = Pair::value;
    static_assert(Values::length == 3);
    static_assert(Values::At<0>::String::equals("1"));
    static_assert(Values::At<1>::String::equals("2"));
    static_assert(Values::At<2>::String::equals("3"));

    constexpr auto json_obj { JSONDeclarator<Result>::createObject() };
    constexpr std::array<int32_t, 3> values { json_obj.value() };
    static_assert (values[0] == 1);
    static_assert (values[1] == 2);
    static_assert (values[2] == 3);
}

static void testTypeList()
{
    using I0 = EmptyTypeList;
    static_assert(I0::length == 0);
    static_assert(std::is_same_v<I0::At<0>, std::false_type>);

    using I1 = TypeList<int>;
    static_assert(I1::length == 1);
    static_assert(std::is_same_v<I1::At<0>, int>);
    static_assert(std::is_same_v<I1::At<1>, std::false_type>);

    using I3 = TypeList<int, double, float>;
    static_assert(I3::length == 3);
    static_assert(std::is_same_v<I3::At<0>, int>);
    static_assert(std::is_same_v<I3::At<1>, double>);
    static_assert(std::is_same_v<I3::At<2>, float>);
    static_assert(std::is_same_v<I3::At<3>, std::false_type>);

    using I0_append1 = I0::Append<float>;
    static_assert(I0_append1::length == 1);
    static_assert(std::is_same_v<I0_append1::At<0>, float>);

    using I1_append = I1::Append<std::string>;
    static_assert(I1_append::length == 2);
    static_assert(std::is_same_v<I1_append::At<0>, int>);
    static_assert(std::is_same_v<I1_append::At<1>, std::string>);
    static_assert(std::is_same_v<I1_append::At<2>, std::false_type>);

    using I3_append = I3::Append<bool>;
    static_assert(I3_append::length == 4);
    static_assert(std::is_same_v<I3_append::At<0>, int>);
    static_assert(std::is_same_v<I3_append::At<1>, double>);
    static_assert(std::is_same_v<I3_append::At<2>, float>);
    static_assert(std::is_same_v<I3_append::At<3>, bool>);
    static_assert(std::is_same_v<I3_append::At<4>, std::false_type>);
}

static constexpr const char address_json[] = R"TAG(
{
"address": {
    "streetAddress": "21 2nd Street",
    "city": "New York",
    "state": 76,
    "postalCode": "10021-3100"
  }
}
)TAG";
static void testTokenizeAddress()
{
	using Input = String<address_json, 0, sizeof(address_json) - 1>;
	using TokenList = JSONTokenizer<Input>;
	static_assert(TokenList::At<0>::type == TokenType::DictOpen);
	static_assert(TokenList::At<1>::type == TokenType::String);
	static_assert(TokenList::At<2>::type == TokenType::Colon);

	static_assert(TokenList::At<3>::type == TokenType::DictOpen);
	static_assert(TokenList::At<4>::type == TokenType::String);
	static_assert(TokenList::At<5>::type == TokenType::Colon);
	static_assert(TokenList::At<6>::type == TokenType::String);
	static_assert(TokenList::At<7>::type == TokenType::Comma);
	static_assert(TokenList::At<8>::type == TokenType::String);
	static_assert(TokenList::At<9>::type == TokenType::Colon);
	static_assert(TokenList::At<10>::type == TokenType::String);
	static_assert(TokenList::At<11>::type == TokenType::Comma);
	static_assert(TokenList::At<12>::type == TokenType::String);
	static_assert(TokenList::At<13>::type == TokenType::Colon);
	static_assert(TokenList::At<14>::type == TokenType::Number);
	static_assert(TokenList::At<15>::type == TokenType::Comma);
	static_assert(TokenList::At<16>::type == TokenType::String);
	static_assert(TokenList::At<17>::type == TokenType::Colon);
	static_assert(TokenList::At<18>::type == TokenType::String);
	static_assert(TokenList::At<19>::type == TokenType::DictClose);

	static_assert(TokenList::At<20>::type == TokenType::DictClose);
}
static void testParseAddress()
{
	using Input = String<address_json, 0, sizeof(address_json) - 1>;
	using Parser = JSONParser<Input>;
	static_assert(Parser::success);

	using Result = Parser::Result;
	static_assert(Result::type::name::equals("\"address\""));

    constexpr auto json_obj {JSONDeclarator<Result>::createObject()};

    static_assert (json_obj.contains("\"address\""));
    static_assert (json_obj.contains("\"streetAddress\""));
    static_assert (!json_obj.contains("\"no such node\""));

    static_assert (json_obj.name().equals("\"address\""));

    static_assert (json_obj.value().node().name().equals("\"streetAddress\""));
    static_assert (json_obj.value().node().value().equals("\"21 2nd Street\""));

    static_assert (json_obj.value().next().node().name().equals("\"city\""));
    static_assert (json_obj.value().next().node().value().equals("\"New York\""));

    static_assert (json_obj.value().next().next().node().name().equals("\"state\""));
    static_assert (json_obj.value().next().next().node().value() == 76);

    static_assert (json_obj.value().next().next().next().name().equals("\"postalCode\""));
    static_assert (json_obj.value().next().next().next().value().equals("\"10021-3100\""));

    static_assert (json_obj.get<int32_t>("\"state\"") == 76);
    static_assert (json_obj.get<StringView>("\"city\"").value().equals("\"New York\""));
}

int main()
{
    testJSONNVParser();
    testTokenizerIterations();
    testTokenizer();
    testTypeList();
    testIntArrayParse();
    testStringArrayParse();
    testObjectParse();
    testObjectParseTwo();
    testObjectParseValueArray();
    testArrayParse_SyntaxError_0();
	testTokenizeAddress();
	testParseAddress();

    using Substr1 = String<source, 0, 4>;
    static_assert(Substr1::find("x") == 0, "");
    static_assert(Substr1::find(":") == 1, "");
    static_assert(Substr1::find("7") == 3, "");
    static_assert(Substr1::find("x:") == 0, "");
    static_assert(Substr1::find("x: 7") == 0, "");
    static_assert(Substr1::find(": ") == 1, "");
    static_assert(Substr1::find(": 7") == 1, "");
    static_assert(Substr1::find(" 7") == 2, "");

    using StrOps = StringOps<Substr1>;
    using LeftStr = typename StrOps::left<Substr1::find(":")>;
    using RightStr = typename StrOps::right<Substr1::find(":")>;
    static_assert(LeftStr::size() == 1, "");
    static_assert(LeftStr::find("x") == 0, "");
    static_assert(RightStr::size() == 2, "");
    static_assert(RightStr::at(0) == ' ', "");
    static_assert(RightStr::at(1) == '7', "");

    using SubstrZero = String<source, Substr1::find("7"), 1>;
    static_assert(SubstrZero::toInt() == 7, "");

    static_assert(Substr1::find("x : 7") == std::string::npos, "");
    static_assert(Substr1::find("x: 7\n") == std::string::npos, "");

    std::cout << '\'' << Substr1::toString() << '\'' << '\n';

    using WSTest = String<whitespaces_str, 0, sizeof(whitespaces_str) - 1>;
    using WSTrim = typename WSTest::Trimmed;
    static_assert(WSTrim::size() == 4, "");
    static_assert(WSTrim::at(0) == 't', "");
    static_assert(WSTrim::at(1) == 'r', "");
    static_assert(WSTrim::at(2) == 'i', "");
    static_assert(WSTrim::at(3) == 'm', "");
    static_assert(WSTrim::at(4) == 0, "");

    std::cout << '\'' << WSTest::toString() << '\'' << '\n';
    std::cout << '\'' << WSTrim::toString() << '\'' << '\n';

    return 0;
}
