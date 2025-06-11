/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"
#include <map>

enum IDType { POLY_TYPE, INPUT_TYPE };

struct Symbol {
  IDType type;
  bool used_in_expr = false;
  bool initialized = false;

  Symbol(IDType type, bool used_in_expr, bool initialized)
    : type(type), used_in_expr(used_in_expr), initialized(initialized) {}
};

class Parser {
  public:
    void parse_program();

  private:
    LexicalAnalyzer lexer;
    void syntax_error();
    Token expect(TokenType expected_type);

    void parse_tasks_section();
    void parse_poly_section();
    void parse_execute_section();
    void parse_statement_list();
    void parse_statement();
    void parse_expr();
    void parse_term();
    void parse_factor();
    void parse_inputs_section();

    std::map<std::string, Symbol> symbol_table;
};

#endif

