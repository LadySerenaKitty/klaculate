#ifndef util_TokenData_h
#define util_TokenData_h

#include "../ktok.h"

#include <map>

struct TokenData {
	bool leftNumber;
	bool rightNumber;

	token insertLeft;
	token insertRight;
	token identity;
};

extern std::map<int, TokenData> tdata;

void tdInit();

bool isValidOperatorToken(token &input);
bool isValidOperatorToken(const token &input);

bool tokenLeftIsNumber(token &input);
bool tokenLeftIsOperator(token &input);

bool tokenRightIsNumber(token &input);
bool tokenRightIsOperator(token &input);

token tokenInsertLeft(token &input);
token tokenInsertRight(token &input);

token tokenIdentity(token &input);
token tokenIdentity(const token &input);

bool tokenNumeric(token &input);
bool tokenNumeric(const token &input);

#endif /* util_TokenData_h */
