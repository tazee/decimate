#pragma once

// Log Messages
//
//   Copyright 0000

#include <lxsdk/lxu_log.hpp>

#include <string>

/*
 * Custom log messages.
 */
class OBJLogMessage : public CLxLuxologyLogMessage
{
    public:
	virtual const char *	GetFormat ()
	{
		return "Wavefront Object";
	}

    private:
	std::string		authoringTool;
};
