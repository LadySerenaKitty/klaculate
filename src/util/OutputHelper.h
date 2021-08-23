#ifndef util_OutputHelper_h
#define util_OutputHelper_h

#include <mutex>
#include <vector>

#include "../klaculate.h"

namespace util {

	struct tCacheData {
		long tcTime;
		std::string tcData;
	};
	static std::map<std::string, tCacheData*> templateCache;
	static std::mutex tcMutex;

	class OutputHelper {
	public:
		OutputHelper();
		OutputHelper(const OutputHelper& orig);

		void addValue(const char* name, const char* value);
		void addValue(std::string name, std::string value);
		void addValues(std::map<std::string, std::string>* vlist);
		void deleteValue(const char* name);
		void deleteValue(std::string name);
		void deleteValues(std::vector<std::string>* vlist);
		void clearValues();

		void addTemplate(const char* tplfile);
		void addTemplate(std::string tplfile);
		void addHelper(OutputHelper* helper);

		bool existValue(std::string key);
		std::string getValue(std::string key);
		std::string getOutput();

		virtual ~OutputHelper();
	private:
		void setParent(OutputHelper* p);
		std::string loadCacheData(std::string fname);
		OutputHelper* parent;
		std::map<std::string, std::string> values;
		std::vector<std::string> templates;
		std::vector<OutputHelper*> helpers;
		std::vector<signed int> stager;
	};
}

#endif /* util_OutputHelper_h */

