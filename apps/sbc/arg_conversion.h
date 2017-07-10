#ifndef _ARG_CONVERSION_H_
#define _ARG_CONVERSION_H_

#include "AmArg.h"

std::string arg2username(const AmArg& a);
bool username2arg(const std::string& src, AmArg& dst);

#endif
