/*
 * Plug-in HSV color model.
 *
 * Copyright 0000
 *
 * A plug-in color model that enables picking colors using
 * hue, saturation, and value color components.
 */
#ifndef HSVCOLORMODEL_H
#define HSVCOLORMODEL_H

#include <lxsdk/lxlog.h>
#include <lxsdk/lxu_command.hpp>
#include <lxsdk/lxu_log.hpp>
#include <lxsdk/lx_color.hpp>
#include <lxsdk/lx_image.hpp>
#include <lxsdk/lx_persist.hpp>
#include <lxsdk/lx_thread.hpp>
#include <lxsdk/lx_value.hpp>
#include <lxsdk/lx_wrap.hpp>

#include <vector>

extern const char* SERVER_HSV_COLOR_MODEL;

class CHSVColorModelLog : public CLxLuxologyLogMessage
{
    public:
			 CHSVColorModelLog ()
				 : CLxLuxologyLogMessage (SERVER_HSV_COLOR_MODEL) { }

	const char *	 GetFormat  () { return "HSV Color Model"; }
};

/*
 * ---------------------------------------------------------------------------
 * Commands.
 */

class CHueAsWheelHSVColorModelCommand : public CLxBasicCommand
{
    public:
			 CHueAsWheelHSVColorModelCommand ();
	virtual		~CHueAsWheelHSVColorModelCommand () {}

	int		 basic_CmdFlags () LXx_OVERRIDE;
	bool		 basic_Notifier (int index, std::string &name, std::string &args) LXx_OVERRIDE;
	bool		 basic_Enable (CLxUser_Message &msg) LXx_OVERRIDE;

	void		 cmd_Execute (unsigned int flags) LXx_OVERRIDE;
	LxResult         cmd_Query (unsigned int index, ILxUnknownID vaQuery) LXx_OVERRIDE;
};

class CWheelHSVColorModelCommand : public CLxBasicCommand
{
    public:
			 CWheelHSVColorModelCommand ();
	virtual		~CWheelHSVColorModelCommand () {}

	int		 basic_CmdFlags () LXx_OVERRIDE;
	bool		 basic_Notifier (int index, std::string &name, std::string &args) LXx_OVERRIDE;
	bool		 basic_Enable (CLxUser_Message &msg) LXx_OVERRIDE;

	void		 cmd_Execute (unsigned int flags) LXx_OVERRIDE;
	LxResult         cmd_Query (unsigned int index, ILxUnknownID vaQuery) LXx_OVERRIDE;

	using CLxDynamicAttributes::atrui_UIHints;	// to distinguish from the overloaded version in CLxImpl_AttributesUI

	void		 atrui_UIHints2 (unsigned int index, CLxUser_UIHints &hints) LXx_OVERRIDE;
};

class CRuleHSVColorModelCommand : public CLxBasicCommand
{
    public:
			 CRuleHSVColorModelCommand ();
	virtual		~CRuleHSVColorModelCommand () {}

	int		 basic_CmdFlags () LXx_OVERRIDE;
	bool		 basic_Notifier (int index, std::string &name, std::string &args) LXx_OVERRIDE;
	bool		 basic_Enable (CLxUser_Message &msg) LXx_OVERRIDE;

	void		 cmd_Execute (unsigned int flags) LXx_OVERRIDE;
	LxResult         cmd_Query (unsigned int index, ILxUnknownID vaQuery) LXx_OVERRIDE;

	using CLxDynamicAttributes::atrui_UIHints;	// to distinguish from the overloaded version in CLxImpl_AttributesUI

	void		 atrui_UIHints2 (unsigned int index, CLxUser_UIHints &hints) LXx_OVERRIDE;
};

class CRuleAdjustHSVColorModelCommand : public CLxBasicCommand
{
    public:
			 CRuleAdjustHSVColorModelCommand ();
	virtual		~CRuleAdjustHSVColorModelCommand () {}

	int		 basic_CmdFlags () LXx_OVERRIDE;
	bool		 basic_Notifier (int index, std::string &name, std::string &args) LXx_OVERRIDE;
	bool		 basic_Enable (CLxUser_Message &msg) LXx_OVERRIDE;

	void		 cmd_Execute (unsigned int flags) LXx_OVERRIDE;
	LxResult         cmd_Query (unsigned int index, ILxUnknownID vaQuery) LXx_OVERRIDE;

	using CLxDynamicAttributes::atrui_UIHints;	// to distinguish from the overloaded version in CLxImpl_AttributesUI

	void		 atrui_UIHints2 (unsigned int index, CLxUser_UIHints &hints) LXx_OVERRIDE;
};

class CLevelsHSVColorModelCommand : public CLxBasicCommand
{
    public:
			 CLevelsHSVColorModelCommand ();
	virtual		~CLevelsHSVColorModelCommand () {}

	int		 basic_CmdFlags () LXx_OVERRIDE;
	bool		 basic_Notifier (int index, std::string &name, std::string &args) LXx_OVERRIDE;
	bool		 basic_Enable (CLxUser_Message &msg) LXx_OVERRIDE;

	void		 cmd_Execute (unsigned int flags) LXx_OVERRIDE;
	LxResult         cmd_Query (unsigned int index, ILxUnknownID vaQuery) LXx_OVERRIDE;

	using CLxDynamicAttributes::atrui_UIHints;	// to distinguish from the overloaded version in CLxImpl_AttributesUI

	void		 atrui_UIHints2 (unsigned int index, CLxUser_UIHints &hints) LXx_OVERRIDE;
};

class CSaveAsPresetsHSVColorModelCommand : public CLxBasicCommand
{
    public:
			 CSaveAsPresetsHSVColorModelCommand ();
	virtual		~CSaveAsPresetsHSVColorModelCommand () {}

	int		 basic_CmdFlags () LXx_OVERRIDE;
	bool		 basic_Notifier (int index, std::string &name, std::string &args) LXx_OVERRIDE;
	bool		 basic_Enable (CLxUser_Message &msg) LXx_OVERRIDE;

	void		 cmd_Execute (unsigned int flags) LXx_OVERRIDE;
	LxResult         cmd_Query (unsigned int index, ILxUnknownID vaQuery) LXx_OVERRIDE;

	using CLxDynamicAttributes::atrui_UIHints;	// to distinguish from the overloaded version in CLxImpl_AttributesUI

	void		 atrui_UIHints2 (unsigned int index, CLxUser_UIHints &hints) LXx_OVERRIDE;
};

/*
 * Notifier responds to changes executed by commands.
 */
class CHSVColorModelNotifier : public CLxCommandNotifier
{
    public:
			 CHSVColorModelNotifier ();
	virtual		~CHSVColorModelNotifier () {}

	virtual void		cn_Parse (std::string &args);
	virtual unsigned int	cn_Flags (int code);
};

/*
 * ---------------------------------------------------------------------------
 * CHSVColorModel.
 */

/*
 * Wheel types
 */
enum {
	HSV_WHEEL_RGB,			// Red, green, and blue is triadic.
	HSV_WHEEL_RYB			// Red, yellow, and blue is triadic.
};

/* 
 * Rules for building swatches.
 */
enum {
	HSV_RULE_SOLO,			// Just the selected color.
	HSV_RULE_COMPLEMENTARY,		// Selected color and its opposite.
	HSV_RULE_ANALOGOUS,		// Adjacent colors.
	HSV_RULE_TRIADIC,		// Three colors at 120 degree stops.
	HSV_RULE_TETRADIC,		// Four colors at 45 degree stops.
	HSV_RULE_COMPOUND,		// Five colors at 30 and 45 degree stops.
	HSV_RULE_TINTS,			// Tinted variations.
	HSV_RULE_SHADES			// Shaded variations.
};

class HSVColor
{
    public:
	LXtFVector	 vec;
};

class CHSVColorModelThreadRangeWorker :
	public CLxImpl_ThreadRangeWorker
{
    public:
	CHSVColorModelThreadRangeWorker ();
	~CHSVColorModelThreadRangeWorker ();

	virtual LxResult	 rngw_Execute (
					int		 index,
					void		*sharedData) LXx_OVERRIDE;
};

class CHSVColorModel :
	public CLxImpl_ColorModel
{
	CHSVColorModelLog	 m_log;
	ILxUnknownID		 rangeWorker;

    public:
	static LXtTagInfoDesc	 descInfo[];
	CLxPolymorph<CHSVColorModelThreadRangeWorker>	 range_factory;

				 CHSVColorModel ();
	virtual			~CHSVColorModel ();

	virtual int		 colm_NumComponents (void) LXx_OVERRIDE;

	virtual LxResult	 colm_ComponentType (
					unsigned	 componentIndex,
					const char	**type) LXx_OVERRIDE;

	virtual LxResult	 colm_ComponentRange (
					unsigned	 componentIndex,
					float		*minValue,
					float		*maxValue) LXx_OVERRIDE;

	virtual LxResult	 colm_ToRGB (
					const float	*hsv,
					float		*rgb) LXx_OVERRIDE;

	virtual LxResult	 colm_FromRGB (
					const float	*rgb,
					float		*hsv) LXx_OVERRIDE;

	virtual LxResult	 colm_DrawSlice (
					ILxUnknownID	 image,
					unsigned	 xAxis,
					unsigned	 yAxis,
					const float	*vec) LXx_OVERRIDE;

	virtual LxResult	 colm_DrawSliceMarker (
					ILxUnknownID	 image,
					unsigned	 xAxis,
					unsigned	 yAxis,
					const float	*downVec,
					const float	*hsv) LXx_OVERRIDE;

	virtual LxResult	 colm_CanSliceBeReused (
					unsigned	  xAxis,
					unsigned	  yAxis,
					const float	 *oldVec,
					const float	 *newVec) LXx_OVERRIDE;

	virtual LxResult	 colm_ToSlicePos (
					unsigned	 xAxis,
					unsigned	 yAxis,
					unsigned	 imgW,
					unsigned	 imgH,
					const float	*hsv,
					unsigned	*imgX,
					unsigned	*imgY) LXx_OVERRIDE;

	virtual LxResult	 colm_FromSlicePos (
					unsigned	 xAxis,
					unsigned	 yAxis,
					unsigned	 imgW,
					unsigned	 imgH,
					unsigned	 imgX,
					unsigned	 imgY,
					float		*downVec,
					float		*hsv) LXx_OVERRIDE;

	virtual LxResult	 colm_StripBaseVector (
					unsigned	 axis,
					int		 dynamic,
					float		*hsv) LXx_OVERRIDE;

	static bool		 HueAxisOnStrip ();
	static bool		 HueAsWheel ();
	static unsigned		 Wheel ();

	static unsigned		 Rule ();
	static float		 RuleAdjust ();
	static float		 PresetRuleAdjust ();

	static unsigned		 Levels ();

	static unsigned		 SwatchCount ();

	static void		 BuildSwatches (
					const HSVColor		&baseColor,
					std::vector<HSVColor>	&swatchColors,
					unsigned		&baseColorIndex);

    private:
	static void		 BuildComplementarySwatches (
					const HSVColor		&baseColor,
					std::vector<HSVColor>	&swatchColors,
					unsigned		&baseColorIndex);

	static void		 BuildAnalogousSwatches (
					const HSVColor		&baseColor,
					std::vector<HSVColor>	&swatchColors,
					unsigned		&baseColorIndex);

	static void		 BuildTriadicSwatches (
					const HSVColor		&baseColor,
					std::vector<HSVColor>	&swatchColors,
					unsigned		&baseColorIndex);

	static void		 BuildTetradicSwatches (
					const HSVColor		&baseColor,
					std::vector<HSVColor>	&swatchColors,
					unsigned		&baseColorIndex);

	static void		 BuildCompoundSwatches (
					const HSVColor		&baseColor,
					std::vector<HSVColor>	&swatchColors,
					unsigned		&baseColorIndex);

	static void		 BuildTintsSwatches (
					const HSVColor		&baseColor,
					std::vector<HSVColor>	&swatchColors,
					unsigned		&baseColorIndex);

	static void		 BuildShadesSwatches (
					const HSVColor		&baseColor,
					std::vector<HSVColor>	&swatchColors,
					unsigned		&baseColorIndex);

	CLxUser_Image		 primary_marker;
	CLxUser_Image		 secondary_marker;
};

#endif // HSVCOLORMODEL_H
