#include "TokenData.h"

std::map<int, TokenData> tdata;
void tdInit() {
	tdata.insert(std::pair<int, TokenData>(ttype('(').num, TokenData{true, false, token{'*', 0}, token{'#', 1}}));
	tdata.insert(std::pair<int, TokenData>(ttype(')').num, TokenData{false, true, token{'#', 1}, token{'*', 0}}));
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
