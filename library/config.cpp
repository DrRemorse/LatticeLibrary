#include <library/config.hpp>

#include <fstream>
#include <library/log.hpp>

namespace library
{
	Config::Config(const std::string& file)
	{
		load(file);
	}

	// load from file
	bool Config::load(const std::string& filename)
	{
		std::ifstream stream(filename.c_str());
		if (!stream) return false; // could not open file

    std::string text((std::istreambuf_iterator<char>(stream)),
                     std::istreambuf_iterator<char>());
    size_t it = 0;

		// read one line at a time until end of file
    while (it < text.size())
		{
			std::string key, value;
			bool keypart = true;
      bool reading = false;
      char last = ' ';

			// iterate config line (string)
      while (it < text.size())
			{
        const char c = text[it];
        if (c == 13 || c == 10) break;

        // stop at space + comment
				if (c == '#' && last == ' ') break;
				if (c == '=')
				{
					keypart = false;
        }
        else if (c == '"') {
          reading = !reading;
        }
				else if (c != ' ' || reading)
				{
					if(keypart)
						key += c;
					else
						value += c;
				}
        last = c;
        it++;
			}
      // read until end of line
      while (it < text.size() && text[it] != 13 && text[it] != 10)
        it++;
      // read past end of line
      while (it < text.size() && (text[it] == 13 || text[it] == 10))
        it++;
      // add new variable
			if (!key.empty())
			{
				kv.emplace(std::piecewise_construct,
                   std::forward_as_tuple(key),
                   std::forward_as_tuple(value));
			}
		}
		logger << Log::INFO << "*** Loaded " << kv.size() << " variables from config." << Log::ENDL;
		return true;

	} // Config::load()
}
