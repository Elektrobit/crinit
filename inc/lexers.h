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

typedef enum ebcl_TokenType_t {
    EBCL_TK_ERR = -1,
    EBCL_TK_END = 0,
    EBCL_TK_ENVKEY,
    EBCL_TK_ENVVAL,
    EBCL_TK_WSPC,
    EBCL_TK_VAR,
    EBCL_TK_ESC,
    EBCL_TK_ESCX,
    EBCL_TK_CPY
} ebcl_TokenType_t;

ebcl_TokenType_t EBCL_envVarInnerLex(const char **s, const char **mbegin, const char **mend);
ebcl_TokenType_t EBCL_envVarOuterLex(const char **s, const char **mbegin, const char **mend); 

#endif /* __LEXERS_H__ */
