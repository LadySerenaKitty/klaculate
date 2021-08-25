#include "klaculate.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <regex>

// from: https://thispointer.com/find-and-replace-all-occurrences-of-a-sub-string-in-c/

void findAndReplaceAll(std::string& data, std::string toSearch, std::string replaceStr) {
	// Get the first occurrence
	size_t pos = data.find(toSearch);
	// Repeat till end is reached
	while (pos != std::string::npos) {
		// Replace this occurrence of Sub String
		data.replace(pos, toSearch.size(), replaceStr);
		// Get the next occurrence from the current position
		pos = data.find(toSearch, pos + replaceStr.size());
	}
}

void findAndReplaceAny(std::string& data, std::string toSearch, std::string replaceStr) {
	std::stringstream ss;
	ss << "([" << toSearch << "]+)";
	std::regex fixer(ss.str());
	std::string tmp = std::regex_replace(data, fixer, replaceStr);
	data = tmp;
}
