/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"

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
};

#endif

