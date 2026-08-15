#include "JamGen.h"

#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// =============================================================================
// Lexer
// =============================================================================

static bool IsIdentStart(char c)
{
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

static bool IsIdentChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

std::vector<JamGenToken> JamGenTokenize(std::string const& source)
{
    std::vector<JamGenToken> tokens;
    size_t   i    = 0;
    uint32_t line = 1;

    auto Push = [&](JamGenTokenType type, std::string value)
    {
        tokens.push_back({ type, std::move(value), line });
    };

    while (i < source.size())
    {
        char const c = source[i];

        if (c == '\n')
        {
            ++line;
            ++i;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(c)))
        {
            ++i;
            continue;
        }

        if (c == '/' && i + 1 < source.size() && source[i + 1] == '/')
        {
            i += 2;
            while (i < source.size() && source[i] != '\n')
                ++i;
            continue;
        }

        switch (c)
        {
            case '{': Push(JamGenTokenType::LBrace,    "{"); ++i; continue;
            case '}': Push(JamGenTokenType::RBrace,    "}"); ++i; continue;
            case '[': Push(JamGenTokenType::LBracket,  "["); ++i; continue;
            case ']': Push(JamGenTokenType::RBracket,  "]"); ++i; continue;
            case '<': Push(JamGenTokenType::LAngle,    "<"); ++i; continue;
            case '>': Push(JamGenTokenType::RAngle,    ">"); ++i; continue;
            case ',': Push(JamGenTokenType::Comma,     ","); ++i; continue;
            case '=': Push(JamGenTokenType::Equals,    "="); ++i; continue;
            case ':': Push(JamGenTokenType::Colon,     ":"); ++i; continue;
            case ';': Push(JamGenTokenType::Semicolon, ";"); ++i; continue;
            default: break;
        }

        if (IsIdentStart(c))
        {
            size_t const start = i;
            while (i < source.size() && IsIdentChar(source[i]))
                ++i;
            Push(JamGenTokenType::Identifier, source.substr(start, i - start));
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c)) ||
            (c == '-' && i + 1 < source.size() && std::isdigit(static_cast<unsigned char>(source[i + 1]))))
        {
            size_t const start = i;

            if (c == '-')
                ++i;

            if (source[i] == '0' && i + 1 < source.size() && (source[i + 1] == 'x' || source[i + 1] == 'X'))
            {
                i += 2;
                while (i < source.size() && std::isxdigit(static_cast<unsigned char>(source[i])))
                    ++i;
            }
            else
            {
                while (i < source.size() && std::isdigit(static_cast<unsigned char>(source[i])))
                    ++i;

                if (i < source.size() && source[i] == '.')
                {
                    ++i;
                    while (i < source.size() && std::isdigit(static_cast<unsigned char>(source[i])))
                        ++i;
                }

                if (i < source.size() && (source[i] == 'f' || source[i] == 'F'))
                    ++i;
            }

            Push(JamGenTokenType::Number, source.substr(start, i - start));
            continue;
        }

        throw std::runtime_error("Unexpected character '" + std::string(1, c) +
                                 "' at line " + std::to_string(line));
    }

    Push(JamGenTokenType::EndOfFile, "");
    return tokens;
}

// =============================================================================
// Emitter
// =============================================================================

static char const s_notice[] =
    "//\n"
    "// NOTICE: This is generated code. DO NOT EDIT!\n"
    "//\n";

static std::string MakeGuard(std::string const& fileName)
{
    std::string g = "__";

    for (size_t i = 0; i < fileName.size(); ++i)
    {
        char const c = fileName[i];
        if (std::isupper(static_cast<unsigned char>(c)) && i > 0)
            g += '_';
        g += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    g += "_H__";
    return g;
}

static std::string HexCode(uint32_t code)
{
    char buf[16];
    std::snprintf(buf, sizeof(buf), "0x%03x", code);
    return buf;
}

static uint32_t FieldFixedSize(JamGenMessage const& m, JamGenField const& f)
{
    if (f.typeName == "string")
        return 0;
    if (f.typeName == "array")
        return 4;
    if (f.typeName == "char" && f.size > 0)
        return f.size;
    return m.ScalarSize(f.typeName);
}

static uint32_t MessageFixedSize(JamGenMessage const& m)
{
    uint32_t n = 0;
    for (JamGenField const& f : m.fields)
        n += FieldFixedSize(m, f);
    return n;
}

static void EmitFieldDecl(std::ostringstream& os, JamGenField const& f)
{
    if (f.typeName == "string")
    {
        os << "    JamDynamicString " << f.name << ";\n";
        return;
    }

    if (f.typeName == "array")
    {
        std::string const elem = (f.elementType == "string") ? "JamDynamicString" : f.elementType;
        os << "    std::vector<" << elem << "> " << f.name << ";\n";
        return;
    }

    if (f.size > 0)
    {
        os << "    " << f.typeName << " " << f.name << "[" << f.size << "];\n";
        return;
    }

    os << "    " << f.typeName << " " << f.name;
    if (!f.defaultValue.empty())
        os << " = " << f.defaultValue;
    os << ";\n";
}

static void EmitReadField(std::ostringstream& os, JamGenMessage const& m, JamGenField const& f)
{
    if (f.typeName == "string")
    {
        os << "    " << f.name << ".Read(data, "
           << (f.size ? std::to_string(f.size) : "0xFFFFFFu") << ");\n";
        return;
    }

    if (f.typeName == "array")
    {
        os << "    JamDynamicArray::Read(data, " << f.name << ", "
           << (f.size ? f.size : 0xFFFFFFu) << ");\n";
        return;
    }

    if (f.size > 0 && f.typeName == "char")
    {
        os << "    data.GetString(" << f.name << ", " << f.size << ");\n";
        return;
    }

    if (JamGenEnum const* e = m.FindEnum(f.typeName))
    {
        os << "    { " << e->wireType << " t; data >> t; " << f.name
           << " = static_cast<" << f.typeName << ">(t); }\n";
        return;
    }

    os << "    data >> " << f.name << ";\n";
}

static void EmitWriteField(std::ostringstream& os, JamGenMessage const& m, JamGenField const& f)
{
    if (f.typeName == "string")
    {
        os << "    " << f.name << ".Write(data);\n";
        return;
    }

    if (f.typeName == "array")
    {
        os << "    JamDynamicArray::Write(data, " << f.name << ");\n";
        return;
    }

    if (f.size > 0 && f.typeName == "char")
    {
        os << "    data.PutString(" << f.name << ");\n";
        return;
    }

    if (JamGenEnum const* e = m.FindEnum(f.typeName))
    {
        os << "    data << static_cast<" << e->wireType << ">(" << f.name << ");\n";
        return;
    }

    os << "    data << " << f.name << ";\n";
}

static void EmitSizeField(std::ostringstream& os, JamGenMessage const& m, JamGenField const& f)
{
    if (f.typeName == "string")
    {
        os << "    n += static_cast<u32>(" << f.name << ".size()) + 1;\n";
        return;
    }

    if (f.typeName == "array")
    {
        if (f.elementType == "string")
            os << "    for (auto const& e : " << f.name << ") n += static_cast<u32>(e.size()) + 1;\n";
        else
            os << "    n += static_cast<u32>(" << f.name << ".size()) * "
               << m.ScalarSize(f.elementType) << ";\n";
    }
}

// Blizzard-style message:
//   Foo(conn, packet)  // ctor unpacks
//   Foo::CallHandler() // s_handler(conn, this)
//   JamFoo::Dispatch   // switch that constructs + CallHandler
static void EmitMessageClass(std::ostringstream& os, JamGenMessage const& m)
{
    os << s_notice;
    os << "class " << m.name << " : public JamMessage\n{\npublic:\n";
    os << "    static const u16 CODE = " << HexCode(m.code) << ";\n\n";

    for (JamGenEnum const& e : m.enums)
    {
        os << "    enum class " << e.name << " : " << e.wireType << " {\n";
        for (JamGenEnumMember const& mb : e.members)
            os << "        " << mb.name << " = " << mb.value << ",\n";
        os << "    };\n\n";
    }

    if (m.direction == JamGenDirection::Inbound)
    {
        os << "    using HandlerFn = JAM_RESULT (*)(WowConnection* conn, " << m.name << "* msg);\n";
        os << "    static HandlerFn s_handler;\n\n";
        os << "    " << m.name << "(WowConnection* conn, WorldPacket& packet);\n";
        os << "    JAM_RESULT CallHandler();\n\n";
        os << "    void Get(ByteBuffer& data) override;\n";
    }
    else
    {
        os << "    " << m.name << "() = default;\n\n";
        os << "    void Put(ByteBuffer& data) const override;\n";
    }

    os << "    u16         GetCode() const override { return CODE; }\n";
    os << "    char const* GetName() const override { return \"" << m.name << "\"; }\n";
    os << "    u32         GetSize() const override;\n\n";

    os << "    /*** DATA START ***/\n";
    for (JamGenField const& f : m.fields)
        EmitFieldDecl(os, f);
    os << "    /*** DATA STOP ***/\n";

    os << "};\n\n";
}

static void EmitMessageMethods(std::ostringstream& os, JamGenMessage const& m)
{
    os << s_notice;

    if (m.direction == JamGenDirection::Inbound)
    {
        os << m.name << "::HandlerFn " << m.name << "::s_handler = nullptr;\n\n";

        os << m.name << "::" << m.name << "(WowConnection* conn, WorldPacket& packet)\n";
        os << "    : JamMessage(conn)\n{\n";
        os << "    Get(packet);\n";
        os << "}\n\n";

        os << "JAM_RESULT " << m.name << "::CallHandler()\n{\n";
        os << "    if (!s_handler)\n";
        os << "        return JAM_FAILED;\n";
        os << "    return s_handler(m_connection, this);\n";
        os << "}\n\n";

        os << "void " << m.name << "::Get(ByteBuffer& data)\n{\n";
        for (JamGenField const& f : m.fields)
            EmitReadField(os, m, f);
        os << "}\n\n";
    }
    else
    {
        os << "void " << m.name << "::Put(ByteBuffer& data) const\n{\n";
        for (JamGenField const& f : m.fields)
            EmitWriteField(os, m, f);
        os << "}\n\n";
    }

    os << "u32 " << m.name << "::GetSize() const\n{\n";
    os << "    u32 n = " << MessageFixedSize(m) << ";\n";
    for (JamGenField const& f : m.fields)
        EmitSizeField(os, m, f);
    os << "    return n;\n";
    os << "}\n\n";
}

static std::string EmitHeader(std::string const& fileName, std::string const& classPrefix, JamGenProtocol const& p)
{
    std::ostringstream os;
    std::string const  guard = MakeGuard(fileName);

    os << "#ifndef " << guard << "\n";
    os << "#define " << guard << "\n\n";
    os << "#include \"JamAutoCode/JamMessage.h\"\n";
    os << "#include \"Jam/JamDynamicString.h\"\n";
    os << "#include \"Jam/JamDynamicArray.h\"\n\n";

    if (p.HasInbound())
        os << "class WorldPacket;\n\n";

    for (JamGenMessage const& m : p.messages)
        EmitMessageClass(os, m);

    if (p.HasInbound())
    {
        os << s_notice;
        os << "class Jam" << classPrefix << "\n{\npublic:\n";
        os << "    static bool Dispatch(WowConnection* conn, WorldPacket& packet);\n";
        os << "};\n\n";
    }

    os << "#endif\n";
    return os.str();
}

static std::string EmitSource(std::string const& fileName, std::string const& classPrefix, JamGenProtocol const& p)
{
    std::ostringstream os;

    os << "#include \"" << fileName << ".h\"\n";
    os << "#include \"Server/WorldPacket.h\"\n\n";

    for (JamGenMessage const& m : p.messages)
        EmitMessageMethods(os, m);

    if (!p.HasInbound())
        return os.str();

    os << s_notice;
    os << "bool Jam" << classPrefix << "::Dispatch(WowConnection* conn, WorldPacket& packet)\n{\n";
    os << "    switch (packet.GetOpcode())\n    {\n";

    for (JamGenMessage const& m : p.messages)
    {
        if (m.direction != JamGenDirection::Inbound)
            continue;

        os << "        case " << m.name << "::CODE:\n";
        os << "        {\n";
        os << "            " << m.name << " msg(conn, packet);\n";
        os << "            msg.CallHandler();\n";
        os << "            return true;\n";
        os << "        }\n";
    }

    os << "        default:\n";
    os << "            return false;\n";
    os << "    }\n";
    os << "}\n";

    return os.str();
}

void JamGenEmit(std::string const&    fileName,
                std::string const&    classPrefix,
                JamGenProtocol const& protocol,
                std::string&          headerOut,
                std::string&          sourceOut)
{
    headerOut = EmitHeader(fileName, classPrefix, protocol);
    sourceOut = EmitSource(fileName, classPrefix, protocol);
}

//=============================================================================
static std::string ReadFile(std::string const& path)
{
    std::ifstream f(path);
    if (!f)
        throw std::runtime_error("Cannot open input file: " + path);

    std::ostringstream os;
    os << f.rdbuf();
    return os.str();
}

//=============================================================================
static void WriteFile(std::string const& path, std::string const& content)
{
    std::ofstream f(path);
    if (!f)
        throw std::runtime_error("Cannot open output file: " + path);

    f << content;
}

//=============================================================================
//=============================================================================
//=============================================================================
int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input.jam> <output-dir>\n";
        return 1;
    }

    try
    {
        std::string const inputPath   = argv[1];
        std::string const outputDir   = argv[2];
        std::string const inputBase   = std::filesystem::path(inputPath).stem().string();
        std::string const fileName    = "Jam" + inputBase;
        std::string const classPrefix = inputBase;

        std::string const      source   = ReadFile(inputPath);
        JamGenProtocol const   protocol = JamGenParse(JamGenTokenize(source));

        std::string header;
        std::string cpp;
        JamGenEmit(fileName, classPrefix, protocol, header, cpp);

        WriteFile(outputDir + "/" + fileName + ".h",   header);
        WriteFile(outputDir + "/" + fileName + ".cpp", cpp);

        std::cout << "Generated " << fileName << " protocol files in " << outputDir << "\n";
        return 0;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
