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

class Parser {
  public:
    void parse_program();

  private:
    LexicalAnalyzer lexer;
    void syntax_error();
    Token expect(TokenType expected_type);

    // ====== Internal state for semantic checks ======
    std::map<std::string, std::vector<int>> poly_decl_lines;
    std::string current_poly;
    std::map<std::string, std::vector<std::string>> poly_params;
    std::vector<int> invalid_lines;

    // ====== Parser methods ======
    void parse_tasks_section();
    void parse_num_list();
    void parse_poly_section();
    void parse_poly_decl_list();
    void parse_poly_decl();
    void parse_poly_header();
    void parse_poly_name();
    std::vector<std::string> parse_id_list();
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

};

#endif

