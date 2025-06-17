/*
 * Copyright (C) Rida Bazzi, 2020
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */
#include <iostream>
#include <cstdlib>
#include "parser.h"
#include <algorithm>
#include <unordered_set>

using namespace std;

// ====== Utility Functions ======
void Parser::syntax_error()
{
    if (!task_numbers.count(1)) return;
    cout << "SYNTAX ERROR !!!!!&%!!\n";
    exit(1);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    // std::cout << "[expect] got token: " << t.lexeme << " (type: " << t.token_type << "), expected: " << expected_type << std::endl;
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

// Parsing
// ====== Top-Level Program ======
void Parser::parse_program()
{
    parse_tasks_section();
    parse_poly_section();
    parse_execute_section();
    parse_inputs_section();
    expect(END_OF_FILE);
}

// ====== TASKS Section ======
void Parser::parse_tasks_section() {
    expect(TASKS);
    parse_num_list();
}

void Parser::parse_num_list() {
    Token num_token = expect(NUM);
    task_numbers.insert(std::stoi(num_token.lexeme));
    if (in_inputs_section) {
        input_values.push_back(std::stoi(num_token.lexeme));
    }

    Token t = lexer.peek(1);
    if (t.token_type == NUM) {
        parse_num_list();
    }
}

// ====== POLY Section ======
void Parser::parse_poly_section() {
    expect(POLY);
    parse_poly_decl_list();

    std::vector<int> duplicates;
    for (const auto& pair : poly_decl_lines) {
        const std::vector<int>& lines = pair.second;
        if (lines.size() > 1) {
            duplicates.insert(duplicates.end(), lines.begin() + 1, lines.end());
        }
    }

    if (task_numbers.count(1)) {
        if (!duplicates.empty()) {
            std::sort(duplicates.begin(), duplicates.end(), std::less<int>());
            std::cout << "Semantic Error Code 1:";
            for (int line : duplicates) {
                std::cout << " " << line;
            }
            std::cout << std::endl;
            exit(0);
        }

        if (!invalid_lines.empty()) {
            std::sort(invalid_lines.begin(), invalid_lines.end());
            std::cout << "Semantic Error Code 2:";
            for (int line : invalid_lines) {
                std::cout << " " << line;
            }
            std::cout << std::endl;
            exit(0);
        }
    }
}

void Parser::parse_poly_decl_list() {
    parse_poly_decl();
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        parse_poly_decl_list();
    } else if (t.token_type != EXECUTE) {
        syntax_error();
    }
}

void Parser::parse_poly_decl() {
    parse_poly_header();
    expect(EQUAL);
    parse_poly_body();
    expect(SEMICOLON);
}

void Parser::parse_poly_header() {
    Token id_token = expect(ID);
    std::string name = id_token.lexeme;
    int line = id_token.line_no;
    poly_decl_lines[name].push_back(line);

    current_poly = name;

    Token t = lexer.peek(1);
    if (t.token_type == LPAREN) {
        expect(LPAREN);
        poly_params[current_poly] = parse_id_list();
        expect(RPAREN);
    } else {
        poly_params[current_poly] = {"x"};
    }
}

std::vector<std::string> Parser::parse_id_list() {
    std::vector<std::string> params;
    Token id_token = expect(ID);
    params.push_back(id_token.lexeme);

    while (lexer.peek(1).token_type == COMMA) {
        expect(COMMA);
        Token next_id = expect(ID);
        params.push_back(next_id.lexeme);
    }
    return params;
}

poly_body_t* Parser::parse_poly_body() {
    term_list_t* terms = parse_term_list();
    poly_body_t* body = new poly_body_t;
    body->terms = terms;
    poly_bodies[current_poly] = body;
    return body;
}

term_list_t* Parser::parse_term_list() {
    OperatorType leading_op = OP_NONE;
    Token t1 = lexer.peek(1);
    if (t1.token_type == PLUS || t1.token_type == MINUS) {
        leading_op = parse_add_operator();
    }
    term_t* first_term = parse_term();
    term_list_t* node = new term_list_t;
    node->term = first_term;
    node->op = leading_op;
    Token t2 = lexer.peek(1);
    if (t2.token_type == PLUS || t2.token_type == MINUS) {
        node->next = parse_term_list();
    } else {
        node->next = nullptr;
    }
    return node;
}

OperatorType Parser::parse_add_operator() {
    Token t = lexer.peek(1);
    if (t.token_type == PLUS) {
        expect(PLUS);
        return OP_PLUS;
    } else if (t.token_type == MINUS) {
        expect(MINUS);
        return OP_MINUS;
    } else {
        syntax_error();
        return OP_NONE;
    }
}

term_t* Parser::parse_term() {
    Token t = lexer.peek(1);
    term_t* term = new term_t;
    if (t.token_type == NUM) {
        term->coefficient = parse_coefficient();
        t = lexer.peek(1);
        if (t.token_type == ID || t.token_type == LPAREN) {
            term->monomial_list = parse_monomial_list();
        } else {
            term->monomial_list = {};
        }
    } else if (t.token_type == ID || t.token_type == LPAREN) {
        term->coefficient = 1;
        term->monomial_list = parse_monomial_list();
    } else {
        syntax_error();
    }
    return term;
}

std::vector<monomial_t*> Parser::parse_monomial_list() {
    std::vector<monomial_t*> monomial_list;
    while (true) {
        Token t = lexer.peek(1);
        if (t.token_type == ID || t.token_type == LPAREN) {
            monomial_t* monomial = parse_monomial();
            monomial_list.push_back(monomial);
            t = lexer.peek(1);
        } else {
            break;
        }
    }
    return monomial_list;
}

monomial_t* Parser::parse_monomial() {
    Token t = lexer.peek(1);
    monomial_t* monomial = new monomial_t;
    if (t.token_type == ID || t.token_type == LPAREN) {
        monomial->primary = parse_primary();
        t = lexer.peek(1);
        if (t.token_type == POWER) {
            monomial->exponent = parse_exponent();
        } else {
            monomial->exponent = 1;
        }
    } else {
        syntax_error();
    }

    return monomial;
}

int Parser::parse_coefficient() {
    Token t = expect(NUM);
    return std::stoi(t.lexeme);
}

int Parser::parse_exponent() {
    expect(POWER);
    Token t = expect(NUM);
    return std::stoi(t.lexeme);
}

primary_t* Parser::parse_primary() {
    Token t = lexer.peek(1);
    primary_t* primary = new primary_t;
    if (t.token_type == ID) {
        Token id_token = expect(ID);
        std::string var_name = id_token.lexeme;
        if (poly_params.find(current_poly) != poly_params.end()) {
            std::vector<std::string> allowed_vars = poly_params[current_poly];
            if (std::find(allowed_vars.begin(), allowed_vars.end(), var_name) == allowed_vars.end()) {
                invalid_lines.push_back(id_token.line_no);
            }
        } else {
            if (var_name != "x") {
                invalid_lines.push_back(id_token.line_no);
            }
        }

        primary->kind = VAR;
        primary->var_name = var_name;
        primary->term_list = nullptr;
    } else if (t.token_type == LPAREN) {
        expect(LPAREN);
        term_list_t* term_list =parse_term_list();
        expect(RPAREN);
        primary->kind = TERM_LIST;
        primary->term_list = term_list;
        primary->var_name = "";
    } else {
        syntax_error();
    }

    return primary;
}

// ====== EXECUTE Section ======
void Parser::parse_execute_section() {
    expect(EXECUTE);
    stmt_list_head = parse_statement_list();
    if (stmt_list_head == nullptr) {
        std::cerr << "[fatal] no statements parsed in EXECUTE section\n";
        syntax_error();
    } 

    if (task_numbers.count(1)) {
        if (!undeclared_eval_lines.empty()) {
            std::sort(undeclared_eval_lines.begin(), undeclared_eval_lines.end());
            std::cout << "Semantic Error Code 3:";
            for (int line : undeclared_eval_lines) {
                std::cout << " " << line;
            }
            std::cout << std::endl;
            exit(0);
        }

        if (!wrong_arity_lines.empty()) {
            std::sort(wrong_arity_lines.begin(), wrong_arity_lines.end());
            std::cout << "Semantic Error Code 4:";
            for (int line : wrong_arity_lines) {
                std::cout << " " << line;
            }
            std::cout << std::endl;
            exit(0);
        }
    }
}

stmt_t* Parser::parse_statement_list() {
    stmt_t* first = parse_statement();
    stmt_t* current = first;
    Token t = lexer.peek(1);
    while (t.token_type == INPUT || t.token_type == OUTPUT || t.token_type == ID) {
       current->next = parse_statement();
       current = current->next;
       t = lexer.peek(1);
    }

    return first;
}

stmt_t* Parser::parse_statement() {
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        return parse_assign_statement();
    } else if (t.token_type == INPUT) {
        return parse_input_statement();
    } else if (t.token_type == OUTPUT) {
        return parse_output_statement();
    } else {
        syntax_error();
        return nullptr;
    }
}

stmt_t* Parser::parse_input_statement() {
    expect(INPUT);
    Token id_token = expect(ID);
    expect(SEMICOLON);

    std::string var_name = id_token.lexeme;
    if (location_table.find(var_name) == location_table.end()) {
        location_table[var_name] = next_available++;
    }
    initialized_vars.insert(var_name);
    input_vars_in_order.push_back(var_name);


    stmt_t* stmt = new stmt_t;
    stmt->type = STMT_INPUT;
    stmt->var = location_table[var_name];
    stmt->line_no = id_token.line_no;
    return stmt;
}

stmt_t* Parser::parse_output_statement() {
    expect(OUTPUT);
    Token id_token = expect(ID);
    expect(SEMICOLON);

    std::string var_name = id_token.lexeme;
    if (location_table.find(var_name) == location_table.end()) {
        location_table[var_name] = next_available++;
    }

    stmt_t* stmt = new stmt_t;
    stmt->type = STMT_OUTPUT;
    stmt->var = location_table[var_name];
    stmt->line_no = id_token.line_no;
    return stmt;
}

stmt_t* Parser::parse_assign_statement() {
    Token lhs_token = expect(ID);
    std::string lhs_name = lhs_token.lexeme;
    if (location_table.find(lhs_name) == location_table.end()) {
        location_table[lhs_name] = next_available++;
    }
    expect(EQUAL);
    poly_eval_t* eval = parse_poly_evaluation();
    expect(SEMICOLON);

    stmt_t* stmt = new stmt_t;
    stmt->type = STMT_ASSIGN;
    if (task_numbers.count(3)) {
        for (const std::string& arg : eval->args) {
            if (initialized_vars.find(arg) == initialized_vars.end()) {
                warning_lines_uninitialized.push_back(lhs_token.line_no);
            }
        }
    }
    initialized_vars.insert(lhs_name);
    stmt->lhs = location_table[lhs_name];
    stmt->eval = eval;
    stmt->line_no = lhs_token.line_no;
    return stmt;
}

poly_eval_t* Parser::parse_poly_evaluation() {
    Token id_token = expect(ID);
    std::string poly_name = id_token.lexeme;
    int line = id_token.line_no;
    if (poly_decl_lines.find(poly_name) == poly_decl_lines.end()) {
        undeclared_eval_lines.push_back(line);
    }
    expect(LPAREN);
    std::vector<std::string> args = parse_argument_list();
    expect(RPAREN);

    if (poly_params.find(poly_name) != poly_params.end()) {
        std::vector<std::string> expected_params = poly_params[poly_name];
        if (args.size() != expected_params.size()) {
            if (std::find(wrong_arity_lines.begin(), wrong_arity_lines.end(), line) == wrong_arity_lines.end()) {
                wrong_arity_lines.push_back(line);
            }
        }
    } else {
        if (args.size() != 1) {
            if (std::find(wrong_arity_lines.begin(), wrong_arity_lines.end(), line) == wrong_arity_lines.end()) {
                wrong_arity_lines.push_back(line);
            }
        }
    }

    poly_eval_t* eval = new poly_eval_t;
    eval->name = poly_name;
    eval->args = args;
    return eval;
}

std::vector<std::string> Parser::parse_argument_list() {
    std::vector<std::string> args;
    parse_argument(args);
    while (lexer.peek(1).token_type == COMMA) {
        expect(COMMA);
        parse_argument(args);
    }
    return args;
}

void Parser::parse_argument(std::vector<std::string>& args) {
    Token t = lexer.peek(1);
    // Only push arguments for base NUM or ID types (not nested evaluations)
    if (t.token_type == NUM) {
        Token t1 = expect(NUM);
        args.push_back(t1.lexeme);
    } else if (t.token_type == ID) {
        Token t2 = lexer.peek(2);
        if (t2.token_type == LPAREN) {
            parse_poly_evaluation();
        } else {
            Token t3 = expect(ID);
            args.push_back(t3.lexeme);
        }
    } else {
        syntax_error();
    }
}

void Parser::execute_program() {
    std::fill(memory.begin(), memory.end(), 0);
    stmt_t* current = stmt_list_head;
    input_counter = 0;

    for (size_t i = 0; i < input_vars_in_order.size(); ++i) {
        int value = (i < input_values.size()) ? input_values[i] : 0;
        int loc = location_table[input_vars_in_order[i]];
        memory[loc] = value;
    }
    while (current != nullptr) {
        switch (current->type) {
            case STMT_INPUT: {
                break;
            }
            case STMT_OUTPUT: {
                std::cout << memory[current->var] << std::endl;
                break;
            }
            case STMT_ASSIGN: {
                poly_eval_t* eval = static_cast<poly_eval_t*>(current->eval);
                std::string poly_name = eval->name;
                std::vector<std::string> args = eval->args;
                current_poly = poly_name;
                std::map<std::string, int> arg_values;
                const std::vector<std::string>& params = poly_params[poly_name];
                if (params.size() != args.size()) {
                    std::cerr << "[fatal] wrong number of arguments for poly " << poly_name << std::endl;
                    exit(1);
                }
                for (size_t i = 0; i < args.size(); ++i) {
                    const std::string& actual = args[i];
                    const std::string& expected = params[i];
                    int value;
                    if (isdigit(actual[0]) || (actual[0] == '-' && actual.length() > 1)) {
                        value = std::stoi(actual);
                    } else {
                        value = memory[location_table[actual]];
                    }
                    arg_values[params[i]] = value;
                }
                memory[current->lhs] = evaluate_poly(poly_bodies[poly_name], arg_values, location_table);
                break;
            }
        }
        current = current->next;
    }
}

int Parser::evaluate_poly(poly_body_t* body, const std::map<std::string, int>& arg_values, const std::map<std::string, int>& location_table) {
    for (const auto& entry : arg_values) {
        const std::string& param = entry.first;
        int value = entry.second;
    }

    int result = 0;
    term_list_t* current_term = body->terms;
    while (current_term != nullptr) {
        term_t* term = current_term->term;
        int term_value = evaluate_term(term, arg_values, location_table);

        if (current_term->op == OP_PLUS || current_term->op == OP_NONE) {
            result += term_value;
        } else if (current_term->op == OP_MINUS) {
            result -= term_value;
        }

        current_term = current_term->next;
    }
    return result;
}

int Parser::evaluate_term(term_t* term, const std::map<std::string, int>& arg_values, const std::map<std::string, int>& location_table) {
    int product = 1;
    for (monomial_t* monomial : term->monomial_list) {
        product *= evaluate_monomial(monomial, arg_values, location_table);
    }

    return term->coefficient * product;
}

int Parser::evaluate_monomial(monomial_t* monomial, const std::map<std::string, int>& arg_values, const std::map<std::string, int>& location_table) { 
    int base = evaluate_primary(monomial->primary, arg_values, location_table);
    int exponent = monomial->exponent;
    int result = 1;
    for (int i = 0; i < exponent; ++i) {
        result *= base;
    }
    return result;
}

int Parser::evaluate_primary(primary_t* primary, const std::map<std::string, int>& arg_values, const std::map<std::string, int>& location_table) {
    if (primary->kind == VAR) {
        if (arg_values.find(primary->var_name) != arg_values.end()) {
            return arg_values.at(primary->var_name);
        } else if (location_table.find(primary->var_name) != location_table.end()) {
            return memory[location_table.at(primary->var_name)];
        } else {
            return 0;
        }
    } else if (primary->kind == TERM_LIST) {
        poly_body_t* body = new poly_body_t;
        body->terms = primary->term_list;
        return evaluate_poly(body, arg_values, location_table);
    } else {
        exit(1);
    }
}

void Parser::check_useless_assignments() {
    std::set<std::string> used_vars;
    std::vector<stmt_t*> statements;
    stmt_t* current = stmt_list_head;
    while (current != nullptr) {
        statements.push_back(current);
        current = current->next;
    }

    // First pass: collect all variables used in OUTPUT statements
    for (int i = statements.size() - 1; i >= 0; --i) {
        stmt_t* stmt = statements[i];
        if (stmt->type == STMT_OUTPUT) {
            for (const auto& entry : location_table) {
                if (entry.second == stmt->var) {
                    used_vars.insert(entry.first);
                    break;
                }
            }
        }
    }

    // Second pass: track assignments and mark useless ones
    for (int i = statements.size() - 1; i >= 0; --i) {
        stmt_t* stmt = statements[i];
        if (stmt->type == STMT_ASSIGN) {
            std::string lhs_name;
            for (const auto& entry : location_table) {
                if (entry.second == stmt->lhs) {
                    lhs_name = entry.first;
                    break;
                }
            }

            poly_eval_t* eval = static_cast<poly_eval_t*>(stmt->eval);
            // Add all variables used in the evaluation to used_vars
            for (const std::string& arg : eval->args) {
                if (isdigit(arg[0]) || (arg[0] == '-' && arg.length() > 1)) continue;
                used_vars.insert(arg);
            }

            // Check if the assigned variable is used
            if (!lhs_name.empty() && used_vars.find(lhs_name) == used_vars.end()) {
                useless_assignments.push_back(stmt->line_no);
            } else {
                used_vars.erase(lhs_name);  // Remove it so we can detect earlier useless assignments
            }
        }
    }
    std::sort(useless_assignments.begin(), useless_assignments.end());
}

// ====== INPUTS Section ======
void Parser::parse_inputs_section() {
    expect(INPUTS);
    in_inputs_section = true;
    parse_num_list();
    in_inputs_section = false;
}

int main()
{
    Parser parser;
    parser.parse_program();

    // Check for semantic errors first
    if (!parser.wrong_arity_lines.empty()) {
        std::sort(parser.wrong_arity_lines.begin(), parser.wrong_arity_lines.end());
        std::cout << "Semantic Error Code 4:";
        for (int line : parser.wrong_arity_lines) {
            std::cout << " " << line;
        }
        std::cout << std::endl;
        return 0;
    }

    if (parser.task_numbers.count(2)) {
        parser.execute_program();
    }

    if (parser.task_numbers.count(3)) {
        if (!parser.warning_lines_uninitialized.empty()) {
            std::sort(parser.warning_lines_uninitialized.begin(), parser.warning_lines_uninitialized.end());
            std::cout << "Warning Code 1:";
            for (int line : parser.warning_lines_uninitialized) {
                std::cout << " " << line;
            }
            std::cout << std::endl;
        }
    }

    if (parser.task_numbers.count(4)) {
        parser.check_useless_assignments();
        if (!parser.useless_assignments.empty()) {
            std::sort(parser.useless_assignments.begin(), parser.useless_assignments.end());
            std::cout << "Warning Code 2:";
            for (int line : parser.useless_assignments) {
                std::cout << " " << line;
            }
            std::cout << std::endl;
        }
    }
}
