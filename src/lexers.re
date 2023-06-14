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
#include <string.h>

#include "lexers.h"
#include "logio.h"

/*!conditions:re2c*/

const char EBCL_escMap[128] = {['n'] = '\n',  ['r'] = '\r', ['b'] = '\b', ['a'] = '\a', ['"'] = '"', ['\''] = '\'',
                               ['\\'] = '\\', ['f'] = '\f', ['t'] = '\t', ['v'] = '\v', ['$'] = '$', [' '] = ' '};

ebcl_TokenType_t EBCL_argvLex(const char **s, const char **mbegin, const char **mend, bool dq) {
    if (s == NULL || *s == NULL || mbegin == NULL || mend == NULL) {
        crinitErrPrint("Input parameters must not be NULL.");
        return EBCL_TK_ERR;
    }

    const char *YYCURSOR = *s, *YYMARKER = *s, *anchor = *s;

    const char *t1, *t2, *yyt1;

    int c = (dq) ? yycdquot : yycnquot;

    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:define:YYGETCONDITION:naked = 1;
        re2c:define:YYGETCONDITION = "c";
        re2c:define:YYSETCONDITION:naked = 1;
        re2c:define:YYSETCONDITION = "c = @@;";
        re2c:yyfill:enable = 0;
        re2c:tags = 1;

        whitespace = [ \t]+;
        end        = "\x00";
        uqstr      = ([^ "\\\n\x00\t] | [\\][^\x00])+;
        dqstr      = ["] @t1 ([^"\\\n\x00] | [\\][^\x00])* @t2 ["];
        nqstr      = ([^ \\\n\x00\t] | [\\][^\x00])+;


        <*>        *          { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_ERR; }
        <*>        end        { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_END; }
        <*>        whitespace { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_WSPC; }
        <dquot>    uqstr      { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_UQSTR; }
        <dquot>    dqstr      { *s = YYCURSOR; *mbegin = t1; *mend = t2; return EBCL_TK_DQSTR; }
        <nquot>    nqstr      { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_UQSTR; }
    */
}

ebcl_TokenType_t EBCL_escLex(const char **s, const char **mbegin, const char **mend) {
    if (s == NULL || *s == NULL || mbegin == NULL || mend == NULL) {
        crinitErrPrint("Input parameters must not be NULL.");
        return EBCL_TK_ERR;
    }

    const char *YYCURSOR = *s, *YYMARKER = *s, *anchor = *s;

    const char *t1, *t2;

    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;
        re2c:tags = 1;

        any        = [^\x00\\];
        escseq     = [\\] ( any | [\\] );
        escseqx    = [\\][x] @t1 [0-9a-fA-F]{2} @t2;

        *          { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_ERR; }
        any        { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_CPY; }
        escseq     { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_ESC; }
        escseqx    { *s = YYCURSOR; *mbegin = t1; *mend = t2; return EBCL_TK_ESCX; }
        end        { *s = YYCURSOR; *mbegin = anchor; *mend = *s; return EBCL_TK_END; }
    */
}

int EBCL_matchQuotedConfig(const char *s, const char **mbegin, const char **mend) {
    if (s == NULL || mbegin == NULL || mend == NULL) {
        crinitErrPrint("Input parameters must not be NULL.");
        return -1;
    }

    const char *YYCURSOR = s, *YYMARKER = s;

    const char *t1, *t2, *yyt1, *yyt2;
    /*!re2c
            re2c:define:YYCTYPE = char;
            re2c:yyfill:enable = 0;
            re2c:tags = 1;

            wspc_opt   = [ ]*;
            qconf      = wspc_opt ["]@t1[^\x00]*@t2["] wspc_opt end;

            *          { *mbegin = s; *mend = strchr(s, '\0'); return 0; }
            qconf      { *mbegin = t1; *mend = t2; return 1; }
    */
}

ebcl_TokenType_t EBCL_envVarOuterLex(const char **s, const char **mbegin, const char **mend) {
    if (s == NULL || *s == NULL || mbegin == NULL || mend == NULL) {
        crinitErrPrint("Input parameters must not be NULL.");
        return EBCL_TK_ERR;
    }

    const char *YYCURSOR = *s, *YYMARKER = *s, *anchor = *s;

    const char *t1, *t2, *yyt1;

    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;
        re2c:tags = 1;

        envkey     = [a-zA-Z_][a-zA-Z0-9_]*;
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
        crinitErrPrint("Input parameters must not be NULL.");
        return EBCL_TK_ERR;
    }

    const char *YYCURSOR = *s, *YYMARKER = *s, *anchor = *s;

    const char *t1, *t2, *yyt1;

    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;
        re2c:tags = 1;

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
