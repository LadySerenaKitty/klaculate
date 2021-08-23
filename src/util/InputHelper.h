#ifndef util_InputHelper_h
#define util_InputHelper_h

#include "../klaculate.h"

#include <vector>
#include <fcgiapp.h>

namespace util {

	class InputHelper {
	public:
		InputHelper(FCGX_Stream* inputs, char** headers);
		InputHelper(const InputHelper& orig);
		virtual ~InputHelper();

		std::string getHeader(std::string in);
		std::string getCookie(std::string in);
		std::string getGetter(std::string in);
		std::string getFormer(std::string in);
		std::string getPath(int in);

		std::map<std::string, std::string>* getHeaderList();
		std::map<std::string, std::string>* getCookieList();
		std::map<std::string, std::string>* getGetterList();
		std::map<std::string, std::string>* getFormerList();
		std::vector<std::string>* getPathList();

		bool existHeader(std::string in);
		bool existCookie(std::string in);
		bool existGetter(std::string in);
		bool existFormer(std::string in);
		bool existPath(int);

		bool emptyHeader(std::string in);
		bool emptyCookie(std::string in);
		bool emptyGetter(std::string in);
		bool emptyFormer(std::string in);
		bool emptyPath(int);

	private:
		std::string getStringer(std::map<std::string, std::string>* mp, std::string in);
		bool existString(std::map<std::string, std::string>* mp, std::string in);
		bool emptyString(std::map<std::string, std::string>* mp, std::string in);
		std::string getStringer(std::map<int, std::string>* mp, int in);
		bool existString(std::map<int, std::string>* mp, int in);
		bool emptyString(std::map<int, std::string>* mp, int in);

		std::map<std::string, std::string> headers;
		std::map<std::string, std::string> cookies;
		std::map<std::string, std::string> gets;
		std::map<std::string, std::string> form;
		std::vector<std::string> path;
	};
}

#endif /* util_InputHelper_h */
