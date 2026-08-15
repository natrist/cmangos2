#include "JamGen.h"

#include <stdexcept>

class JamGenParser
{
public:
    explicit JamGenParser(std::vector<JamGenToken> const& tokens)
        : m_tokens(tokens)
    {
    }

    JamGenProtocol ParseProtocol();

private:
    JamGenToken const& Peek() const { return m_tokens[m_pos]; }
    JamGenToken const& Advance()    { return m_tokens[m_pos++]; }

    JamGenToken const& Expect(JamGenTokenType type, char const* what);
    void               ExpectKeyword(char const* keyword);
    void               ConsumeOptionalSemicolon();

    static uint32_t ParseInt(std::string const& s)
    {
        return static_cast<uint32_t>(std::stoul(s, nullptr, 0));
    }

    bool IsKnownType(std::string const& name, JamGenMessage const& currentMsg) const;

    JamGenEnumMember ParseEnumMember();
    JamGenEnum       ParseEnum();
    JamGenField      ParseField(JamGenMessage const& currentMsg);
    void             ParseMessageAttributes(JamGenMessage& m);
    JamGenMessage    ParseMessage(JamGenDirection dir);
    void             ParseBlock(JamGenProtocol& p, JamGenDirection dir);

    std::vector<JamGenToken> const& m_tokens;
    size_t                          m_pos = 0;
};

JamGenToken const& JamGenParser::Expect(JamGenTokenType type, char const* what)
{
    JamGenToken const& tok = Peek();

    if (tok.type != type)
    {
        throw std::runtime_error("Expected " + std::string(what) + " at line " +
                                 std::to_string(tok.line) + ", got '" + tok.value + "'");
    }

    return Advance();
}

void JamGenParser::ExpectKeyword(char const* keyword)
{
    JamGenToken const& tok = Peek();

    if (tok.type != JamGenTokenType::Identifier || tok.value != keyword)
    {
        throw std::runtime_error("Expected keyword '" + std::string(keyword) + "' at line " +
                                 std::to_string(tok.line) + ", got '" + tok.value + "'");
    }

    Advance();
}

void JamGenParser::ConsumeOptionalSemicolon()
{
    if (Peek().type == JamGenTokenType::Semicolon)
        Advance();
}

bool JamGenParser::IsKnownType(std::string const& name, JamGenMessage const& currentMsg) const
{
    return JamGenIsBuiltinType(name) || currentMsg.FindEnum(name) != nullptr;
}

JamGenEnumMember JamGenParser::ParseEnumMember()
{
    JamGenEnumMember m;

    m.name = Expect(JamGenTokenType::Identifier, "enum member name").value;
    Expect(JamGenTokenType::Equals, "=");
    m.value = ParseInt(Expect(JamGenTokenType::Number, "enum member value").value);
    Expect(JamGenTokenType::Semicolon, ";");

    return m;
}

JamGenEnum JamGenParser::ParseEnum()
{
    ExpectKeyword("enum");

    JamGenEnum e;
    e.name = Expect(JamGenTokenType::Identifier, "enum name").value;

    // Optional wire width: enum Error : u8 { ... }  (default u32)
    if (Peek().type == JamGenTokenType::Colon)
    {
        Advance();
        JamGenToken const& widthTok = Expect(JamGenTokenType::Identifier, "enum wire type");
        e.wireType = widthTok.value;

        if (!JamGenIsIntegerType(e.wireType))
        {
            throw std::runtime_error("Invalid enum wire type '" + e.wireType +
                                     "' at line " + std::to_string(widthTok.line) +
                                     " (expected u8/u16/u32/u64 or i8/i16/i32/i64)");
        }
    }

    Expect(JamGenTokenType::LBrace, "{");

    while (Peek().type != JamGenTokenType::RBrace)
        e.members.push_back(ParseEnumMember());

    Advance();
    ConsumeOptionalSemicolon();
    return e;
}

JamGenField JamGenParser::ParseField(JamGenMessage const& currentMsg)
{
    JamGenField f;

    JamGenToken const& typeTok = Expect(JamGenTokenType::Identifier, "field type");
    f.typeName = typeTok.value;

    if (f.typeName == "array")
    {
        Expect(JamGenTokenType::LAngle, "<");

        JamGenToken const& elemTok = Expect(JamGenTokenType::Identifier, "array element type");
        f.elementType = elemTok.value;

        if (!IsKnownType(f.elementType, currentMsg))
        {
            throw std::runtime_error("Unknown array element type '" + f.elementType +
                                     "' at line " + std::to_string(elemTok.line));
        }

        Expect(JamGenTokenType::RAngle, ">");
    }
    else if (!IsKnownType(f.typeName, currentMsg))
    {
        throw std::runtime_error("Unknown type '" + f.typeName + "' at line " +
                                 std::to_string(typeTok.line));
    }

    f.name = Expect(JamGenTokenType::Identifier, "field name").value;

    if (Peek().type == JamGenTokenType::LBracket)
    {
        Advance();
        f.size = ParseInt(Expect(JamGenTokenType::Number, "array size").value);
        Expect(JamGenTokenType::RBracket, "]");
    }

    if (Peek().type == JamGenTokenType::Equals)
    {
        Advance();
        f.defaultValue = Advance().value;
    }

    Expect(JamGenTokenType::Semicolon, ";");
    return f;
}

void JamGenParser::ParseMessageAttributes(JamGenMessage& m)
{
    if (Peek().type != JamGenTokenType::LBracket)
        return;

    Advance();

    while (Peek().type != JamGenTokenType::RBracket)
    {
        std::string const key = Expect(JamGenTokenType::Identifier, "attribute name").value;
        Expect(JamGenTokenType::Equals, "=");
        std::string const value = Advance().value;

        if (key == "code")
            m.code = ParseInt(value);

        if (Peek().type == JamGenTokenType::Comma)
            Advance();
    }

    Advance();
}

JamGenMessage JamGenParser::ParseMessage(JamGenDirection dir)
{
    ExpectKeyword("message");

    JamGenMessage m;
    m.direction = dir;
    m.name      = Expect(JamGenTokenType::Identifier, "message name").value;
    ParseMessageAttributes(m);
    Expect(JamGenTokenType::LBrace, "{");

    while (Peek().type != JamGenTokenType::RBrace)
    {
        if (Peek().type == JamGenTokenType::Identifier && Peek().value == "enum")
            m.enums.push_back(ParseEnum());
        else
            m.fields.push_back(ParseField(m));
    }

    Advance();
    ConsumeOptionalSemicolon();
    return m;
}

void JamGenParser::ParseBlock(JamGenProtocol& p, JamGenDirection dir)
{
    Advance();
    Expect(JamGenTokenType::LBrace, "{");

    while (Peek().type != JamGenTokenType::RBrace)
        p.messages.push_back(ParseMessage(dir));

    Advance();
    ConsumeOptionalSemicolon();
}

JamGenProtocol JamGenParser::ParseProtocol()
{
    JamGenProtocol p;

    while (Peek().type != JamGenTokenType::EndOfFile)
    {
        JamGenToken const& dirTok = Peek();

        if (dirTok.type != JamGenTokenType::Identifier)
        {
            throw std::runtime_error("Expected 'inbound' or 'outbound' at line " +
                                     std::to_string(dirTok.line));
        }

        if (dirTok.value == "inbound")
            ParseBlock(p, JamGenDirection::Inbound);
        else if (dirTok.value == "outbound")
            ParseBlock(p, JamGenDirection::Outbound);
        else
        {
            throw std::runtime_error("Expected 'inbound' or 'outbound' at line " +
                                     std::to_string(dirTok.line) + ", got '" + dirTok.value + "'");
        }
    }

    return p;
}

JamGenProtocol JamGenParse(std::vector<JamGenToken> const& tokens)
{
    return JamGenParser(tokens).ParseProtocol();
}
