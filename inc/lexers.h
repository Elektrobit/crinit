// SPDX-License-Identifier: MIT
/**
 * @file lexers.h
 * @brief Header related to lexers built with re2c, used in the config/envset parser.
 */
#ifndef __LEXERS_H__
#define __LEXERS_H__

#include <stdbool.h>

/**
 * Enum data type for token types return by the lexers.
 */
typedef enum crinitTokenType {
    CRINIT_TK_ERR = -1,  ///< Lexer error.
    CRINIT_TK_END = 0,   ///< End-of-string encountered.
    CRINIT_TK_ENVKEY,    ///< Environment key encountered (crinitEnvVarOuterLex())
    CRINIT_TK_ENVVAL,    ///< Environment value encountered (crinitEnvVarOuterLex())
    CRINIT_TK_WSPC,      ///< Whitespace  encountered (crinitEnvVarOuterLex())
    CRINIT_TK_VAR,       ///< Variable reference encountered (crinitEnvVarInnerLex())
    CRINIT_TK_ESC,       ///< Regular escape sequence encountered (crinitEnvVarInnerLex())
    CRINIT_TK_ESCX,      ///< Hexadecimal escape sequence encountered (crinitEnvVarInnerLex())
    CRINIT_TK_CPY,       ///< Single character to copy encountered (crinitEnvVarInnerLex())
    CRINIT_TK_DQSTR,     ///< Double-quoted string encountered (crinitArgvLex())
    CRINIT_TK_UQSTR      ///< Unquoted string encountered (crinitArgvLex())
} crinitTokenType_t;

/**
 * Escape sequence map.
 *
 * crinitEscMap[c] will return the character which the escape sequence \\'c' specifies.
 *
 * Values initialized in src/lexers.re
 */
extern const char crinitEscMap[128];

/**
 * Lexer/Tokenizer for argv-like string Arrays.
 *
 * With dq==true, will respect double quotes. Otherwise it will only delimit tokens by unescaped whitespace. Escaped
 * double quotes (`\"`) are detected and handled as regular characters in both modes.
 *
 * Should be called in a loop, will consume a single token on each call.
 *
 * @param s       The string to tokenize.
 * @param mbegin  Begin of a matched token, will not include enclosing quotes if dq==true.
 * @param mend    End of a matched token, will not include enclosing quotes if dq==true.
 * @param dq      Set to true to activate handling of double quoted strings.
 *
 * @return  The token type that was just consumed. CRINIT_TK_UQSTR for an unquoted string, CRINIT_TK_DQSTR for a doubly-
 *          quoted string (only if dq==true), CRINIT_TK_WSPC for whitespace, CRINIT_TK_END for end-of-string, and
 *          CRINIT_TK_ERR if the lexer encountered an error.
 */
crinitTokenType_t crinitArgvLex(const char **s, const char **mbegin, const char **mend, bool dq);
/**
 * Lexer/Tokenizer for escape characters.
 *
 * Will either consume/tokenize a single character or a whole escape sequence.
 *
 * Should be called in a loop, will consume a single token on each call.
 *
 * @param s       The string to tokenize.
 * @param mbegin  Begin of a matched token.
 * @param mend    End of a matched token.
 *
 * @return The token type that was just consumed. CRINIT_TK_CPY for a single character to copy, CRINIT_TK_ESCSEQ for a
 *         two-character escape sequence (like `\n` for example), CRINIT_TK_ESCSEQX for a hexadecimal escape sequence
 *         (like `\x4f` for `O`), CRINIT_TK_END for end-of-string, and CRINIT_TK_ERR if the lexer encountered an error.
 */
crinitTokenType_t crinitEscLex(const char **s, const char **mbegin, const char **mend);

/**
 * Matches a fully quoted config value and removes quotes from match.
 *
 * Takes the \a value input from the libinih parser and takes care of things like
 * ```
 * "quoted '""''' string here"
 * ```
 * Resulting string (the range of characters between mbegin and mend) shall be
 * ```
 * quoted '""''' string here
 * ```
 * A string without outer "top-level" quotes will not match, neither a string which is not fully quoted.
 *
 * @param s       The string to try to match
 * @param mbegin  Output pointer for the begin of the match, will be equal to s on no match.
 * @param mend    Output pointer for the end of the match, will be equal to the terminating null char of s on no match.
 *
 * @return 1 on a match, 0 on no match, -1 on error
 */
int crinitMatchQuotedConfig(const char *s, const char **mbegin, const char **mend);

/**
 * Lexer/tokenizer for parsing an ENV_SET directive on the upper level.
 *
 * When repeatedly fed a string of the form `ENV_VAR_NAME "env var content"`, this function will advance the given
 * pointer over each token and tokenize that string as `<CRINIT_TK_ENVKEY CRINIT_TK_ENVVAR>`.
 *
 * In general, the function will tokenize any free-standing alphanumeric text as an env key and any quoted characters
 * as an env value. Sanity-checking to make sure we get a single key followed by a single value is left to the upper
 * layer.
 *
 * The function will return an CRINIT_TK_ERR if a free-standing (unquoted) non-alphanumeric character is encountered or
 * if a key begins with a number.
 *
 * Environment keys are matched fully while the quotes of environment values are consumed but left out of the matched
 * string.
 *
 * Whitespaces are matched/consumed as blocks.
 *
 * @param s       The string to tokenize, will be advanced over one token per call.
 * @param mbegin  Begin of a token match.
 * @param mend    End of a token match.
 *
 * @return CRINIT_TK_ERR on any error, CRINIT_TK_END on the end of the string, CRINIT_TK_ENVKEY on an env key match,
 *         CRINIT_TK_ENVVAL on an env value match, CRINIT_TK_WSPC on a whitespace (block) match.
 */
crinitTokenType_t crinitEnvVarOuterLex(const char **s, const char **mbegin, const char **mend);
/**
 * Lexer/tokenizer for parsing the value part of an ENV_SET directive.
 *
 * The lexer supports simple characters to copy, escape sequences (standard and hexadecimal), and variables of the form
 * `${VAR_NAME}`.
 *
 * In general, the match will contain the full token with two exceptions:
 *     - CRINIT_TK_VAR will consume `${VAR_NAME}` but match only `VAR_NAME`.
 *     - CRINIT_TK_ESCX will consume for example `\x5e` but match only `5e`.
 *
 * The only parser error to be encountered is a single backslash followed by the end-of-string, resulting in an illegal
 * escape sequence.
 *
 * @param s       The string to tokenize, will be advanced over one token per call.
 * @param mbegin  Begin of a token match.
 * @param mend    End of a token match.
 *
 * @return  An crinitTokenType_t as detailed above.
 */
crinitTokenType_t crinitEnvVarInnerLex(const char **s, const char **mbegin, const char **mend);

/**
 * Lexer/tokenizer for parsing the Kernel command line.
 *
 * Matches all variables of the form `crinit.<key>=<val>`. The value can be double-quoted.
 *
 * @param s  The string to tokenize, will be advanced over one toke per call.
 * @param keyBegin  Begin of the `<key>` part within a token match as explained above.
 * @param keyEnd    End of the `<key>` part within a token match as explained above.
 * @param valBegin  Begin of the `<val>` part within a token match as explained above.
 * @param valEnd    End of the `<val>` part within a token match as explained above.
 *
 * @return CRINIT_TK_ERR on any error, CRINIT_TK_END on the end of the string, CRINIT_TK_VAR on a variable match as
 *         shown above, CRINIT_TK_WSPC on a whitespace (block) match, CRINIT_TK_CPY on other unrelated characters.
 */
crinitTokenType_t crinitKernelCmdlineLex(const char **s, const char **keyBegin, const char **keyEnd,
                                         const char **valBegin, const char **valEnd);

#endif /* __LEXERS_H__ */
