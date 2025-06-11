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
    parse_execute_section();
    expect(END_OF_FILE);
}

void Parser::parse_tasks_section() {
    expect(TASKS);

    Token t = lexer.peek(1);
    while (t.token_type == NUM) {
        lexer.GetToken();
        t = lexer.peek(1);
    }
}

void Parser::parse_poly_section() {
    expect(POLY);

    Token t = lexer.peek(1);
    while (t.token_type == ID) {
        expect(ID);
        expect(EQUAL);

        Token m = lexer.GetToken();
        while (m.token_type != SEMICOLON) {
            if (m.token_type == END_OF_FILE) syntax_error();
            m = lexer.GetToken();
        }

        t = lexer.peek(1);
    }
}

void Parser::parse_execute_section() {
    expect(EXECUTE);

    parse_statement_list();
}

void Parser::parse_statement_list() {
    parse_statement();

    Token t = lexer.peek(1);
    if (t.token_type == INPUT || t.token_type == OUTPUT || t.token_type ==ID) {
        parse_statement_list();
    }
}

void Parser::parse_statement() {
    cout << "[DEBUG] Entering parse_statement(), peek = " << lexer.peek(1).lexeme << "\n";
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        cout << "[DEBUG] In parse_statement(), next token: " << lexer.peek(1).lexeme << "\n";
        expect(ID);
        expect(EQUAL);
        parse_expr();
        expect(SEMICOLON);
    } else if (t.token_type == INPUT) {
        cout << "[DEBUG] Parsing INPUT statement\n";
        expect(INPUT);
        expect(ID);
        expect(SEMICOLON);
    } else if (t.token_type == OUTPUT) {
        cout << "[DEBUG] Parsing OUTPUT statement\n";
        expect(OUTPUT);
        expect(ID);
        expect(SEMICOLON);
    } else {
        syntax_error();
    }
}

void Parser::parse_expr() {
    cout << "[DEBUG] Entered parse_expr()\n";
    parse_term();

    Token t = lexer.peek(1);
    while (t.token_type == PLUS || t.token_type == MINUS) {
        lexer.GetToken();
        parse_term();
        t = lexer.peek(1);
    }
}

void Parser::parse_term() {
    cout << "[DEBUG] Entered parse_term()\n";
    parse_factor();
    
    Token t = lexer.peek(1);
    if (t.token_type == POWER) {
        lexer.GetToken();
        expect(NUM);
    }
}

void Parser::parse_factor() {
    cout << "[DEBUG] Entered parse_factor()\n";
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        expect(ID);
    } else if (t.token_type == NUM) {
        expect(NUM);
    } else if (t.token_type == LPAREN) {
        expect(LPAREN);
        parse_expr();
        expect(RPAREN);
    } else {
        syntax_error();
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
