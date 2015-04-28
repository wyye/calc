#include "HelpTools.h"

void calc_irrecoverable_error(std::string message, std::string file, int line)
{
	char line_str[20];
	sprintf(line_str, "%d", line);
	std::string full_msg = "Error: '" + message + "' at file '" + file + "' line " + line_str;
	throw std::logic_error(full_msg);
}
