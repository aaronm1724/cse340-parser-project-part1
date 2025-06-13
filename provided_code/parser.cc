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

using namespace std;

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

// This function is simply to illustrate the GetToken() function
// you will not need it for your project and you can delete it
// the function also illustrates the use of peek()
void Parser::parse_program()
{
    parse_tasks_section();
    parse_poly_section();
    // parse_execute_section();
    //parse_inputs_section();
    expect(END_OF_FILE);
}

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

void Parser::parse_poly_section() {
    expect(POLY);
    parse_poly_decl_list();
}

void Parser::parse_poly_decl_list() {
    parse_poly_decl();
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        parse_poly_decl_list();
    } else if (t.token_type != EXECUTE && t.token_type != END_OF_FILE) {
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
    Token t = lexer.peek(1);
    if (t.token_type == LPAREN) {
        expect(LPAREN);
        parse_id_list();
        expect(RPAREN);
    }
}

void Parser::parse_id_list() {
    expect(ID);
    Token t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        parse_id_list();
    }
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
    Token t = lexer.peek(1);
    if (t.token_type == ID || t.token_type == LPAREN) {
        parse_monomial();
        parse_monomial_list();
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
        expect(ID);
    } else if (t.token_type == LPAREN) {
        expect(LPAREN);
        parse_term_list();
        expect(RPAREN);
    } else {
        syntax_error();
    }
}


void Parser::parse_execute_section() {
    expect(EXECUTE);

    parse_statement_list();
}

void Parser::parse_statement_list() {

}

void Parser::parse_statement() {
}

void Parser::parse_inputs_section() {
    cout << "[DEBUG] Entered parse_inputs_section()\n";
    expect(INPUTS);

    Token t = lexer.peek(1);
    while (t.token_type == NUM) {
        expect(NUM);
        t = lexer.peek(1);
    }
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
