/*
 * INITIALIZER.CPP	Shader entry points.
 *
 *	Copyright 0000
 */
#include <lxsdk/lx_plugin.hpp>

namespace Halftone_Shader {
	extern void	initialize ();
	extern void	cleanup ();
};


	void
initialize ()
{
	Halftone_Shader     ::initialize ();
}

	void
cleanup ()
{
//	Halftone_Shader     ::cleanup ();
}

