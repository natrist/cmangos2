#ifndef JAMGEN_H_
#define JAMGEN_H_

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


enum class JamGenTokenType
{
    Identifier,
    Number,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
    LAngle,
    RAngle,
    Comma,
    Equals,
    Colon,
    Semicolon,
    EndOfFile,
};

struct JamGenToken
{
    JamGenTokenType type;
    std::string     value;
    uint32_t        line;
};


inline uint32_t JamGenWireSize(std::string_view t)
{
    if (t == "u8"  || t == "i8" || t == "bool" || t == "char") return 1;
    if (t == "u16" || t == "i16")                             return 2;
    if (t == "u32" || t == "i32" || t == "float")             return 4;
    if (t == "u64" || t == "i64" || t == "double")            return 8;
    if (t == "C3Vector")                                      return 12;
    return 0;
}

inline bool JamGenIsIntegerType(std::string_view t)
{
    return t == "u8" || t == "u16" || t == "u32" || t == "u64"
        || t == "i8" || t == "i16" || t == "i32" || t == "i64";
}

inline bool JamGenIsBuiltinType(std::string_view t)
{
    return t == "string" || t == "float" || t == "double" || t == "bool"
        || t == "char"   || t == "C3Vector" || JamGenIsIntegerType(t);
}


enum class JamGenDirection
{
    Inbound,
    Outbound,
};

struct JamGenField
{
    std::string typeName;
    std::string elementType;    // set when typeName == "array"
    std::string name;
    std::string defaultValue;
    uint32_t    size = 0;       // fixed array / string max; 0 = unbounded
};

struct JamGenEnumMember
{
    std::string name;
    uint32_t    value;
};

struct JamGenEnum
{
    std::string                   name;
    std::string                   wireType = "u32";
    std::vector<JamGenEnumMember> members;
};

struct JamGenMessage
{
    std::string              name;
    uint32_t                 code      = 0;
    JamGenDirection          direction = JamGenDirection::Inbound;
    std::vector<JamGenEnum>  enums;
    std::vector<JamGenField> fields;

    JamGenEnum const* FindEnum(std::string_view typeName) const
    {
        for (JamGenEnum const& e : enums)
        {
            if (e.name == typeName)
                return &e;
        }
        return nullptr;
    }

    uint32_t ScalarSize(std::string_view typeName) const
    {
        if (JamGenEnum const* e = FindEnum(typeName))
            return JamGenWireSize(e->wireType);
        return JamGenWireSize(typeName);
    }
};

struct JamGenProtocol
{
    std::vector<JamGenMessage> messages;

    bool HasInbound() const
    {
        for (JamGenMessage const& m : messages)
        {
            if (m.direction == JamGenDirection::Inbound)
                return true;
        }
        return false;
    }
};


std::vector<JamGenToken> JamGenTokenize(std::string const& source);
JamGenProtocol           JamGenParse(std::vector<JamGenToken> const& tokens);

void JamGenEmit(std::string const&    fileName,
                std::string const&    classPrefix,
                JamGenProtocol const& protocol,
                std::string&          headerOut,
                std::string&          sourceOut);

#endif // JAMGEN_H_
