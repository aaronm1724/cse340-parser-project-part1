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
#include <string>

enum IDType { POLY_TYPE, INPUT_TYPE };

struct Symbol {
  IDType type;
  bool used_in_expr = false;
  int line_declared = -1;

  Symbol() = default;

  Symbol(IDType type, bool used_in_expr,  int line_declared)
    : type(type), used_in_expr(used_in_expr), line_declared(line_declared) {}
};

class Parser {
  public:
    void parse_program();

  private:
    LexicalAnalyzer lexer;
    void syntax_error();
    Token expect(TokenType expected_type);

    void parse_tasks_section();
    void parse_num_list();
    void parse_poly_section();
    void parse_poly_decl_list();
    void parse_poly_decl();
    void parse_poly_header();
    void parse_poly_name();
    void parse_id_list();
    void parse_poly_body();
    void parse_term_list();
    void parse_term();
    void parse_monomial_list();
    void parse_monomial();
    void parse_primary();
    void parse_exponent();
    void parse_add_operator();
    void parse_coefficient();
    void parse_execute_section();
    void parse_statement_list();
    void parse_statement();
    void parse_input_statement();
    void parse_output_statement();
    void parse_assign_statement();
    void parse_poly_evaluation();
    void parse_argument_list();
    void parse_argument();
    void parse_inputs_section();

    std::map<std::string, Symbol> symbol_table;
    std::string current_assignment_lhs;
};

#endif

