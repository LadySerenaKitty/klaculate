#ifndef ktok_h
#define ktok_h

#include <ostream>

#define TOKEN_PAREN_OPEN 1
#define TOKEN_PAREN_CLOSE 2
#define TOKEN_EXPONENT 3
#define TOKEN_MULT 4
#define TOKEN_DIV 5
#define TOKEN_MOD 6
#define TOKEN_ADD 7
#define TOKEN_SUB 8
#define TOKEN_NUMBER 0x7f

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
