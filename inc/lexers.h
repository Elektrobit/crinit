/**
 * @file lexers.h
 * @brief Header related to lexers built with re2c, used in the config/envset parser.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __LEXERS_H__
#define __LEXERS_H__

/**
 * Enum data type for token types return by the lexers.
 */
typedef enum ebcl_TokenType_t {
    EBCL_TK_ERR = -1,  ///< Lexer error.
    EBCL_TK_END = 0,   ///< End-of-string encountered.
    EBCL_TK_ENVKEY,    ///< Environment key encountered (EBCL_envVarOuterLex())
    EBCL_TK_ENVVAL,    ///< Environment value encountered (EBCL_envVarOuterLex())
    EBCL_TK_WSPC,      ///< Whitespace  encountered (EBCL_envVarOuterLex())
    EBCL_TK_VAR,       ///< Variable reference encountered (EBCL_envVarInnerLex())
    EBCL_TK_ESC,       ///< Regular escape sequence encountered (EBCL_envVarInnerLex())
    EBCL_TK_ESCX,      ///< Hexadecimal escape sequence encountered (EBCL_envVarInnerLex())
    EBCL_TK_CPY        ///< Single character to copy encountered (EBCL_envVarInnerLex())
} ebcl_TokenType_t;

/**
 * Lexer/tokenizer for parsing an ENV_SET directive on the upper level.
 *
 * When repeatedly fed a string of the form `ENV_VAR_NAME "env var content"`, this function will advance the given
 * pointer over each token and tokenize that string as `<EBCL_TK_ENVKEY EBCL_TK_ENVVAR>`.
 *
 * In general, the function will tokenive any free-standing alphanumeric text as an env key and any quoted characters
 * as an env value. Sanity-checking to make sure we get a single key followed by a single value is left to the upper
 * layer.
 *
 * The function will return an EBCL_TK_ERR if a free-standing (unquoted) non-alphanumeric character is encountered or
 * if a key would begin with a number.
 *
 * Environment keys are matched fully while the quotes of environment values are consumed but left out of the matched
 * string.
 *
 * Whitespaces are
 *
 * @param s       The string to tokenize, will be advanced over one token per call.
 * @param mbegin  Begin of a token match.
 * @param mend    End of a token match.
 *
 * @return EBCL_TK_ERR on any error, EBCL_TK_END on the end of the string, EBCL_TK_ENVKEY on an env key match,
 *         EBCL_TK_ENVVAL on an env value match.
 */
ebcl_TokenType_t EBCL_envVarOuterLex(const char **s, const char **mbegin, const char **mend);
/**
 * Lexer/tokenizer for parsing the value part of an ENV_SET directive.
 *
 * The lexer supports simple characters to copy, escape sequences (standard and hexadecimal), and variables of the form
 * `${VAR_NAME}`.
 *
 * In general, the match will contain the full token with two exceptions:
 *     - EBCL_TK_VAR will consume `${VAR_NAME}` but match only `VAR_NAME`.
 *     - EBCL_TK_ESCX will consume for example `\x5e` but match only `5e`.
 *
 * The only parser error to be encountered is a single slash followed by the end-of-string, resulting in an illegal
 * escape sequences.
 *
 * @param s       The string to tokenize, will be advanced over one token per call.
 * @param mbegin  Begin of a token match.
 * @param mend    End of a token match.
 *
 * @return  An ebcl_TokenType_t as detailed above.
 */
ebcl_TokenType_t EBCL_envVarInnerLex(const char **s, const char **mbegin, const char **mend);

#endif /* __LEXERS_H__ */
