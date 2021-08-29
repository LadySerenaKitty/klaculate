#include "ktok.h"
#include <cmath>
#include <iomanip>

bool operator==(ttype &l, unsigned char r[sizeof(int)]) {
	return l.num == ttype(r).num;
}
bool operator==(ttype &l, int r) {
	return l.num == r;
}
bool operator==(ttype &l, ttype &r) {
	return l.num == r.num;
}
bool operator==(unsigned char l[sizeof(int)], ttype &r) {
	return ttype(l).num == r.num;
}
bool operator==(int l, ttype &r) {
	return l == r.num;
}

bool operator!=(ttype &l, unsigned char r[sizeof(int)]) {
	return !(l.num == ttype(r).num);
}
bool operator!=(ttype &l, int r) {
	return !(l.num == r);
}
bool operator!=(ttype &l, ttype &r) {
	return !(l.num == r.num);
}
bool operator!=(char l[sizeof(int)], ttype &r) {
	return !(ttype(r).num == r.num);
}
bool operator!=(int l, ttype &r) {
	return !(l == r.num);
}

ttype::ttype() {
	num = 0;
}
ttype::ttype(unsigned char r[sizeof(int)]) {
	for (int a = 0; a < sizeof(int); a++) { str[a] = r[a]; }
}
ttype::ttype(int r) {
	num = r;
}

ttype& ttype::operator=(int r) {
	num = r;
	return *this;
}
ttype& ttype::operator=(ttype &r) {
	num = r.num;
	return *this;
}
ttype& ttype::operator=(const ttype &r) {
	num = r.num;
	return *this;
}

bool operator==(token &l, token &r) {
	if (l.value != r.value) { return false; }
	if (l.type.num != r.type.num) { return false; }
	return true;
}

token operator*(token &l, token &r) {
	return token{l.type, l.value * r.value};
}
token operator/(token &l, token &r) {
	return token{TOKEN_NUMBER, l.value / r.value};
}
token operator%(token &l, token &r) {
	return token{TOKEN_NUMBER, std::fmod(l.value, r.value)};
}
token operator+(token &l, token &r) {
	return token{TOKEN_NUMBER, l.value + r.value};
}
token operator-(token &l, token &r) {
	return token{TOKEN_NUMBER, l.value - r.value};
}

token& token::operator=(token &r) {
	type = r.type;
	value = r.value;
	return *this;
}
token& token::operator=(const token &r) {
	type = r.type;
	value = r.value;
	return *this;
}

std::ostream& operator<<(std::ostream &l, ttype &r) {
	l << r.str;
	return l;
}
std::ostream& operator<<(std::ostream &l, token &r) {
	if (r.type == TOKEN_NUMBER) {
		if (std::isinf(r.value)) { l << "∞"; }
		else { l << r.value; }
	}
	else { l << r.type; }
	return l;
}
