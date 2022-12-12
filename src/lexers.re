/**
 * @file lexers.c
 * @brief Implementation lexers built with re2c, used in the config/envset parser.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "lexers.h"
#include "logio.h"

ebcl_TokenType_t EBCL_envVarOuterLex(const char **s, const char **mbegin, const char **mend) {
    if (s == NULL || *s == NULL || mbegin == NULL || mend == NULL) {
        EBCL_errPrint("Input parameters must not be NULL.");
        return EBCL_TK_ERR;
    }

    const char *YYCURSOR = *s, *YYMARKER = *s, *anchor = *s;

    const char *t1, *t2, *yyt1;

    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;
        re2c:tags = 1;

        end        = "\x00";
        envkey     = [a-zA-Z_][a-zA-Z0-9_]*;
        whitespace = [ ]+;
        envval     = ["]@t1[^\x00]*@t2["];

        *          { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_ERR; }
        envkey     { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_ENVKEY; }
        end        { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_END; }
        whitespace { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_WSPC; }
        envval     { *s = YYCURSOR; *mbegin = t1; *mend = t2; return EBCL_TK_ENVVAL; }
    */
}

ebcl_TokenType_t EBCL_envVarInnerLex(const char **s, const char **mbegin, const char **mend) {
    if (s == NULL || *s == NULL || mbegin == NULL || mend == NULL) {
        EBCL_errPrint("Input parameters must not be NULL.");
        return EBCL_TK_ERR;
    }

    const char *YYCURSOR = *s, *YYMARKER = *s, *anchor = *s;

    const char *t1, *t2, *yyt1;

    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;
        re2c:tags = 1;

        any        = [^\x00\\];
        escseq     = [\\] ( any | [\\] );
        escseqx    = [\\][x] @t1 [0-9a-fA-F]{2} @t2;
        varname    = [a-zA-Z_][a-zA-Z0-9_]*;
        var        = "\$\{" @t1 varname @t2 "\}";

        *          { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_ERR; }
        any        { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_CPY; }
        escseq     { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_ESC; }
        escseqx    { *s = YYCURSOR; *mbegin = t1; *mend = t2; return EBCL_TK_ESCX; }
        end        { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_END; }
        var        { *s = YYCURSOR; *mbegin = t1; *mend = t2; return EBCL_TK_VAR; }
    */
}
