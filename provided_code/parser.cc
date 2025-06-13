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

using namespace std;

// ====== Utility Functions ======
void Parser::syntax_error()
{
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
    expect(NUM);
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

    if (!duplicates.empty()) {
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

void Parser::parse_poly_body() {
    parse_term_list();
}

void Parser::parse_term_list() {
    parse_term();
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS) {
        parse_add_operator();
        parse_term_list();
    }
}

void Parser::parse_add_operator() {
    Token t = lexer.peek(1);
    if (t.token_type == PLUS) {
        expect(PLUS);
    } else if (t.token_type == MINUS) {
        expect(MINUS);
    } else {
        syntax_error();
    }
}

void Parser::parse_term() {
    Token t = lexer.peek(1);
    if (t.token_type == NUM) {
        parse_coefficient();
        t = lexer.peek(1);
        if (t.token_type == ID || t.token_type == LPAREN) {
            parse_monomial_list();
        }
    } else if (t.token_type == ID || t.token_type == LPAREN) {
        parse_monomial_list();
    } else {
        syntax_error();
    }
}

void Parser::parse_monomial_list() {
    while (true) {
        Token t = lexer.peek(1);
        if (t.token_type == ID || t.token_type == LPAREN) {
            parse_monomial();
            t = lexer.peek(1);
        } else {
            break;
        }
    }
}

void Parser::parse_monomial() {
    Token t = lexer.peek(1);
    if (t.token_type == ID || t.token_type == LPAREN) {
        parse_primary();
        t = lexer.peek(1);
        if (t.token_type == POWER) {
            parse_exponent();
        }
    } else {
        syntax_error();
    }
}

void Parser::parse_coefficient() {
    expect(NUM);
}

void Parser::parse_exponent() {
    expect(POWER);
    expect(NUM);
}

void Parser::parse_primary() {
    Token t = lexer.peek(1);
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
    } else if (t.token_type == LPAREN) {
        expect(LPAREN);
        parse_term_list();
        expect(RPAREN);
    } else {
        syntax_error();
    }
}

// ====== EXECUTE Section ======
void Parser::parse_execute_section() {
    expect(EXECUTE);
    parse_statement_list();
}

void Parser::parse_statement_list() {
    parse_statement();
    Token t = lexer.peek(1);
    if (t.token_type == INPUT || t.token_type == OUTPUT || t.token_type == ID) {
        parse_statement_list();
    }
}

void Parser::parse_statement() {
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        parse_assign_statement();
    } else if (t.token_type == INPUT) {
        parse_input_statement();
    } else if (t.token_type == OUTPUT) {
        parse_output_statement();
    } else {
        syntax_error();
    }
}

void Parser::parse_input_statement() {
    expect(INPUT);
    expect(ID);
    expect(SEMICOLON);
}

void Parser::parse_output_statement() {
    expect(OUTPUT);
    expect(ID);
    expect(SEMICOLON);
}

void Parser::parse_assign_statement() {
    expect(ID);
    expect(EQUAL);
    parse_poly_evaluation();
    expect(SEMICOLON);
}

void Parser::parse_poly_evaluation() {
    expect(ID);
    expect(LPAREN);
    parse_argument_list();
    expect(RPAREN);
}

void Parser::parse_argument_list() {
    parse_argument();
    Token t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        parse_argument_list();
    }
}

void Parser::parse_argument() {
    Token t = lexer.peek(1);
    if (t.token_type == NUM) {
        expect(NUM);
    } else if (t.token_type == ID) {
        Token t2 = lexer.peek(2);
        if (t2.token_type == LPAREN) {
            parse_poly_evaluation();
        } else {
            expect(ID);
        }
    } else {
        syntax_error();
    }
}

// ====== INPUTS Section ======
void Parser::parse_inputs_section() {
    expect(INPUTS);
    parse_num_list();
}

int main()
{
    // note: the parser class has a lexer object instantiated in it. You should not be declaring
    // a separate lexer object. You can access the lexer object in the parser functions as shown in the
    // example method Parser::ConsumeAllInput
    // If you declare another lexer object, lexical analysis will not work correctly
    Parser parser;

    parser.parse_program();
}
