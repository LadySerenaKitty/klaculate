#include "TokenData.h"

std::map<int, TokenData> tdata;
void tdInit() {
	tdata.insert(std::pair<int, TokenData>(ttype('(').num,
		TokenData{true, false, token{TOKEN_MULT, 0}, token{TOKEN_NUMBER, 1}, token{TOKEN_PAREN_OPEN, 0}}));
	tdata.insert(std::pair<int, TokenData>(ttype(')').num,
		TokenData{false, true, token{TOKEN_NUMBER, 1}, token{TOKEN_MULT, 0}, token{TOKEN_PAREN_CLOSE, 0}}));
	tdata.insert(std::pair<int, TokenData>(ttype('^').num,
		TokenData{false, false, token{TOKEN_NUMBER, 1}, token{TOKEN_NUMBER, 1}, token{TOKEN_EXPONENT, 0}}));
	tdata.insert(std::pair<int, TokenData>(ttype('*').num,
		TokenData{false, false, token{TOKEN_NUMBER, 1}, token{TOKEN_NUMBER, 1}, token{TOKEN_MULT, 0}}));
	tdata.insert(std::pair<int, TokenData>(ttype('/').num,
		TokenData{false, false, token{TOKEN_NUMBER, 1}, token{TOKEN_NUMBER, 1}, token{TOKEN_DIV, 0}}));
	tdata.insert(std::pair<int, TokenData>(ttype('%').num,
		TokenData{false, false, token{TOKEN_NUMBER, 1}, token{TOKEN_NUMBER, 1}, token{TOKEN_MOD, 0}}));
	tdata.insert(std::pair<int, TokenData>(ttype('+').num,
		TokenData{false, false, token{TOKEN_NUMBER, 0}, token{TOKEN_NUMBER, 0}, token{TOKEN_ADD, 0}}));
	tdata.insert(std::pair<int, TokenData>(ttype('-').num,
		TokenData{false, false, token{TOKEN_NUMBER, 0}, token{TOKEN_NUMBER, 0}, token{TOKEN_SUB, 0}}));
	tdata.insert(std::pair<int, TokenData>(ttype(tsize{'p', 'i', 0, 0}).num,
		TokenData{true, true, token{TOKEN_ADD, 0}, token{TOKEN_ADD, 0}, token{TOKEN_NUMBER, 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651328230664709384460955058223172535940812848111745028410270193852110555964462294895493}}));
}

bool isValidOperatorToken(token &input) {
	try { tdata.at(input.type.num); }
	catch (std::out_of_range oor) { return false; }
	return true;
}
bool isValidOperatorToken(const token &input) {
	try { tdata.at(input.type.num); }
	catch (std::out_of_range oor) { return false; }
	return true;
}

bool tokenLeftIsNumber(token &input) {
	try { return tdata.at(input.type.num).leftNumber; }
	catch (std::out_of_range oor) { return true; }
}
bool tokenLeftIsOperator(token &input) {
	return !(tokenLeftIsNumber(input));
}
bool tokenRightIsNumber(token &input) {
	try { return tdata.at(input.type.num).rightNumber; }
	catch (std::out_of_range oor) { return true; }
}
bool tokenRightIsOperator(token &input) {
	return !(tokenRightIsNumber(input));
}

token tokenInsertLeft(token &input) {
	try { return tdata.at(input.type.num).insertLeft; }
	catch (std::out_of_range oor) { return token{TOKEN_NUMBER, 1}; }

}
token tokenInsertRight(token &input) {
	try { return tdata.at(input.type.num).insertRight; }
	catch (std::out_of_range oor) { return token{TOKEN_NUMBER, 1}; }
}

token tokenIdentity(token &input) {
	if (input.type == TOKEN_NUMBER) { return input; }
	try { return tdata.at(input.type.num).identity; }
	catch (std::out_of_range oor) { return token{TOKEN_NUMBER, 1}; }
}
token tokenIdentity(const token &input) {
	if (input.type.num == ttype(TOKEN_NUMBER).num) { return input; }
	try { return tdata.at(input.type.num).identity; }
	catch (std::out_of_range oor) { return token{TOKEN_NUMBER, 1}; }
}

bool tokenNumeric(token &input) {
	if (input.type == TOKEN_NUMBER) { return true; }
	try { return tdata.at(input.type.num).identity.type == TOKEN_NUMBER; }
	catch (std::out_of_range oor) { return true; }
}
bool tokenNumeric(const token &input) {
	if (input.type.num == ttype(TOKEN_NUMBER).num) { return true; }
	try { return tdata.at(input.type.num).identity.type == TOKEN_NUMBER; }
	catch (std::out_of_range oor) { return true; }
}

void tdList() {
	for (std::pair<int, TokenData> p : tdata) {
		std::cout << p.first << "\t";
		TokenData td = p.second;
		std::cout << td.leftNumber << ", " << td.rightNumber;
		std::cout << ", token{" << td.insertLeft.type.str << ", " << td.insertLeft.value << "}";
		std::cout << ", token{" << td.insertRight.type.str << ", " << td.insertRight.value << "}";
		std::cout << ", token{" << td.identity.type.str << ", " << td.identity.value << "}\n";
	}
}
