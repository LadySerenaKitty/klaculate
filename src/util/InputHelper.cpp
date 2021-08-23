#include "InputHelper.h"

#include <regex>
#include <stdexcept>
#include <list>

namespace util {

	std::string urlStrip(std::string input) {
		if (input.empty()) {
			return "";
		}
		else if ((input.find('%', 0) == std::string::npos) && (input.find('+', 0) == std::string::npos)) {
			return input;
		}

		std::string out;
		out.append(input);
		if (input.find('+', 0) != std::string::npos) {
			findAndReplaceAll(out, "+", " ");
		}
		if (input.find('%', 0) != std::string::npos) {
			std::regex re("%([0-9a-fA-F]{2})");
			std::smatch match;
			std::string chr;
			char pt;
			while (std::regex_search(out, match, re)) {
				pt = (char) std::stoi(match[1], nullptr, 16);
				chr = pt;
				findAndReplaceAll(out, match[0].str(), chr);
			}
		}
		return out;
	}

	InputHelper::InputHelper(FCGX_Stream* inputs, char** heads) {
		int a = 0;
		std::regex re("([^=]+)=(.+)?");
		while (heads[a] != NULL) {
			std::smatch match;
			std::string s(heads[a]);
			std::regex_search(s, match, re);
			headers.insert(std::pair<std::string, std::string>(urlStrip(match[1]), urlStrip(match[2])));
			a++;
		}

		if (!(headers.at("QUERY_STRING").empty())) {
			std::regex re("([^=]+)=?([^&]+)?&?");
			std::smatch match;
			std::string s = headers.at("QUERY_STRING");
			while (std::regex_search(s, match, re)) {
				gets.insert(std::pair<std::string, std::string>(urlStrip(match[1]), urlStrip(match[2])));
				s = match.suffix().str();
			}
		}

		try {
			if (!(headers.at("HTTP_COOKIE").empty())) {
				std::regex re("([^ =]+)=?([^&]+)?;?");
				std::smatch match;
				std::string s = headers.at("HTTP_COOKIE");
				while (std::regex_search(s, match, re)) {
					cookies.insert(std::pair<std::string, std::string>(urlStrip(match[1]), urlStrip(match[2])));
					s = match.suffix().str();
				}
			}
		} catch (...) {
		}

		if (!(headers.at("REQUEST_METHOD").empty()) && (headers.at("REQUEST_METHOD").compare("POST") == 0)) {
			std::string s;
			char p[1025];
			while (FCGX_GetLine(p, 1024, inputs)) {
				s.append(p);
			}
			std::regex re("([^=]+)=?([^&]+)?&?");
			std::smatch match;
			while (std::regex_search(s, match, re)) {
				form.insert(std::pair<std::string, std::string>(urlStrip(match[1]), urlStrip(match[2])));
				s = match.suffix().str();
			}
		}

		if (!(headers.at("DOCUMENT_URI").empty()) && (headers.at("DOCUMENT_URI").compare("/") != 0)) {
			std::regex re("([^/]+)");
			std::smatch match;
			std::string s = headers.at("DOCUMENT_URI");
			int a = 0;
			while (std::regex_search(s, match, re)) {
				path.push_back(urlStrip(match[1]));
				s = match.suffix().str();
				a++;
			}
		}
	}

	InputHelper::InputHelper(const InputHelper& orig) {
	}

	std::string InputHelper::getHeader(std::string in) {
		return getStringer(&headers, in);
	}

	std::string InputHelper::getCookie(std::string in) {
		return getStringer(&cookies, in);
	}

	std::string InputHelper::getGetter(std::string in) {
		return getStringer(&gets, in);
	}

	std::string InputHelper::getFormer(std::string in) {
		return getStringer(&form, in);
	}

	std::string InputHelper::getPath(int in) {
		if (emptyPath(in)) {
			return "";
		}
		return path[in];
	}

	std::map<std::string, std::string>* InputHelper::getHeaderList() {
		return &headers;
	}

	std::map<std::string, std::string>* InputHelper::getCookieList() {
		return &cookies;
	}

	std::map<std::string, std::string>* InputHelper::getGetterList() {
		return &gets;
	}

	std::map<std::string, std::string>* InputHelper::getFormerList() {
		return &form;
	}

	std::vector<std::string>* InputHelper::getPathList() {
		return &path;
	}

	bool InputHelper::existHeader(std::string in) {
		return existString(&headers, in);
	}

	bool InputHelper::existCookie(std::string in) {
		return existString(&cookies, in);
	}

	bool InputHelper::existGetter(std::string in) {
		return existString(&gets, in);
	}

	bool InputHelper::existFormer(std::string in) {
		return existString(&form, in);
	}

	bool InputHelper::existPath(int in) {
		return (in < path.size());
	}

	bool InputHelper::emptyHeader(std::string in) {
		return emptyString(&headers, in);
	}

	bool InputHelper::emptyCookie(std::string in) {
		return emptyString(&cookies, in);
	}

	bool InputHelper::emptyGetter(std::string in) {
		return emptyString(&gets, in);
	}

	bool InputHelper::emptyFormer(std::string in) {
		return emptyString(&form, in);
	}

	bool InputHelper::emptyPath(int in) {
		if (!existPath(in)) {
			return true;
		}
		return path[in].empty();
	}

	InputHelper::~InputHelper() {
		if (!headers.empty()) {
			headers.clear();
		}
		if (!cookies.empty()) {
			cookies.clear();
		}
		if (!gets.empty()) {
			gets.clear();
		}
		if (!form.empty()) {
			form.clear();
		}
		if (!path.empty()) {
			path.clear();
		}
	}

	std::string InputHelper::getStringer(std::map<std::string, std::string>* mp, std::string in) {
		return existString(mp, in) ? mp->at(in) : std::string();
	}

	bool InputHelper::existString(std::map<std::string, std::string>* mp, std::string in) {
		std::map<std::string, std::string>::iterator it = mp->find(in);
		return !(it == std::end(*mp));
	}

	bool InputHelper::emptyString(std::map<std::string, std::string>* mp, std::string in) {
		return (existString(mp, in) ? (mp->at(in).empty()) : true);
	}

	std::string InputHelper::getStringer(std::map<int, std::string>* mp, int in) {
		return existString(mp, in) ? mp->at(in) : std::string();
	}

	bool InputHelper::existString(std::map<int, std::string>* mp, int in) {
		std::map<int, std::string>::iterator it = mp->find(in);
		return !(it == std::end(*mp));
	}

	bool InputHelper::emptyString(std::map<int, std::string>* mp, int in) {
		return (existString(mp, in) ? (mp->at(in).empty()) : true);
	}
}
