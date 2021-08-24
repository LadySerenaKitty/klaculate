#ifndef ktok_h
#define ktok_h

#include <ostream>

#define TOKEN_PAREN_OPEN '('
#define TOKEN_PAREN_CLOSE ')'
#define TOKEN_EXPONENT '^'
#define TOKEN_MULT '*'
#define TOKEN_DIV '/'
#define TOKEN_MOD '%'
#define TOKEN_ADD '+'
#define TOKEN_SUB '-'
#define TOKEN_NUMBER '#'
#define TOKEN_INVALID '?'

typedef struct tokens {
	unsigned char type;
	double value;
} token;

token operator*(token &l, token &r);
token operator/(token &l, token &r);
token operator%(token &l, token &r);
token operator+(token &l, token &r);
token operator-(token &l, token &r);

std::ostream& operator<<(std::ostream &l, token &r);
#endif /* ktok_h */
