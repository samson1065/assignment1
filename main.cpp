// bitf22m047
//Salmon Samson
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <set>
#include <cctype>
#include <cstdlib>

using namespace std;
// Token Types
enum TokenType {
    TOKEN_NUMBER,
    TOKEN_VARIABLE,
    TOKEN_OPERATOR,
    TOKEN_OPEN_BRACKET,
    TOKEN_CLOSE_BRACKET
};

// Token Structure
struct Token {
    TokenType type;
    string value;

    Token(TokenType t, const string& v) : type(t), value(v) {}
};

// Helper: Check if a character is an operator
bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

// Helper: Check if a character is an opening bracket
bool isOpenBracket(char c) {
    return c == '(' || c == '[' || c == '{';
}

// Helper: Check if a character is a closing bracket
bool isCloseBracket(char c) {
    return c == ')' || c == ']' || c == '}';
}

// Helper: Get matching opening bracket for a closing bracket
char matchingOpen(char close) {
    if (close == ')') return '(';
    if (close == ']') return '[';
    if (close == '}') return '{';
    return '\0';
}
// Helper: Get operator precedence
int precedence(const string& op) {
    if (op == "*" || op == "/") return 2;
    if (op == "+" || op == "-") return 1;
    return 0;
}
// STAGE 1: Tokenization
vector<Token> tokenize(const string& expr) {
    vector<Token> tokens;
    int i = 0;
    int len = expr.length();

    while (i < len) {
        // Skip whitespace
        if (isspace(expr[i])) {
            i++;
            continue;
        }

        // Number: sequence of digits
        if (isdigit(expr[i])) {
            string num = "";
            while (i < len && isdigit(expr[i])) {
                num += expr[i];
                i++;
            }
            tokens.push_back(Token(TOKEN_NUMBER, num));
            continue;
        }

        if (isalpha(expr[i]) || expr[i] == '_') {
            string var = "";
            while (i < len && (isalnum(expr[i]) || expr[i] == '_')) {
                var += expr[i];
                i++;
            }
            tokens.push_back(Token(TOKEN_VARIABLE, var));
            continue;
        }

        // Operator
        if (isOperator(expr[i])) {
            tokens.push_back(Token(TOKEN_OPERATOR, string(1, expr[i])));
            i++;
            continue;
        }

        // Opening bracket
        if (isOpenBracket(expr[i])) {
            tokens.push_back(Token(TOKEN_OPEN_BRACKET, string(1, expr[i])));
            i++;
            continue;
        }

        // Closing bracket
        if (isCloseBracket(expr[i])) {
            tokens.push_back(Token(TOKEN_CLOSE_BRACKET, string(1, expr[i])));
            i++;
            continue;
        }

        // Invalid character
        cerr << "Syntax Error: Invalid character '" << expr[i] << "' in expression." << endl;
        exit(1);
    }

    return tokens;
}
// STAGE 1.5: Syntax Validation
void validateSyntax(const vector<Token>& tokens) {
    if (tokens.empty()) {
        cerr << "Syntax Error: Empty expression." << endl;
        exit(1);
    }

    // Check bracket matching
    stack<char> bracketStack;
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].type == TOKEN_OPEN_BRACKET) {
            bracketStack.push(tokens[i].value[0]);
        } else if (tokens[i].type == TOKEN_CLOSE_BRACKET) {
            if (bracketStack.empty()) {
                cerr << "Syntax Error: Unmatched closing bracket '" << tokens[i].value << "'." << endl;
                exit(1);
            }
            char expected = matchingOpen(tokens[i].value[0]);
            if (bracketStack.top() != expected) {
                cerr << "Syntax Error: Mismatched brackets. Expected closing for '"
                     << bracketStack.top() << "' but found '" << tokens[i].value << "'." << endl;
                exit(1);
            }
            bracketStack.pop();
        }
    }
    if (!bracketStack.empty()) {
        cerr << "Syntax Error: Unmatched opening bracket '" << bracketStack.top() << "'." << endl;
        exit(1);
    }

    // Check for valid token sequence
    for (size_t i = 0; i < tokens.size(); i++) {
        TokenType curr = tokens[i].type;

        // First token cannot be an operator
        if (i == 0 && curr == TOKEN_OPERATOR) {
            cerr << "Syntax Error: Expression cannot start with operator '" << tokens[i].value << "'." << endl;
            exit(1);
        }

        // Last token cannot be an operator
        if (i == tokens.size() - 1 && curr == TOKEN_OPERATOR) {
            cerr << "Syntax Error: Expression cannot end with operator '" << tokens[i].value << "'." << endl;
            exit(1);
        }

        // Check for two consecutive operators
        if (i > 0 && curr == TOKEN_OPERATOR && tokens[i - 1].type == TOKEN_OPERATOR) {
            cerr << "Syntax Error: Two consecutive operators '" << tokens[i - 1].value
                 << "' and '" << tokens[i].value << "'." << endl;
            exit(1);
        }

        // Check for two consecutive operands (number/variable) without operator
        if (i > 0) {
            TokenType prev = tokens[i - 1].type;
            bool prevIsOperand = (prev == TOKEN_NUMBER || prev == TOKEN_VARIABLE);
            bool currIsOperand = (curr == TOKEN_NUMBER || curr == TOKEN_VARIABLE);
            if (prevIsOperand && currIsOperand) {
                cerr << "Syntax Error: Missing operator between '" << tokens[i - 1].value
                     << "' and '" << tokens[i].value << "'." << endl;
                exit(1);
            }
        }

        // Opening bracket followed by operator (except after another open bracket)
        if (i > 0 && curr == TOKEN_OPERATOR && tokens[i - 1].type == TOKEN_OPEN_BRACKET) {
            cerr << "Syntax Error: Operator '" << tokens[i].value << "' cannot follow opening bracket." << endl;
            exit(1);
        }

        // Operator followed by closing bracket
        if (i > 0 && curr == TOKEN_CLOSE_BRACKET && tokens[i - 1].type == TOKEN_OPERATOR) {
            cerr << "Syntax Error: Closing bracket cannot follow operator '" << tokens[i - 1].value << "'." << endl;
            exit(1);
        }

        // Empty brackets
        if (i > 0 && curr == TOKEN_CLOSE_BRACKET && tokens[i - 1].type == TOKEN_OPEN_BRACKET) {
            cerr << "Syntax Error: Empty brackets detected." << endl;
            exit(1);
        }
    }
}

// STAGE 2: Infix to Postfix Conversion (Shunting-Yard)
vector<Token> infixToPostfix(const vector<Token>& tokens) {
    vector<Token> output;
    stack<Token> opStack;

    for (size_t i = 0; i < tokens.size(); i++) {
        const Token& tok = tokens[i];

        switch (tok.type) {
            case TOKEN_NUMBER:
            case TOKEN_VARIABLE:
                // Operands go directly to output
                output.push_back(tok);
                break;

            case TOKEN_OPERATOR:
                // Pop operators with greater or equal precedence
                while (!opStack.empty() &&
                       opStack.top().type == TOKEN_OPERATOR &&
                       precedence(opStack.top().value) >= precedence(tok.value)) {
                    output.push_back(opStack.top());
                    opStack.pop();
                }
                opStack.push(tok);
                break;

            case TOKEN_OPEN_BRACKET:
                opStack.push(tok);
                break;

            case TOKEN_CLOSE_BRACKET:
                // Pop until matching opening bracket
                while (!opStack.empty() && opStack.top().type != TOKEN_OPEN_BRACKET) {
                    output.push_back(opStack.top());
                    opStack.pop();
                }
                // Pop the opening bracket (don't add to output)
                if (!opStack.empty()) {
                    opStack.pop();
                }
                break;
        }
    }

    // Pop remaining operators
    while (!opStack.empty()) {
        output.push_back(opStack.top());
        opStack.pop();
    }

    return output;
}

// STAGE 3: Collect Variables and Prompt for Values
map<string, int> collectVariables(const vector<Token>& postfix) {
    // Use a vector to preserve order of first appearance
    vector<string> varOrder;
    set<string> seen;

    for (size_t i = 0; i < postfix.size(); i++) {
        if (postfix[i].type == TOKEN_VARIABLE) {
            if (seen.find(postfix[i].value) == seen.end()) {
                seen.insert(postfix[i].value);
                varOrder.push_back(postfix[i].value);
            }
        }
    }

    map<string, int> varValues;
    for (size_t i = 0; i < varOrder.size(); i++) {
        cerr << "Enter value for " << varOrder[i] << ": ";
        int val;
        if (!(cin >> val)) {
            cerr << "Runtime Error: Invalid input for variable '" << varOrder[i] << "'." << endl;
            exit(2);
        }
        varValues[varOrder[i]] = val;
    }

    return varValues;
}

// STAGE 4: Postfix Evaluation
int evaluatePostfix(const vector<Token>& postfix, const map<string, int>& varValues) {
    stack<int> evalStack;

    for (size_t i = 0; i < postfix.size(); i++) {
        const Token& tok = postfix[i];

        if (tok.type == TOKEN_NUMBER) {
            evalStack.push(atoi(tok.value.c_str()));
        } else if (tok.type == TOKEN_VARIABLE) {
            map<string, int>::const_iterator it = varValues.find(tok.value);
            if (it != varValues.end()) {
                evalStack.push(it->second);
            } else {
                cerr << "Logical Error: Variable '" << tok.value << "' has no assigned value." << endl;
                exit(3);
            }
        } else if (tok.type == TOKEN_OPERATOR) {
            if (evalStack.size() < 2) {
                cerr << "Logical Error: Insufficient operands for operator '" << tok.value << "'." << endl;
                exit(3);
            }

            int right = evalStack.top(); evalStack.pop();
            int left  = evalStack.top(); evalStack.pop();
            int result = 0;

            if (tok.value == "+") {
                result = left + right;
            } else if (tok.value == "-") {
                result = left - right;
            } else if (tok.value == "*") {
                result = left * right;
            } else if (tok.value == "/") {
                if (right == 0) {
                    cerr << "Runtime Error: Division by zero." << endl;
                    exit(2);
                }
                result = left / right;
            }

            evalStack.push(result);
        }
    }

    if (evalStack.size() != 1) {
        cerr << "Logical Error: Invalid expression - too many operands." << endl;
        exit(3);
    }

    return evalStack.top();
}

// Helper: Print postfix expression
string postfixToString(const vector<Token>& postfix) {
    string result = "";
    for (size_t i = 0; i < postfix.size(); i++) {
        if (i > 0) result += " ";
        result += postfix[i].value;
    }
    return result;
}
// MAIN
int main() {
    // Read the infix expression
    string expression;
    getline(cin, expression);

    // Stage 1: Tokenize
    vector<Token> tokens = tokenize(expression);

    // Stage 1.5: Validate syntax
    validateSyntax(tokens);

    // Stage 2: Convert to postfix
    vector<Token> postfix = infixToPostfix(tokens);

    // Print postfix expression to stdout
    cout << postfixToString(postfix) << endl;

    // Stage 3: Collect variable values (prompts go to stderr)
    map<string, int> varValues = collectVariables(postfix);

    // Stage 4: Evaluate postfix expression
    int result = evaluatePostfix(postfix, varValues);

    // Print result to stdout
    cout << result << endl;

    return 0;
}
