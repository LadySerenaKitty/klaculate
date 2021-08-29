#ifndef ktok_h
#define ktok_h

#include "klaculate.h"

#include <ostream>

#define tsize (unsigned char[sizeof(int)])

#define TOKEN_PAREN_OPEN tsize{'(', 0, 0, 0}
#define TOKEN_PAREN_CLOSE tsize{')', 0, 0, 0}
#define TOKEN_EXPONENT tsize{'^', 0, 0, 0}
#define TOKEN_MULT tsize{'*', 0, 0, 0}
#define TOKEN_DIV tsize{'/', 0, 0, 0}
#define TOKEN_MOD tsize{'*', 0, 0, 0}
#define TOKEN_ADD tsize{'+', 0, 0, 0}
#define TOKEN_SUB tsize{'-', 0, 0, 0}
#define TOKEN_NUMBER tsize{'#', 0, 0, 0}
#define TOKEN_INVALID tsize{'?', 0, 0, 0}

union ttype {
	unsigned char str[sizeof(int)];
	int num;
	ttype();
	ttype(unsigned char r[sizeof(int)]);
	ttype(int r);
	ttype& operator=(int r);
	ttype& operator=(ttype &r);
	ttype& operator=(const ttype &r);
};

struct token {
	ttype type;
	double value;
	token& operator=(token &r);
	token& operator=(const token &r);
};

bool operator==(ttype &l, unsigned char r[sizeof(int)]);
bool operator==(ttype &l, int r);
bool operator==(ttype &l, ttype &r);
bool operator==(unsigned char l[sizeof(int)], ttype &r);
bool operator==(int l, ttype &r);

bool operator!=(ttype &l, unsigned char r[sizeof(int)]);
bool operator!=(ttype &l, int r);
bool operator!=(ttype &l, ttype &r);
bool operator!=(unsigned char l[sizeof(int)], ttype &r);
bool operator!=(int l, ttype &r);

bool operator==(token &l, token &r);

token operator*(token &l, token &r);
token operator/(token &l, token &r);
token operator%(token &l, token &r);
token operator+(token &l, token &r);
token operator-(token &l, token &r);

std::ostream& operator<<(std::ostream &l, ttype &r);
std::ostream& operator<<(std::ostream &l, token &r);
#endif /* ktok_h */
