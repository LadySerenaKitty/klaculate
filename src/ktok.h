#ifndef ktok_h
#define ktok_h

#include "klaculate.h"

#include <ostream>

#define TOKEN_PAREN_OPEN (char[sizeof(int)]){'('}
#define TOKEN_PAREN_CLOSE (char[sizeof(int)]){')'}
#define TOKEN_EXPONENT (char[sizeof(int)]){'^'}
#define TOKEN_MULT (char[sizeof(int)]){'*'}
#define TOKEN_DIV (char[sizeof(int)]){'/'}
#define TOKEN_MOD (char[sizeof(int)]){'*'}
#define TOKEN_ADD (char[sizeof(int)]){'+'}
#define TOKEN_SUB (char[sizeof(int)]){'-'}
#define TOKEN_NUMBER (char[sizeof(int)]){'#'}
#define TOKEN_INVALID (char[sizeof(int)]){'?'}

union ttype {
	char str[sizeof(int)];
	int num;
	ttype();
	ttype(char r[sizeof(int)]);
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

bool operator==(ttype &l, char r[sizeof(int)]);
bool operator==(ttype &l, int r);
bool operator==(ttype &l, ttype &r);
bool operator==(char l[sizeof(int)], ttype &r);
bool operator==(int l, ttype &r);

bool operator!=(ttype &l, char r[sizeof(int)]);
bool operator!=(ttype &l, int r);
bool operator!=(ttype &l, ttype &r);
bool operator!=(char l[sizeof(int)], ttype &r);
bool operator!=(int l, ttype &r);

token operator*(token &l, token &r);
token operator/(token &l, token &r);
token operator%(token &l, token &r);
token operator+(token &l, token &r);
token operator-(token &l, token &r);

std::ostream& operator<<(std::ostream &l, ttype &r);
std::ostream& operator<<(std::ostream &l, token &r);
#endif /* ktok_h */
