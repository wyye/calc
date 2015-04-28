#ifndef HELPTOOLS_H
#define HELPTOOLS_H

#include <cstdio>
#include <stdexcept>
#include <string>

#define calc_unreachable(error_message) calc_irrecoverable_error(error_message, __FILE__, __LINE__)

void calc_irrecoverable_error(std::string message, std::string file, int line);

#endif // HELPTOOLS_H
