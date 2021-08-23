#include "OutputHelper.h"

#include <limits>
#include <regex>
#include <fstream>
#include <sstream>

#include <sys/stat.h>

namespace util {

	void replaceAll(std::string& str, const std::string& from, const std::string& to) {
		if (from.empty()) {
			return;
		}
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}
	}

	std::string html_escape(const std::string& s) {
		std::string ret;
		ret.reserve(s.size()*2);
		for (const auto ch : s) {
			switch (ch) {
				case '&':
					ret.append({'&', 'a', 'm', 'p', ';'});
					break;
				case '\"':
					ret.append({'&', 'q', 'u', 'o', 't', ';'});
					break;
				case '\'':
					ret.append({'&', 'a', 'p', 'o', 's', ';'});
					break;
				default:
					ret.append(1, ch);
					break;
			}
		}
		return ret;
	}

	OutputHelper::OutputHelper() {
		parent = nullptr;
		addValue("KLACULATE_NAME", KLACULATE_NAME);
		addValue("KLACULATE_VERSION", KLACULATE_VERSION);
		addValue("MATH_SIZE", std::to_string(sizeof(double)));
		addValue("MATH_MIN", std::to_string(std::numeric_limits<double>::min()));
		addValue("MATH_MAX", std::to_string(std::numeric_limits<double>::max()));
	}

	OutputHelper::OutputHelper(const OutputHelper& orig) {
	}

	void OutputHelper::addValue(const char *name, const char *value) {
		addValue(std::string(name), std::string(value));
	}

	void OutputHelper::addValue(std::string name, std::string value) {
		try {
			values.erase(name);
		} catch (const std::out_of_range& oor) {
		}

		values.insert(std::pair<std::string, std::string>(name, value));
	}

	void OutputHelper::addValues(std::map<std::string, std::string> *vlist) {
		for (std::pair<std::string, std::string> p : *vlist) {
			addValue(p.first, p.second);
		}
	}

	void OutputHelper::deleteValue(const char* name) {
		deleteValue(std::string(name));
	}

	void OutputHelper::deleteValue(std::string name) {
		try {
			values.erase(name);
		} catch (const std::out_of_range& oor) {
		}
	}

	void OutputHelper::deleteValues(std::vector<std::string>* vlist) {
		for (std::string s : *vlist) {
			deleteValue(s);
		}
	}

	void OutputHelper::clearValues() {
		values.clear();
	}

	void OutputHelper::addTemplate(const char* tplfile) {
		addTemplate(std::string(tplfile));
	}

	void OutputHelper::addTemplate(std::string tplfile) {
		int a = templates.size();
		helpers.push_back(nullptr);
		templates.push_back(tplfile);

		int b = stager.size();
		stager.push_back(b);
	}

	void OutputHelper::addHelper(OutputHelper* helper) {
		signed int a = (signed int) helpers.size();
		helpers.push_back(helper);
		templates.push_back("blank.html");

		a++;
		a *= -1;
		int b = stager.size();
		stager.push_back(a);
	}

	bool OutputHelper::existValue(std::string key) {
		std::map<std::string, std::string>::iterator it = values.find(key);
		return !(it == std::end(values));
	}

	std::string OutputHelper::getValue(std::string key) {
		return existValue(key) ? values.at(key) : std::string();
	}

	std::string OutputHelper::getOutput() {
		std::stringstream output;

		std::map<std::string, std::string> mdata;
		if (parent != nullptr) {
			for (std::pair<std::string, std::string>p : parent->values) {
				mdata.insert(p);
			}
		} // */
		for (std::pair<std::string, std::string>p : values) {
			mdata.insert(p);
		}

		signed int c = 0;
		int max = stager.size();
		for (int a = 0; a < max; ++a) {
			c = stager[a];
			if (c < 0) { // helper
				c *= -1;
				c--;
				output << helpers[c]->getOutput();
			}
			else { // template file
				std::string tplfile(templates[c]);

				// check cache
				std::string filename("/usr/home/klaculate/templates/");
				filename.append(tplfile);
				struct stat fs;
				long tm = 0;
				int result = stat(filename.c_str(), &fs);
				if (result == 0) {
					tm = fs.st_mtim.tv_sec;
				}

				tCacheData* test = nullptr;
				if (!templateCache.empty()) {
					try {
						test = templateCache.at(tplfile);
					} catch (std::out_of_range oor) {
					}
				}

				if (test == nullptr) { // not cached
					tcMutex.lock();
					tCacheData* tc = new tCacheData{tm, loadCacheData(tplfile)};
					templateCache.insert(std::pair<std::string, tCacheData*>(tplfile, tc));
					tcMutex.unlock();
				}
				else { // cached
					tCacheData* tc;
					tc = templateCache.at(tplfile);
					if (tc->tcTime < tm) { // file was updated
						tcMutex.lock();
						tc->tcData = loadCacheData(tplfile);
						tc->tcTime = tm;
						tcMutex.unlock();
					}
				}

				// pre-checks comprete
				tCacheData* tc = nullptr;
				tc = templateCache.at(tplfile);
				int prev = 0;
				int fnd = tc->tcData.find("{");
				if (fnd == std::string::npos) {
					output << tc->tcData;
				}
				else {
					std::stringstream tout;
					while (fnd != std::string::npos) {
						tout << tc->tcData.substr(prev, (fnd - prev));
						prev = tc->tcData.find("}", fnd);
						fnd++;
						try {
							tout << mdata.at(tc->tcData.substr(fnd, (prev - fnd)));
						} catch (...) {
						}
						prev++;
						fnd = tc->tcData.find("{", prev);
					}
					tout << tc->tcData.substr(prev);
					output << tout.str();
				}
			}
		}
		return output.str();
	}

	OutputHelper::~OutputHelper() {
		clearValues();
		templates.clear();
		helpers.clear();
		stager.clear();
	}

	void OutputHelper::setParent(OutputHelper *p) {
		parent = p;
	}

	std::string OutputHelper::loadCacheData(std::string fname) {
		std::string filename("/usr/home/klaculate/templates/");
		filename.append(fname);
		std::ifstream fin(filename);
		std::stringstream data;
		std::string pt;

		while (fin.good()) {
			std::getline(fin, pt, '\n');
			data << pt << "\n";
			pt.clear();
		}
		fin.close();
		std::string out = data.str();
		return out;
	}
}
