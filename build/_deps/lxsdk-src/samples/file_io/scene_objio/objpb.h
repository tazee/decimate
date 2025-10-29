#pragma once

/*
 * OBJ Preset Browser
 *
 *  Copyright 0000
 */

#include <lxsdk/lx_image.hpp>
#include <lxsdk/lx_pretype.hpp>

class ObjPresetMetrics;
class ObjPresetType : public CLxImpl_PresetType {
    public:
	static LXtTagInfoDesc			 descInfo[];
	static CLxPolymorph<ObjPresetMetrics>	 metricFactory;

	static void				Initialize ();

	virtual LxResult	ptyp_Recognize			(const char *path, const char **category);
	virtual LxResult	ptyp_Do				(const char *path);
	virtual LxResult	ptyp_Metrics			(const char *path, int flags, int w, int h, ILxUnknownID prevMetrics, void **ppvObj);
	virtual LxResult	ptyp_GenericThumbnailResource	(const char *path, const char **resourceName);
};

class ObjPresetMetrics : public CLxImpl_PresetMetrics {
public:
	ObjPresetMetrics () : metadata (NULL), w (0), h (0) {}

	~ObjPresetMetrics () {
		if (metadata)
			lx::ObjRelease(metadata);
	}

	virtual LxResult	pmet_Flags		(int *flags);
	virtual LxResult	pmet_Metadata		(void **ppvObj);
	virtual LxResult	pmet_Markup		(void **ppvObj);
	virtual LxResult	pmet_ThumbnailImage	(void **ppvObj);
	virtual LxResult	pmet_ThumbnailIdealSize	(int *idealW, int *idealH);

	CLxUser_Image		 image;
	void			*metadata;
	int			 w, h;
};
