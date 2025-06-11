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
    parse_inputs_section();
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
        Token id_token = expect(ID);
        std::string id_name = id_token.lexeme;

        if (symbol_table.find(id_name) != symbol_table.end()) {
            cout << "ERROR: Redeclaration of identifier " << id_name << "\n";
            exit(1);
        }

        symbol_table[id_name] = {POLY_TYPE, false, true};

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
    //cout << "[DEBUG] Entering parse_statement(), peek = " << lexer.peek(1).lexeme << "\n";
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        //cout << "[DEBUG] In parse_statement(), next token: " << lexer.peek(1).lexeme << "\n";
        Token lhs = expect(ID);
        std::string id = lhs.lexeme;
        if (symbol_table.find(id) == symbol_table.end()) {
            cout << "ERROR: Undeclared identifier " << id << "\n";
            exit(1);
        }
        if (symbol_table[id].type != POLY_TYPE) {
            cout << "ERROR: Cannot assign to non-polynomial identifier " << id << "\n";
            exit(1);
        }
        expect(EQUAL);
        parse_expr();
        expect(SEMICOLON);
    } else if (t.token_type == INPUT) {
        cout << "[DEBUG] Parsing INPUT statement\n";
        expect(INPUT);
        Token id_token = expect(ID);
        std::string id = id_token.lexeme;
        if (symbol_table.find(id) != symbol_table.end()) {
            cout << "ERROR: Redeclaration of identifier " << id << "\n";
            exit(1);
        }
        symbol_table[id] = {INPUT_TYPE, false, false};
        expect(SEMICOLON);
    } else if (t.token_type == OUTPUT) {
        //cout << "[DEBUG] Parsing OUTPUT statement\n";
        expect(OUTPUT);
        Token id_token = expect(ID);
        std::string id = id_token.lexeme;
        if (symbol_table.find(id) == symbol_table.end()) {
            cout << "ERROR: Undeclared identifier " << id << "\n";
            exit(1);
        }
        expect(SEMICOLON);
    } else {
        syntax_error();
    }
}

void Parser::parse_expr() {
    //cout << "[DEBUG] Entered parse_expr()\n";
    parse_term();

    Token t = lexer.peek(1);
    while (t.token_type == PLUS || t.token_type == MINUS) {
        lexer.GetToken();
        parse_term();
        t = lexer.peek(1);
    }
}

void Parser::parse_term() {
    //cout << "[DEBUG] Entered parse_term()\n";
    parse_factor();
    
    Token t = lexer.peek(1);
    if (t.token_type == POWER) {
        lexer.GetToken();
        expect(NUM);
    }
}

void Parser::parse_factor() {
    //cout << "[DEBUG] Entered parse_factor()\n";
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        Token id_token = expect(ID);
        std::string id = id_token.lexeme;
        if (symbol_table.find(id) == symbol_table.end()) {
            cout << "ERROR: Undeclared identifier " << id << "\n";
            exit(1);
        }
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

void Parser::parse_inputs_section() {
    cout << "[DEBUG] Entered parse_inputs_section()\n";
    expect(INPUTS);

    cout << "[DEBUG] Symbol Table at start of parse_inputs_section():\n";
    for (const auto& pair : symbol_table) {
        cout << "  Name: " << pair.first
        << " | Type: " << (pair.second.type == INPUT_TYPE ? "INPUT" : "POLY")
        << " | Initialized: " << (pair.second.initialized ? "true" : "false") << "\n";
    }
    Token t = lexer.peek(1);
    while (t.token_type == NUM) {
        Token num = expect(NUM);
        bool matched = false;

        for (auto &pair : symbol_table) {
            Symbol &s = pair.second;
            if (s.type == INPUT_TYPE && !s.initialized) {
                s.initialized = true;
                matched = true;
                break;
            }
        }
        if (!matched) {
            cout << "ERROR: Too many input values\n";
            exit(1);
        }
        t = lexer.peek(1);
    }

    for (const auto &pair : symbol_table) {
        const Symbol &s = pair.second;
        if (s.type == INPUT_TYPE && !s.initialized) {
            cout << "ERROR: Uninitialized input variable\n";
            exit(1);
        }
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
