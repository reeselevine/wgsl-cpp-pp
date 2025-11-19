#ifndef WGSLPP_HPP
#define WGSLPP_HPP

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <optional>
#include <iostream>

namespace wgslpp {

//==============================================================
// Options
//==============================================================
struct Options {
    std::string include_path = ".";
};

//==============================================================
// Utility: trim
//==============================================================
static inline std::string trim(const std::string& s) {
    size_t a = 0;
    while (a < s.size() && std::isspace((unsigned char)s[a])) a++;
    size_t b = s.size();
    while (b > a && std::isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

//==============================================================
// Tokenizer for expressions in #if
//==============================================================
class ExprLexer {
public:
    enum Kind {
        END, IDENT, NUMBER,
        OP, LPAREN, RPAREN
    };

    struct Tok {
        Kind kind;
        std::string text;
    };

    explicit ExprLexer(std::string_view sv) : src(sv), pos(0) {}

    Tok next() {
        skipWS();
        if (pos >= src.size()) return { END, "" };

        char c = src[pos];

        // number
        if (std::isdigit((unsigned char)c)) {
            size_t start = pos;
            while (pos < src.size() && std::isdigit((unsigned char)src[pos]))
                pos++;
            return { NUMBER, std::string(src.substr(start, pos - start)) };
        }

        // identifier
        if (std::isalpha((unsigned char)c) || c == '_') {
            size_t start = pos;
            while (pos < src.size() &&
                   (std::isalnum((unsigned char)src[pos]) || src[pos] == '_'))
                pos++;
            return { IDENT, std::string(src.substr(start, pos - start)) };
        }

        if (c == '(') { pos++; return { LPAREN, "(" }; }
        if (c == ')') { pos++; return { RPAREN, ")" }; }

        // multi-char operators
        static const char* two_ops[] = {
            "==","!=", "<=", ">=", "&&","||", "<<",">>"
        };
        for (auto op : two_ops) {
            if (src.substr(pos, 2) == op) {
                pos += 2;
                return { OP, std::string(op) };
            }
        }

        // single-char operators
        if (std::string("+-*/%<>!").find(c) != std::string::npos) {
            pos++;
            return { OP, std::string(1, c) };
        }

        // unexpected
        pos++;
        return { END, "" };
    }

private:
    std::string_view src;
    size_t pos;

    void skipWS() {
        while (pos < src.size() && std::isspace((unsigned char)src[pos]))
            pos++;
    }
};

//==============================================================
// Expression Parser (recursive descent)
//==============================================================
class ExprParser {
public:
    ExprParser(std::string_view expr,
               const std::unordered_map<std::string,std::string>& macros)
        : lex(expr), macros(macros)
    {
        advance();
    }

    int parse() {
        return parseLogicalOr();
    }

private:
    ExprLexer lex;
    ExprLexer::Tok tok;
    const std::unordered_map<std::string,std::string>& macros;

    void advance() { tok = lex.next(); }

    bool acceptOp(const std::string& s) {
        if (tok.kind == ExprLexer::OP && tok.text == s) {
            advance();
            return true;
        }
        return false;
    }

    bool acceptKind(ExprLexer::Kind k) {
        if (tok.kind == k) {
            advance();
            return true;
        }
        return false;
    }

    int parseLogicalOr() {
        int v = parseLogicalAnd();
        while (acceptOp("||")) {
            int rhs = parseLogicalAnd();
            v = (v || rhs);
        }
        return v;
    }

    int parseLogicalAnd() {
        int v = parseEquality();
        while (acceptOp("&&")) {
            int rhs = parseEquality();
            v = (v && rhs);
        }
        return v;
    }

    int parseEquality() {
        int v = parseRelational();
        for (;;) {
            if (acceptOp("==")) {
                int rhs = parseRelational();
                v = (v == rhs);
            } else if (acceptOp("!=")) {
                int rhs = parseRelational();
                v = (v != rhs);
            } else break;
        }
        return v;
    }

    int parseRelational() {
        int v = parseShift();
        for (;;) {
            if (acceptOp("<"))  { int rhs = parseShift(); v = (v < rhs); }
            else if (acceptOp(">")) { int rhs = parseShift(); v = (v > rhs); }
            else if (acceptOp("<=")){ int rhs = parseShift(); v = (v <= rhs); }
            else if (acceptOp(">=")){ int rhs = parseShift(); v = (v >= rhs); }
            else break;
        }
        return v;
    }

    int parseShift() {
        int v = parseAdd();
        for (;;) {
            if (acceptOp("<<")) { int rhs = parseAdd(); v = (v << rhs); }
            else if (acceptOp(">>")) { int rhs = parseAdd(); v = (v >> rhs); }
            else break;
        }
        return v;
    }

    int parseAdd() {
        int v = parseMult();
        for (;;) {
            if (acceptOp("+")) { int rhs = parseMult(); v = (v + rhs); }
            else if (acceptOp("-")) { int rhs = parseMult(); v = (v - rhs); }
            else break;
        }
        return v;
    }

    int parseMult() {
        int v = parseUnary();
        for (;;) {
            if (acceptOp("*")) { int rhs = parseUnary(); v = (v * rhs); }
            else if (acceptOp("/")) { int rhs = parseUnary(); v = (rhs == 0?0:v / rhs); }
            else if (acceptOp("%")) { int rhs = parseUnary(); v = (rhs == 0?0:v % rhs); }
            else break;
        }
        return v;
    }

    int parseUnary() {
        if (acceptOp("!")) return !parseUnary();
        if (acceptOp("-")) return -parseUnary();
        if (acceptOp("+")) return +parseUnary();
        return parsePrimary();
    }

    int parsePrimary() {
        // '(' expr ')'
        if (acceptKind(ExprLexer::LPAREN)) {
            int v = parse();
            if (!acceptKind(ExprLexer::RPAREN))
                throw std::runtime_error("missing ')'");
            return v;
        }

        // number
        if (tok.kind == ExprLexer::NUMBER) {
            int v = std::stoi(tok.text);
            advance();
            return v;
        }

        // defined(identifier)
        if (tok.kind == ExprLexer::IDENT && tok.text == "defined") {
            advance();
            if (acceptKind(ExprLexer::LPAREN)) {
                if (tok.kind != ExprLexer::IDENT)
                    throw std::runtime_error("expected identifier in defined()");
                std::string name = tok.text;
                advance();
                if (!acceptKind(ExprLexer::RPAREN))
                    throw std::runtime_error("missing ) in defined()");
                return macros.count(name) ? 1 : 0;
            } else {
                // defined NAME
                if (tok.kind != ExprLexer::IDENT)
                    throw std::runtime_error("expected identifier in defined NAME");
                std::string name = tok.text;
                advance();
                return macros.count(name) ? 1 : 0;
            }
        }

        // identifier -> treat as integer, if defined use its value else 0
        if (tok.kind == ExprLexer::IDENT) {
            std::string name = tok.text;
            advance();
            auto it = macros.find(name);
            if (it == macros.end()) return 0;
            if (it->second.empty()) return 1;
            return std::stoi(it->second);
        }

        // unexpected
        return 0;
    }
};

//==============================================================
// Preprocessor
//==============================================================
class Preprocessor {
public:
    explicit Preprocessor(Options opts = {})
        : opts_(std::move(opts)) {}

    std::string run(const std::string& filename) {
        include_stack.clear();
        return processFile(filename);
    }

    std::string run_from_string(const std::string& contents)
    {
        include_stack.clear();
        return processString(contents);
    }

    // TODO: add API to separate out includes vs. other directives

private:
    Options opts_;
    std::unordered_map<std::string,std::string> macros;
    std::unordered_set<std::string> include_stack;

    struct Cond {
        bool parent_active;
        bool active;
        bool taken;
    };
    std::vector<Cond> cond;

    //----------------------------------------------------------
    // Helpers
    //----------------------------------------------------------
    std::string loadFile(const std::string& fname) {
        std::ifstream f(fname);
        if (!f.is_open())
            throw std::runtime_error("Could not open file: " + fname);
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    bool currentActive() const {
        if (cond.empty()) return true;
        return cond.back().active;
    }

    //----------------------------------------------------------
    // Process a file
    //----------------------------------------------------------
    std::string processFile(const std::string& name) {
        if (include_stack.count(name))
            throw std::runtime_error("Recursive include: " + name);

        include_stack.insert(name);
        std::string shader_code = loadFile(name);
        std::string out = processString(shader_code);
        include_stack.erase(name);
        return out;
    }

    std::string processIncludeFile(const std::string& fname) {
        std::string full_path = opts_.include_path + "/" + fname;
        return processFile(full_path);
    }

    //----------------------------------------------------------
    // Process text
    //----------------------------------------------------------
    std::string processString(const std::string& shader_code)
    {
        std::stringstream out;
        std::istringstream in(shader_code);
        std::string line;

        while (std::getline(in, line)) {
            std::string t = trim(line);

            if (!t.empty() && t[0] == '#') {
                handleDirective(t, out);
            } else {
                if (currentActive())
                    out << line << "\n";
            }
        }
        return out.str();
    }

    //----------------------------------------------------------
    // Directive handler
    //----------------------------------------------------------
    void handleDirective(const std::string& t, std::stringstream& out) {
        // split into tokens
        std::string body = t.substr(1);
        std::istringstream iss(body);
        std::string cmd;
        iss >> cmd;

        if (cmd == "include") {
            if (!currentActive()) return;
            std::string file;
            iss >> file;
            std::cout << "Including file: " << file << "\n";
            if (file.size() >= 2 && file.front()=='"' && file.back()=='"')
                file = file.substr(1, file.size()-2);
            out << processIncludeFile(file);
            return;
        }

        if (cmd == "define") {
            if (!currentActive()) return;
            std::string name;
            iss >> name;
            std::string value;
            std::getline(iss, value);
            value = trim(value);
            macros[name] = value;
            return;
        }

        if (cmd == "ifdef") {
            std::string name; iss >> name;
            bool p = currentActive();
            bool v = macros.count(name);
            cond.push_back({p, p && v, p && v});
            return;
        }

        if (cmd == "ifndef") {
            std::string name; iss >> name;
            bool p = currentActive();
            bool v = !macros.count(name);
            cond.push_back({p, p && v, p && v});
            return;
        }

        if (cmd == "if") {
            std::string expr;
            std::getline(iss, expr);
            expr = trim(expr);
            bool p = currentActive();
            bool v = false;
            if (p) {
                ExprParser ep(expr, macros);
                v = ep.parse() != 0;
            }
            cond.push_back({p, p && v, p && v});
            return;
        }

        if (cmd == "elif") {
            std::string expr;
            std::getline(iss, expr);
            expr = trim(expr);

            if (cond.empty())
                throw std::runtime_error("#elif without #if");

            Cond& c = cond.back();
            if (!c.parent_active) {
                c.active = false;
                return;
            }

            if (c.taken) {
                c.active = false;
                return;
            }

            ExprParser ep(expr, macros);
            bool v = ep.parse() != 0;
            c.active = v;
            if (v) c.taken = true;
            return;
        }

        if (cmd == "else") {
            if (cond.empty())
                throw std::runtime_error("#else without #if");

            Cond& c = cond.back();
            if (!c.parent_active) {
                c.active = false;
                return;
            }
            if (c.taken) {
                c.active = false;
            } else {
                c.active = true;
                c.taken = true;
            }
            return;
        }

        if (cmd == "endif") {
            if (cond.empty())
                throw std::runtime_error("#endif without #if");
            cond.pop_back();
            return;
        }

        // TODO: error on uknown directive
    }
};

} // namespace wgslpp

#endif // WGSLPP_HPP
