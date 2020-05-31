#pragma once

#include "StringUtils.h"
#include "TypeListUtils.h"
#include <string>
#include <type_traits>

///
/// Utilities for compile-time parsing of JSON
///
namespace ctjson
{

	template <typename InputString, size_t tokenLength>
	class ReadTokenResult;

	template <typename InputString>
	class ReadTokenResult<InputString, 0> {
	public:
		using Token = EmptyString;
		using Rest = EmptyString;
	};

	template <typename InputString, size_t tokenLength>
	class ReadTokenResult {
		using Ops = StringOps<InputString>;

	public:
		using Token = typename InputString::template Substring<0, tokenLength>;
		using Rest = typename Ops::template right<tokenLength - 1>;
	};

	enum class TokenType : uint8_t {
		String,
		Number,
		Boolean,
		Null,
		DictOpen,
		DictClose,
		ArrayOpen,
		ArrayClose,
		Colon,
		Comma,
		Invalid
	};

	template <typename InputString>
	class ReadToken {
		static constexpr bool isNumber(char c) noexcept
		{
			return c >= '0' && c <= '9';
		}

	private:
		using Input = typename InputString::Trimmed;

		static constexpr std::pair<TokenType, size_t> nextTokenSpecs() noexcept
		{
			if constexpr (Input::at(0) == '{' || Input::at(0) == '}') {
				return { (Input::at(0) == '{' ? TokenType::DictOpen : TokenType::DictClose), 1 };
			}
			else if constexpr (Input::at(0) == '[' || Input::at(0) == ']') {
				return { (Input::at(0) == '[' ? TokenType::ArrayOpen : TokenType::ArrayClose), 1 };
			}
			else if constexpr (Input::at(0) == ':') {
				return { TokenType::Colon, 1 };
			}
			else if constexpr (Input::at(0) == ',') {
				return { TokenType::Comma, 1 };
			}
			else if constexpr (Input::at(0) == '"') {
				constexpr auto pos{ Input::find("\"", 1) };
				return { (pos != std::string::npos) ? TokenType::String : TokenType::Invalid, (pos != std::string::npos) ? pos + 1 : 0 };
			}
			else if constexpr (isNumber(Input::at(0))) {
				for (size_t i = 1; i < Input::size(); ++i) {
					//TODO support float literals
					if (!isNumber(Input::at(i))) {
						return { TokenType::Number, i };
					}
				}
				return { TokenType::Number, Input::size() };
			}
			else if constexpr ((Input::find("true") == 0) || (Input::find("false") == 0)) {
				return { TokenType::Boolean, 4 };
			}
			else if constexpr (Input::find("null") == 0) {
				return { TokenType::Null, 4 };
			}
			return { TokenType::Invalid, 0 };
		}

		static constexpr auto tokenSpec{ nextTokenSpecs() };
		using Result = ReadTokenResult<Input, tokenSpec.second>;

	public:
		using Token = typename Result::Token;
		using Rest = typename Result::Rest;
		static constexpr TokenType type{ tokenSpec.first };
	};

	template <typename InputString, bool is_empty = (InputString::size() == 0)>
	class JSONTokenizer;

	template <typename InputString>
	class JSONTokenizer<InputString, false> {
		using Tokenizer = ReadToken<InputString>;

	public:
		using Token = typename Tokenizer::Token;
		using Next = JSONTokenizer<typename Tokenizer::Rest>;
		static constexpr TokenType type{ Tokenizer::type };

		template <size_t idx>
		using At = std::conditional_t < idx == 0,
			JSONTokenizer<InputString>,
			typename Next::template At<idx - 1>
		>;
	};

	template <typename InputString>
	class JSONTokenizer<InputString, true> {
	public:
		using Token = std::false_type;
		using Next = std::false_type;
		static constexpr TokenType type{ TokenType::Invalid };

		template <size_t idx>
		using At = std::false_type;
	};

	template <typename TokenT>
	class JSONTokenSpecs {
	public:
		static constexpr auto type = TokenT::type;
	};

	template <>
	class JSONTokenSpecs<std::false_type> {
	public:
		static constexpr auto type = TokenType::Invalid;
	};

	template <typename Name, typename Value>
	struct JSONNameValue {
		using name = Name;
		using value = Value;
	};

	template <TokenType Type, typename Value>
	struct JSONBaseValue
	{
		static constexpr TokenType type = Type;
		using String = Value;
	};

	template <typename TokenList>
	class ParseJSONName {
	public:
		using Result = typename TokenList::Token;
		constexpr static bool success{
			JSONTokenSpecs<TokenList>::type == TokenType::String
		};
		using NextTokens = std::conditional_t<success, typename TokenList::Next, TokenList>;
	};

	template <typename TokenList, TokenType type>
	class ParseJSONValueImpl;

	template <typename TokenList>
	class JSONObjectParser;

	template <typename TokenList>
	class ParseJSONValueImpl<TokenList, TokenType::DictOpen> {
		using TryParseObject = JSONObjectParser<TokenList>;

	public:
		constexpr static bool success{
			TryParseObject::success
		};
		using Result = typename TryParseObject::Result;
		using NextTokens = std::conditional_t<success, typename TryParseObject::NextTokens, TokenList>;
	};

	template <typename TokenList>
	class ParseJSONValueImpl<TokenList, TokenType::String> {
	public:
		constexpr static bool success{
			JSONTokenSpecs<TokenList>::type == TokenType::String
		};
		using Result = JSONBaseValue<TokenType::String, typename TokenList::Token>;
		using NextTokens = std::conditional_t<success, typename TokenList::Next, TokenList>;
	};

	template <typename TokenList>
	class ParseJSONValueImpl<TokenList, TokenType::Number> {
	public:
		constexpr static bool success{
			JSONTokenSpecs<TokenList>::type == TokenType::Number
		};
		using Result = JSONBaseValue<TokenType::Number, typename TokenList::Token>;
		using NextTokens = std::conditional_t<success, typename TokenList::Next, TokenList>;
	};

	template <typename TokenList>
	class ParseJSONValueImpl<TokenList, TokenType::Boolean> {
	public:
		constexpr static bool success{
			JSONTokenSpecs<TokenList>::type == TokenType::Boolean
		};
		using Result = JSONBaseValue<TokenType::Boolean, typename TokenList::Token>;
		using NextTokens = std::conditional_t<success, typename TokenList::Next, TokenList>;
	};

	template <typename TokenList>
	class ParseJSONValueImpl<TokenList, TokenType::Null> {
	public:
		constexpr static bool success{
			JSONTokenSpecs<TokenList>::type == TokenType::Null
		};
		using Result = JSONBaseValue<TokenType::Null, typename TokenList::Token>;
		using NextTokens = std::conditional_t<success, typename TokenList::Next, TokenList>;
	};

	template <typename ResultList, typename TokenList, TokenType token>
	class ParseOneJSONArrayValue;

	template <typename ResultList, typename TokenList>
	class ParseOneJSONArrayValue<ResultList, TokenList, TokenType::ArrayOpen> {
		using ParserImpl = ParseOneJSONArrayValue<ResultList, typename TokenList::Next, JSONTokenSpecs<typename TokenList::Next>::type>;

	public:
		using Result = typename ParserImpl::Result;

		static constexpr bool success{
			ParserImpl::success
		};

		using NextTokens = std::conditional_t<success, typename ParserImpl::NextTokens, TokenList>;
	};

	template <typename ResultList, typename TokenList>
	class ParseOneJSONArrayValue<ResultList, TokenList, TokenType::ArrayClose> {
	public:
		using Result = ResultList;
		using NextTokens = typename TokenList::Next;

		static constexpr bool success{
			true
		};
	};

	template <typename ResultList, typename TokenList>
	class ParseOneJSONArrayValue<ResultList, TokenList, TokenType::DictClose> {
	public:
		using Result = ResultList;
		using NextTokens = TokenList;

		static constexpr bool success{
			false
		};
	};

	template <typename ResultList, typename TokenList>
	class ParseOneJSONArrayValue<ResultList, TokenList, TokenType::Comma> {
		using ParserImpl = ParseOneJSONArrayValue<ResultList, typename TokenList::Next, JSONTokenSpecs<typename TokenList::Next>::type>;

	public:
		using Result = typename ParserImpl::Result;

		static constexpr bool success{
			ParserImpl::success
		};

		using NextTokens = std::conditional_t<success, typename ParserImpl::NextTokens, TokenList>;
	};

	template <typename ResultList, typename TokenList, TokenType token = JSONTokenSpecs<TokenList>::type>
	class ParseOneJSONArrayValue {
		using ParseValue = ParseJSONValueImpl<TokenList, token>;
		using ThisResult = typename ResultList::template Append<typename ParseValue::Result>;

		using ParserImpl = ParseOneJSONArrayValue<ThisResult, typename ParseValue::NextTokens>;

	public:
		using Result = typename ParserImpl::Result;

		constexpr static bool success{
			ParserImpl::success
		};

		using NextTokens = std::conditional_t<success, typename ParserImpl::NextTokens, TokenList>;
	};

	template <typename TokenList>
	class ParseJSONArray {
		using ParserImpl = ParseOneJSONArrayValue<EmptyTypeList, TokenList>;

	public:
		using Result = typename ParserImpl::Result;

		constexpr static bool success{
			ParserImpl::success
		};

		using NextTokens = std::conditional_t<success, typename ParserImpl::NextTokens, TokenList>;
	};

	template <typename TokenList>
	class ParseJSONValueImpl<TokenList, TokenType::ArrayOpen> {
		using ParseArray = ParseJSONArray<TokenList>;

	public:
		static constexpr bool success{
			ParseArray::success
		};
		using Result = typename ParseArray::Result;
		using NextTokens = std::conditional_t<success, typename ParseArray::NextTokens, TokenList>;
	};

	template <typename TokenList>
	class ParseJSONValue {
		using ValueParser = ParseJSONValueImpl<TokenList, JSONTokenSpecs<TokenList>::type>;

	public:
		constexpr static bool success{
			ValueParser::success
		};
		using Result = typename ValueParser::Result;
		using NextTokens = std::conditional_t<success, typename ValueParser::NextTokens, TokenList>;
	};

	template <typename TokenList>
	class JSONNameValuePairParser {
	private:
		using ParseName = ParseJSONName<TokenList>;
		using MoreTokens = typename ParseName::NextTokens;

		constexpr static bool colon_ok{ JSONTokenSpecs<MoreTokens>::type == TokenType::Colon };
		using ParseValue = ParseJSONValue<typename MoreTokens::Next>;

	public:
		using Name = typename ParseName::Result;
		using Value = typename ParseValue::Result;

		constexpr static bool success{ ParseName::success && colon_ok && ParseValue::success };
		using Result = JSONNameValue<Name, Value>;
		using NextTokens = std::conditional_t<success, typename ParseValue::NextTokens, TokenList>;
	};

	template <typename ResultList, typename TokenList, TokenType token = JSONTokenSpecs<TokenList>::type>
	class ParseSingleJSONObjectEntry;

	template <typename ResultList, typename TokenList>
	class ParseSingleJSONObjectEntry<ResultList, TokenList, TokenType::DictOpen> {
		using ParseMore = ParseSingleJSONObjectEntry<ResultList, typename TokenList::Next>;

	public:
		using Result = typename ParseMore::Result;

		static constexpr bool success{
			ParseMore::success
		};

		using NextTokens = std::conditional_t<success, typename ParseMore::NextTokens, TokenList>;
	};

	template <typename ResultList, typename TokenList>
	class ParseSingleJSONObjectEntry<ResultList, TokenList, TokenType::DictClose> {
	public:
		using Result = ResultList;

		static constexpr bool success{
			true
		};

		using NextTokens = typename TokenList::Next;
	};

	template <typename ResultList, typename TokenList>
	class ParseSingleJSONObjectEntry<ResultList, TokenList, TokenType::Comma> {
		using ParseMore = ParseSingleJSONObjectEntry<ResultList, typename TokenList::Next>;

	public:
		using Result = typename ParseMore::Result;

		static constexpr bool success{
			ParseMore::success
		};

		using NextTokens = std::conditional_t<success, typename ParseMore::NextTokens, TokenList>;
	};

	template <typename ResultList, typename TokenList>
	class ParseSingleJSONObjectEntry<ResultList, TokenList, TokenType::String> {
		using ParseEntry = JSONNameValuePairParser<TokenList>;
		using ThisResult = typename ResultList::template Append<typename ParseEntry::Result>;

		using ParseMore = ParseSingleJSONObjectEntry<ThisResult, typename ParseEntry::NextTokens>;

	public:
		using Result = typename ParseMore::Result;
		static constexpr bool success{
			ParseMore::success
		};

		using NextTokens = std::conditional_t<success, typename ParseMore::NextTokens, TokenList>;
	};

	template <typename TokenList>
	class JSONObjectParser {
		using ParseObj = ParseSingleJSONObjectEntry<EmptyTypeList, TokenList>;

	public:
		using Result = typename ParseObj::Result;

		static constexpr bool success{
			ParseObj::success
		};

		using NextTokens = std::conditional_t<success, typename ParseObj::NextTokens, TokenList>;
	};

	template <typename Input>
	class JSONParser {
	private:
		using Tokens = JSONTokenizer<Input>;
		using Parser = JSONObjectParser<Tokens>;

	public:
		using Result = typename Parser::Result;
		static constexpr bool success{
			Parser::success
		};
	};
} // namespace ctjson