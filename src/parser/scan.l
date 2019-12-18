%top{
#include <litesql/int.h>
#include <litesql/parser/parser.h>
#include <litesql/parser/keywords.h>
#include <litesql/utils/memctx.h>
#include <litesql/utils/elog.h>
#include <algorithm>
#include "gram.hpp"
using namespace db;
}

%{
typedef PARSER_STYPE YYSTYPE;
typedef PARSER_LTYPE YYLTYPE;

#define SCANNER (&((Parser*)scanner_get_extra(yyscanner))->scanner)

/*
 * Advance yylloc by the given number of bytes.
 */

#define startlit()	( SCANNER->literalBuf.clear() )

static void addlit(char *ytext, int yleng, yyscan_t yyscanner);
static void addlitchar(unsigned char ychar, yyscan_t yyscanner);
static char *litbufdup(yyscan_t yyscanner);

static int	process_integer_literal(const char *token, YYSTYPE *lval);

%}

%option reentrant
%option bison-bridge
%option bison-locations
%option 8bit
%option never-interactive
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option noyyalloc
%option noyyrealloc
%option noyyfree
%option warn
%option prefix="scanner_"

/*
 * OK, here is a short description of lex/flex rules behavior.
 * The longest pattern which matches an input string is always chosen.
 * For equal-length patterns, the first occurring in the rules list is chosen.
 * INITIAL is the starting state, to which all non-conditional rules apply.
 * Exclusive states change parsing rules while the state is active.  When in
 * an exclusive state, only those rules defined for that state apply.
 *
 * We use exclusive states for quoted strings, extended comments,
 * and to eliminate parsing troubles for numeric strings.
 * Exclusive states:
 *  <xb> bit string literal
 *  <xc> extended C-style comments
 *  <xd> delimited identifiers (double-quoted identifiers)
 *  <xh> hexadecimal numeric string
 *  <xq> standard quoted strings
 *
 * Remember to add an <<EOF>> case whenever you add a new exclusive state!
 * The default one is probably not the right thing.
 */

%x xb
%x xc
%x xd
%x xh
%x xq

/*
 * In order to make the world safe for Windows and Mac clients as well as
 * Unix ones, we accept either \n or \r as a newline.  A DOS-style \r\n
 * sequence will be seen as two successive newlines, but that doesn't cause
 * any problems.  Comments that start with -- and extend to the next
 * newline are treated as equivalent to a single whitespace character.
 *
 * NOTE a fine point: if there is no newline following --, we will absorb
 * everything to the end of the input as a comment.  This is correct.  Older
 * versions of Postgres failed to recognize -- as a comment if the input
 * did not end with a newline.
 *
 * XXX perhaps \f (formfeed) should be treated as a newline as well?
 *
 * XXX if you change the set of whitespace characters, fix scanner_isspace()
 * to agree, and see also the plpgsql lexer.
 */

space			[ \t\n\r\f]
horiz_space		[ \t\f]
newline			[\n\r]
non_newline		[^\n\r]

comment			("--"{non_newline}*)

whitespace		({space}+|{comment})

/*
 * SQL requires at least one newline in the whitespace separating
 * string literals that are to be concatenated.  Silly, but who are we
 * to argue?  Note that {whitespace_with_newline} should not have * after
 * it, whereas {whitespace} should generally have a * after it...
 */

special_whitespace		({space}+|{comment}{newline})
horiz_whitespace		({horiz_space}|{comment})
whitespace_with_newline	({horiz_whitespace}*{newline}{special_whitespace}*)

/*
 * To ensure that {quotecontinue} can be scanned without having to back up
 * if the full pattern isn't matched, we include trailing whitespace in
 * {quotestop}.  This matches all cases where {quotecontinue} fails to match,
 * except for {quote} followed by whitespace and just one "-" (not two,
 * which would start a {comment}).  To cover that we have {quotefail}.
 * The actions for {quotestop} and {quotefail} must throw back characters
 * beyond the quote proper.
 */
quote			'
quotestop		{quote}{whitespace}*
quotecontinue	{quote}{whitespace_with_newline}{quote}
quotefail		{quote}{whitespace}*"-"

/* Bit string
 * It is tempting to scan the string for only those characters
 * which are allowed. However, this leads to silently swallowed
 * characters if illegal characters are included in the string.
 * For example, if xbinside is [01] then B'ABCD' is interpreted
 * as a zero-length string, and the ABCD' is lost!
 * Better to pass the string forward and let the input routines
 * validate the contents.
 */
xbstart			[bB]{quote}
xbinside		[^']*

/* Hexadecimal number */
xhstart			[xX]{quote}
xhinside		[^']*

/* Extended quote
 * xqdouble implements embedded quote, ''''
 */
xqstart			{quote}
xqdouble		{quote}{quote}
xqinside		[^']+

/* Double quote
 * Allows embedded spaces and other special characters into identifiers.
 */
dquote			\"
xdstart			{dquote}
xdstop			{dquote}
xddouble		{dquote}{dquote}
xdinside		[^"]+

/* C-style comments
 *
 * The "extended comment" syntax closely resembles allowable operator syntax.
 * The tricky part here is to get lex to recognize a string starting with
 * slash-star as a comment, when interpreting it as an operator would produce
 * a longer match --- remember lex will prefer a longer match!  Also, if we
 * have something like plus-slash-star, lex will think this is a 3-character
 * operator whereas we want to see it as a + operator and a comment start.
 * The solution is two-fold:
 * 1. append {op_chars}* to xcstart so that it matches as much text as
 *    {operator} would. Then the tie-breaker (first matching rule of same
 *    length) ensures xcstart wins.  We put back the extra stuff with yyless()
 *    in case it contains a star-slash that should terminate the comment.
 * 2. In the operator rule, check for slash-star within the operator, and
 *    if found throw it back with yyless().  This handles the plus-slash-star
 *    problem.
 * Dash-dash comments have similar interactions with the operator rule.
 */
xcstart			\/\*{op_chars}*
xcstop			\*+\/
xcinside		[^*/]+

digit			[0-9]
ident_start		[A-Za-z\200-\377_]
ident_cont		[A-Za-z\200-\377_0-9\$]

identifier		{ident_start}{ident_cont}*

/*
 * These operator-like tokens (unlike the above ones) also match the {operator}
 * rule, which means that they might be overridden by a longer match if they
 * are followed by a comment start or a + or - character. Accordingly, if you
 * add to this list, you must also add corresponding code to the {operator}
 * block to return the correct token in such cases.
 */
equals_greater	"=>"
less_equals		"<="
greater_equals	">="
less_greater	"<>"
not_equals		"!="

/*
 * "self" is the set of chars that should be returned as single-character
 * tokens.  "op_chars" is the set of chars that can make up "Op" tokens,
 * which can be one or more characters long (but if a single-char token
 * appears in the "self" set, it is not to be returned as an Op).  Note
 * that the sets overlap, but each has some chars that are not in the other.
 *
 * If you change either set, adjust the character lists appearing in the
 * rule for "operator"!
 */
self            [,()\[\].;\:\+\-\*\/\%\^\<\>\=]
op_chars		[\~\!\@\#\^\&\|\`\?\+\-\*\/\%\<\>\=]
operator		{op_chars}+

/* we no longer allow unary minus in numbers.
 * instead we pass it separately to parser. there it gets
 * coerced via doNegate() -- Leon aug 20 1999
 *
 * {decimalfail} is used because we would like "1..10" to lex as 1, dot_dot, 10.
 *
 * {realfail1} and {realfail2} are added to prevent the need for scanner
 * backup when the {real} rule fails to match completely.
 */

integer			{digit}+
decimal			(({digit}*\.{digit}+)|({digit}+\.{digit}*))
decimalfail		{digit}+\.\.
real			({integer}|{decimal})[Ee][-+]?{digit}+
realfail1		({integer}|{decimal})[Ee]
realfail2		({integer}|{decimal})[Ee][-+]

param			\${integer}

other			.

%%
{whitespace}	{
					/* ignore */
				}

{xcstart}		{
					SCANNER->xcdepth = 0;
					BEGIN(xc);
					/* Put back any characters past slash-star; see above */
					yyless(2);
				}

<xc>{xcstart}	{
					(SCANNER->xcdepth)++;
					/* Put back any characters past slash-star; see above */
					yyless(2);
				}

<xc>{xcstop}	{
					if (SCANNER->xcdepth <= 0)
						BEGIN(INITIAL);
					else
						(SCANNER->xcdepth)--;
				}

<xc>{xcinside}	{
					/* ignore */
				}

<xc>{op_chars}	{
					/* ignore */
				}

<xc>\*+			{
					/* ignore */
				}

<xc><<EOF>>		{ elog(ERROR, "unterminated /* comment"); }

{xbstart}		{
					/* Binary bit type.
					 * At some point we should simply pass the string
					 * forward to the parser and label it there.
					 * In the meantime, place a leading "b" on the string
					 * to mark it for the input routine as a binary string.
					 */
					BEGIN(xb);
					startlit();
					addlitchar('b', yyscanner);
				}
<xb>{quotestop}	|
<xb>{quotefail} {
					yyless(1);
					BEGIN(INITIAL);
					yylval->str = litbufdup(yyscanner);
					return BCONST;
				}
<xh>{xhinside}	|
<xb>{xbinside}	{
					addlit(yytext, yyleng, yyscanner);
				}
<xh>{quotecontinue}	|
<xb>{quotecontinue}	{
                        /* ignore */
				    }
<xb><<EOF>>		{ elog(ERROR, "unterminated bit string literal"); }

{xhstart}		{
					/* Hexadecimal bit type.
					 * At some point we should simply pass the string
					 * forward to the parser and label it there.
					 * In the meantime, place a leading "x" on the string
					 * to mark it for the input routine as a hex string.
					 */
					BEGIN(xh);
					startlit();
					addlitchar('x', yyscanner);
				}
<xh>{quotestop}	|
<xh>{quotefail} {
					yyless(1);
					BEGIN(INITIAL);
					yylval->str = litbufdup(yyscanner);
					return XCONST;
				}
<xh><<EOF>>		{ elog(ERROR, "unterminated hexadecimal string literal"); }

{xqstart}		{
                    BEGIN(xq);
					startlit();
				}
<xq>{quotestop}	|
<xq>{quotefail} {
					yyless(1);
					BEGIN(INITIAL);
					yylval->str = litbufdup(yyscanner);
					return SCONST;
				}
<xq>{xqdouble}  { addlitchar('\'', yyscanner); }
<xq>{xqinside}  { addlit(yytext, yyleng, yyscanner); }
<xq>{quotecontinue} { /* ignore */ }
<xq><<EOF>>		{ elog(ERROR, "unterminated quoted string"); }

{xdstart}		{
					BEGIN(xd);
					startlit();
				}

<xd>{xdstop}	{
					BEGIN(INITIAL);
					if (SCANNER->literalBuf.empty()) {
						elog(ERROR, "zero-length delimited identifier");
					}
					yylval->str = litbufdup(yyscanner);
					return IDENT;
				}
<xd>{xddouble}	{
					addlitchar('"', yyscanner);
				}
<xd>{xdinside}	{
					addlit(yytext, yyleng, yyscanner);
				}
<xd><<EOF>>		{ elog(ERROR, "unterminated quoted identifier"); }

{equals_greater} {
					return EQUALS_GREATER;
				}

{less_equals}	{
					return LESS_EQUALS;
				}

{greater_equals} {
					return GREATER_EQUALS;
				}

{less_greater}	{
					/* We accept both "<>" and "!=" as meaning NOT_EQUALS */
					return NOT_EQUALS;
				}

{not_equals}	{
					/* We accept both "<>" and "!=" as meaning NOT_EQUALS */
					return NOT_EQUALS;
				}

{self}			{
					return yytext[0];
				}
{operator}		{
					/*
					 * Check for embedded slash-star or dash-dash; those
					 * are comment starts, so operator must stop there.
					 * Note that slash-star or dash-dash at the first
					 * character will match a prior rule, not this one.
					 */
					int			nchars = yyleng;
					char	   *slashstar = strstr(yytext, "/*");
					char	   *dashdash = strstr(yytext, "--");

					if (slashstar && dashdash)
					{
						/* if both appear, take the first one */
						if (slashstar > dashdash)
							slashstar = dashdash;
					}
					else if (!slashstar)
						slashstar = dashdash;
					if (slashstar)
						nchars = slashstar - yytext;

					/*
					 * For SQL compatibility, '+' and '-' cannot be the
					 * last char of a multi-char operator unless the operator
					 * contains chars that are not in SQL operators.
					 * The idea is to lex '=-' as two operators, but not
					 * to forbid operator names like '?-' that could not be
					 * sequences of SQL operators.
					 */
					if (nchars > 1 &&
						(yytext[nchars - 1] == '+' ||
						 yytext[nchars - 1] == '-'))
					{
						int			ic;

						for (ic = nchars - 2; ic >= 0; ic--)
						{
							char c = yytext[ic];
							if (c == '~' || c == '!' || c == '@' ||
								c == '#' || c == '^' || c == '&' ||
								c == '|' || c == '`' || c == '?' ||
								c == '%')
								break;
						}
						if (ic < 0)
						{
							/*
							 * didn't find a qualifying character, so remove
							 * all trailing [+-]
							 */
							do {
								nchars--;
							} while (nchars > 1 &&
								 (yytext[nchars - 1] == '+' ||
								  yytext[nchars - 1] == '-'));
						}
					}

					if (nchars < yyleng)
					{
						/* Strip the unwanted chars from the token */
						yyless(nchars);
						/*
						 * If what we have left is only one char, and it's
						 * one of the characters matching "self", then
						 * return it as a character token the same way
						 * that the "self" rule would have.
						 */
						if (nchars == 1 &&
							strchr(",()[].;:+-*/%^<>=", yytext[0]))
							return yytext[0];
						/*
						 * Likewise, if what we have left is two chars, and
						 * those match the tokens ">=", "<=", "=>", "<>" or
						 * "!=", then we must return the appropriate token
						 * rather than the generic Op.
						 */
						if (nchars == 2)
						{
							if (yytext[0] == '=' && yytext[1] == '>')
								return EQUALS_GREATER;
							if (yytext[0] == '>' && yytext[1] == '=')
								return GREATER_EQUALS;
							if (yytext[0] == '<' && yytext[1] == '=')
								return LESS_EQUALS;
							if (yytext[0] == '<' && yytext[1] == '>')
								return NOT_EQUALS;
							if (yytext[0] == '!' && yytext[1] == '=')
								return NOT_EQUALS;
						}
					}

					/*
					 * Complain if operator is too long.  Unlike the case
					 * for identifiers, we make this an error not a notice-
					 * and-truncate, because the odds are we are looking at
					 * a syntactic mistake anyway.
					 */
					if (nchars >= NAMEDATALEN)
						elog(ERROR, "operator too long");

					yylval->str = Strdup(yytext);
					return OP;
				}

{param}			{
					yylval->ival = atol(yytext + 1);
					return PARAM;
				}
{integer}		{
					return process_integer_literal(yytext, yylval);
				}
{decimal}		{
					yylval->str = Strdup(yytext);
					return FCONST;
				}
{decimalfail}	{
					/* throw back the .., and treat as integer */
					yyless(yyleng - 2);
					return process_integer_literal(yytext, yylval);
				}
{real}			{
					yylval->str = Strdup(yytext);
					return FCONST;
				}
{realfail1}		{
					/*
					 * throw back the [Ee], and treat as {decimal}.  Note
					 * that it is possible the input is actually {integer},
					 * but since this case will almost certainly lead to a
					 * syntax error anyway, we don't bother to distinguish.
					 */
					yyless(yyleng - 1);
					yylval->str = Strdup(yytext);
					return FCONST;
				}
{realfail2}		{
					/* throw back the [Ee][+-], and proceed as above */
					yyless(yyleng - 2);
					yylval->str = Strdup(yytext);
					return FCONST;
				}

{identifier}	{
					const ScanKeyword *keyword;
					char	   *ident;

					/* Is it a keyword? */
					keyword = ScanKeywordLookup(yytext);
					if (keyword != NULL)
					{
						yylval->keyword = keyword->name;
						return keyword->value;
					}
					ident = (char*) Malloc(yyleng + 1);
					memcpy(ident, yytext, yyleng);
					ident[yyleng] = 0;
					yylval->str = ident;
					return IDENT;
				}

{other}			{
					return yytext[0];
				}

<<EOF>>			{
					yyterminate();
				}

%%

void *scanner_alloc (yy_size_t size,yyscan_t yyscanner) {
    if (size > 0) {
        return Malloc(size);
    } else {
        return NULL;
    }
}

void scanner_free(void *ptr, yyscan_t yyscanner) {
    if (ptr) {
        Free(ptr);
    }
}

void *scanner_realloc (void* ptr, yy_size_t size, yyscan_t yyscanner) {
    if (ptr) {
        void* ret = Malloc(size);
        memcpy(ret, ptr, size);
        return ret;
    } else {
        return NULL;
    }
}

static void addlit(char *ytext, int yleng, yyscan_t yyscanner) {
    SCANNER->literalBuf.insert(SCANNER->literalBuf.end(), ytext, ytext + yleng);
}

static void addlitchar(unsigned char ychar, yyscan_t yyscanner) {
    SCANNER->literalBuf.push_back(ychar);
}

static char *litbufdup(yyscan_t yyscanner) {
    size_t len = SCANNER->literalBuf.size();
    char *str = (char*) Malloc(len + 1);
    memcpy(str, SCANNER->literalBuf.data(), len);
    str[len] = '\0';
    return str;
}

static int	process_integer_literal(const char *token, YYSTYPE *lval) {
    long val;
    char *endptr;

	errno = 0;
	val = strtol(token, &endptr, 10);
	if (*endptr != '\0' || val != (int)val)
	{
		/* integer too large, treat it as a float */
		lval->str = Strdup(token);
		return FCONST;
	}
	lval->ival = val;
	return ICONST;
}

namespace db
{

int ScannerCurrentPosition(Scanner* scanner, char* token, size_t tokenSize) {
    struct yyguts_t * yyg = (struct yyguts_t*)(scanner->flex);
    if (yytext) {
        size_t position = yytext - scanner-> scanBuf;
        snprintf(token, tokenSize, "%s", yytext);
        return position + 1;
    } else {
        return 0;
    }
}

void ScannerInit(Scanner* scanner, char* query, size_t queryLen) {
    void* flex;
    if (scanner_lex_init(&flex) != 0 ) {
         elog(ERROR, "scanner_lex_init failed");
    }
    scanner_set_extra(scanner, flex);
    scanner->scanBuf = query;
    scanner->scanBufLen = queryLen;
    scanner->scanBuf[queryLen] = scanner->scanBuf[queryLen + 1] = YY_END_OF_BUFFER_CHAR;
    scanner->flex = flex;
    scanner->scanBufLen = queryLen + 2;
    scanner__scan_buffer(scanner->scanBuf, queryLen + 2, flex);
}

void ScannerFinish(Scanner* scanner) {

}

}
