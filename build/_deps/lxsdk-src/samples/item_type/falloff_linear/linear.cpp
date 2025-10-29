/*
 * LINEAR.CPP		Linear Falloff for Deformations
 *
 *	Copyright 0000
 */
#include <lxsdk/lx_item.hpp>
#include <lxsdk/lx_package.hpp>
#include <lxsdk/lx_plugin.hpp>
#include <lxsdk/lx_filter.hpp>
#include <lxsdk/lx_visitor.hpp>
#include <lxsdk/lx_value.hpp>
#include <lxsdk/lx_vmodel.hpp>
#include <lxsdk/lx_draw.hpp>
#include <lxsdk/lx_locator.hpp>
#include <lxsdk/lxu_deform.hpp>
#include <lxsdk/lxu_package.hpp>
#include <lxsdk/lxu_modifier.hpp>
#include <lxsdk/lxu_matrix.hpp>
#include <lxsdk/lxidef.h>
#include <string>
#include <math.h>

using namespace lx_err;


	namespace Falloff_Linear {	// disambiguate everything with a namespace

#define SRVNAME_ITEMTYPE		"falloff.linear"	// unique within item types
#define SRVNAME_MODIFIER		"falloff.linear"	// unique within modifiers

	#if 1

/*
 * ----------------------------------------------------------------
 * Channels
 *
 * The CChannels class both describes the channels for our item in the
 * attribute description object, but also serves as a container for
 * specific channel values. There are also a couple of methods for
 * computing common values from channel values.
 */
#define Cs_RANGESETUP		"useSetup"
#define Cs_RANGE		"range"
#define Cs_MODE			"decayMode"
#define Cs_AXIS			"linearAxis"
#define Cs_FLIP			"flip"
#define Cs_MIRROR		"mirror"

#define ESHPi_LINEAR	 0
#define ESHPi_EASE_IN	 1
#define ESHPi_EASE_OUT	 2
#define ESHPi_SMOOTH	 3

static LXtTextValueHint hint_Mode[] = {
	ESHPi_LINEAR,		"linear",
	ESHPi_EASE_IN,		"easeIn",
	ESHPi_EASE_OUT,		"easeOut",
	ESHPi_SMOOTH,		"smooth",
	-1,			"=linear-decay-type",
	-1,			 0
};

class CChannels :
		public CLxChannels
{
    public:
	bool		 cv_rangeSetup;
	double		 cv_range;
	int		 cv_mode;
	int		 cv_axis;
 #ifdef HAS_FLIP
	bool		 cv_flip;
 #else
	static const bool cv_flip = false;
 #endif
	bool		 cv_mirror;

		void
	init_chan (
		CLxAttributeDesc	&desc)		LXx_OVERRIDE
	{

		CChannels		*chan = 0;

		desc.add_channel (Cs_RANGESETUP, LXsTYPE_BOOLEAN, false, &chan->cv_rangeSetup, LXfECHAN_READ);

		desc.add_channel (Cs_RANGE, LXsTYPE_DISTANCE, 1.0, &chan->cv_range, LXfECHAN_READ);
		desc.set_min (0.0);

		desc.add_channel (Cs_MODE, LXsTYPE_INTEGER, ESHPi_LINEAR, &chan->cv_mode, LXfECHAN_READ);
		desc.hint (hint_Mode);

		desc.add_channel (Cs_AXIS, LXsTYPE_AXIS, 2, &chan->cv_axis, LXfECHAN_READ);

 #ifdef HAS_FLIP
		desc.add_channel (Cs_FLIP, LXsTYPE_BOOLEAN, false, &chan->cv_flip, LXfECHAN_READ);
 #endif

		desc.add_channel (Cs_MIRROR, LXsTYPE_BOOLEAN, false, &chan->cv_mirror, LXfECHAN_READ);
	}

		void
	clamp_ranges ()
	{
		cv_axis = LXxCLAMP (cv_axis, 0, 2);
	}
};

static CLxMeta_Channels<CChannels>		chan_meta;


/*
 * ----------------------------------------------------------------
 * Falloff
 *
 * A subclass of the Channels object so that it has the state of the falloff,
 * the falloff object is an exported COM object implementing the ILxFalloff
 * methods. It also has a transform and ease that need to be initialized.
 */
class CFalloff :
		public CChannels,
		public CLxFalloff
{
    public:
	bool			 cv_invert;
	CLxEaseFraction		 m_ease;

		void
	set_ease ()
	{
		if (cv_mode == ESHPi_LINEAR)
			m_ease.set_shape (LXiESHP_LINEAR);
		else if (cv_mode == ESHPi_EASE_IN)
			m_ease.set_shape (LXiESHP_EASE_IN);
		else if (cv_mode == ESHPi_EASE_OUT)
			m_ease.set_shape (LXiESHP_EASE_OUT);
		else if (cv_mode == ESHPi_SMOOTH)
			m_ease.set_shape (LXiESHP_SMOOTH);

		m_ease.flags = (cv_flip ? LXfEASE_NEGATIVE : 0) | (cv_mirror ? LXfEASE_DOUBLE : 0);
	}

		float
	weight_local (
		const float		*pos,
		const LXtPointID	 point,
		const LXtPolygonID	 polygon)	LXx_OVERRIDE
	{
		return 1.0 - m_ease.evaluate (pos[cv_axis] / cv_range);
	}

		void
	draw (
		CLxUser_StrokeDraw	&stroke,
		int			 selFlags,
		const LXtVector		 itemColor)
	{
		CLxVector		 rad, v;
		double			 alpha;
		int			 ix, iy, iz;

		ix = (cv_axis + 1) % 3;
		iy = (cv_axis + 2) % 3;
		iz =  cv_axis;
		alpha = (selFlags & LXiSELECTION_SELECTED) ? 1.0 : 0.5;

		rad.clear ();
		rad[iz] = cv_invert ? 0.0 : cv_range;

		v.clear ();
		v[iz] = cv_invert ? cv_range : 0.0;
		stroke.BeginW (LXiSTROKE_LINE_STRIP, itemColor, alpha, 2 * alpha);
		v[ix] = cv_range;
		v[iy] = cv_range;
		stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
		v[iy] = -cv_range;
		stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
		v[ix] = -cv_range;
		stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
		v[iy] = cv_range;
		stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
		v[ix] = cv_range;
		stroke.Vertex (v, LXiSTROKE_ABSOLUTE);

		v.clear ();
		stroke.BeginW (LXiSTROKE_LINES, itemColor, alpha, 2 * alpha);
		stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
		stroke.Vertex (rad, LXiSTROKE_RELATIVE);
		if (cv_mirror)
		{
			stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
			stroke.Vertex (rad * -1.0, LXiSTROKE_RELATIVE);
		}

		v[cv_axis] = cv_invert ? cv_range : 0;
		stroke.BeginWD (LXiSTROKE_CIRCLES, itemColor, alpha, 1.0, LXiLPAT_DOTSLONG);
		stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
		stroke.Vertex (rad, LXiSTROKE_RELATIVE);

		if (selFlags & LXiSELECTION_SELECTED)
		{
			double			 w, d, x, pw, px;

			for (int i = 1; i <= 10; i++)
			{
				d = i / 10.0;
				x = cv_range * d;
				if (cv_invert)
					d = 1.0 - d;

				w = 1.0 - m_ease.evaluate (d);
				rad[cv_axis] = cv_range * w;
				v[cv_axis] = x;
				stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
				stroke.Vertex (rad, LXiSTROKE_RELATIVE);
				if (cv_mirror)
				{
					v[cv_axis] = -x;
					stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
					stroke.Vertex (rad, LXiSTROKE_RELATIVE);
				}
			}

			v.clear ();
			rad.clear ();
			pw = cv_invert ? 0 : 1;
			px = 0;
			stroke.Begin (LXiSTROKE_LINES, itemColor, alpha);
			for (int i = 1; i <= 10; i++)
			{
				d = i / 10.0;
				x = cv_range * d;
				if (cv_invert)
					d = 1.0 - d;

				w = 1.0 - m_ease.evaluate (d);

				rad[iy] = 0;
				rad[ix] = cv_range * pw;
				rad[iz] = px;
				v[iy] = 0;
				v[ix] = cv_range * w;
				v[iz] = x;
				stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
				stroke.Vertex (rad, LXiSTROKE_ABSOLUTE);
				if (cv_mirror)
				{
					v[iz] = -x;
					rad[iz] = -px;
					stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
					stroke.Vertex (rad, LXiSTROKE_ABSOLUTE);
				}

				rad[ix] = -cv_range * pw;
				rad[iy] = 0;
				rad[iz] = px;
				v[ix] = -cv_range * w;
				v[iy] = 0;
				v[iz] = x;
				stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
				stroke.Vertex (rad, LXiSTROKE_ABSOLUTE);
				if (cv_mirror)
				{
					v[iz] = -x;
					rad[iz] = -px;
					stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
					stroke.Vertex (rad, LXiSTROKE_ABSOLUTE);
				}

				rad[ix] = 0;
				rad[iy] = cv_range * pw;
				rad[iz] = px;
				v[ix] = 0;
				v[iy] = cv_range * w;
				v[iz] = x;
				stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
				stroke.Vertex (rad, LXiSTROKE_ABSOLUTE);
				if (cv_mirror)
				{
					v[iz] = -x;
					rad[iz] = -px;
					stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
					stroke.Vertex (rad, LXiSTROKE_ABSOLUTE);
				}

				rad[ix] = 0;
				rad[iy] = -cv_range * pw;
				rad[iz] = px;
				v[ix] = 0;
				v[iy] = -cv_range * w;
				v[iz] = x;
				stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
				stroke.Vertex (rad, LXiSTROKE_ABSOLUTE);
				if (cv_mirror)
				{
					v[iz] = -x;
					rad[iz] = -px;
					stroke.Vertex (v, LXiSTROKE_ABSOLUTE);
					stroke.Vertex (rad, LXiSTROKE_ABSOLUTE);
				}

				pw = w;
				px = x;
			}
		}
	}
};


/*
 * ----------------------------------------------------------------
 * Package (Item Type)
 */
class CViewItem3D :
		public CLxViewItem3D
{
    public:
		void
	draw (
		CLxUser_Item		&item,
		CLxUser_ChannelRead	&chan,
		CLxUser_StrokeDraw	&stroke,
		int			 selFlags,
		const CLxVector		&color)		LXx_OVERRIDE
	{
		CFalloff		 fall;

		chan_meta->chan_read (chan, item, (CChannels *) &fall);
		fall.cv_invert = chan.IValue (item, LXsICHAN_FALLOFF_INVERT);

		fall.clamp_ranges ();
		fall.set_ease ();
		fall.draw (stroke, selFlags, color);
	}
};


/*
 * ----------------------------------------------------------------
 * Modifier
 */
class CModifier :
		public CLxEvalModifier,
		public CLxTypeEvaluation<CFalloff>
{
    public:
		void
	bind (
		CLxUser_Item		&item,
		unsigned		 index)		LXx_OVERRIDE
	{
		mod_add_chan (item, LXsICHAN_FALLOFF_INVERT);
		mod_add_chan (item, LXsICHAN_XFRMCORE_WORLDMATRIX);
	}

		void
	init_obj (
		CLxEvalModifier		&com,
		CFalloff		&fall)		LXx_OVERRIDE
	{
		CLxMatrix4		 xfrm;

		com.mod_read_attr ((CChannels *) &fall);
		com.mod_cust_val (0, &fall.cv_invert);

		if (fall.cv_rangeSetup)
			com.mod_eval() -> SetAlternateSetup ();

		com.mod_cust_val (1, xfrm);

		fall.world_inverse = xfrm.inverse ();
		fall.clamp_ranges ();
		fall.set_ease ();
	}
};


/*
 * ----------------------------------------------------------------
 * Metaclasses
 *
 *	+ root
 *	    + channels
 *	    + package
 *	        + view3D
 *	    + modifier
 *	        + type eval
 *	            + falloff
 */
static CLxMetaRoot				root_meta;

static CLxMeta_Package<CLxPackage>		pkg_meta (SRVNAME_ITEMTYPE);
static CLxMeta_ViewItem3D<CViewItem3D>		v3d_meta;

static CLxMeta_EvalModifier<CModifier>		mod_meta (SRVNAME_MODIFIER);
static CLxMeta_TypeEvaluation<CModifier, CFalloff>
						eval_meta (LXsICHAN_FALLOFF_FALLOFF);
static CLxMeta_Falloff<CFalloff>		fall_meta;


	void
initialize ()
{
	root_meta.add (&chan_meta);
	root_meta.add (&pkg_meta);
	root_meta.add (&mod_meta);

	pkg_meta.set_supertype (LXsITYPE_FALLOFF);
	pkg_meta.add (&v3d_meta);

	mod_meta.add  (&eval_meta);
	eval_meta.add (&fall_meta);
	fall_meta.set_local ();
}

	#else

#define SPWNAME_INSTANCE		"falloff.linear.inst"	// unique within spawners in this module
#define SPWNAME_FALLOFF			"falloff.linear"	// unique within spawners in this module


/*
 * Class Declarations
 *
 * These have to come before their implementions because they reference each
 * other. Descriptions are attached to the implementations.
 */
class CPackage;

class CInstance :
		public CLxImpl_PackageInstance,
		public CLxImpl_ViewItem3D
{
    public:
	CPackage	*src_pkg;
	CLxUser_Item	 m_item;

	LxResult	 pins_Initialize (ILxUnknownID item, ILxUnknownID super)	LXx_OVERRIDE;
	void		 pins_Cleanup (void)						LXx_OVERRIDE;

	LxResult	 vitm_Draw (
				ILxUnknownID	 itemChanRead,
				ILxUnknownID	 viewStrokeDraw,
				int		 selectionFlags,
				const LXtVector	 itemColor)				LXx_OVERRIDE;

};

class CPackage :
		public CLxImpl_Package
{
    public:
	static LXtTagInfoDesc	 descInfo[];
	CLxSpawner<CInstance>	 spawn;

	CPackage () : spawn (SPWNAME_INSTANCE) {}

	LxResult		pkg_SetupChannels (ILxUnknownID addChan) LXx_OVERRIDE;
	LxResult		pkg_TestInterface (const LXtGUID *guid) LXx_OVERRIDE;
	LxResult		pkg_Attach (void **ppvObj) LXx_OVERRIDE;
};


/*
 * ----------------------------------------------------------------
 * Package Class
 *
 * Packages implement item types, or simple item extensions. They are
 * like the metatype object for the item type. They define the common
 * set of channels for the item type and spawn new instances.
 */
LXtTagInfoDesc	 CPackage::descInfo[] = {
	{ LXsPKG_SUPERTYPE,	LXsITYPE_FALLOFF	},
	{ 0 }
};

/*
 * The package has a set of standard channels with default values. These
 * are setup at the start using the AddChannel interface.
 */
#define Cs_RANGESETUP		"useSetup"
#define Cs_RANGE		"range"
#define Cs_MODE			"decayMode" // type of falloff
#define Cs_AXIS			"linearAxis"
#define Cs_FLIP			"flip"
#define Cs_MIRROR		"mirror"

enum {
	RF_LINEAR = 0,
	RF_EASE_IN,
	RF_EASE_OUT,
	RF_SMOOTH
};

#define RF_DECAY_LAST RF_SMOOTH
#define EASEf_NEGATIVE	 1
#define EASEf_DOUBLE	 2

#define ESHPi_LINEAR	 RF_LINEAR
#define ESHPi_EASE_IN	 RF_EASE_IN
#define ESHPi_EASE_OUT	 RF_EASE_OUT
#define ESHPi_SMOOTH	 RF_SMOOTH

class EaseFraction 
{
    public:
	double		 p0;
	double		 p1;
	int		 flags;

	EaseFraction() {p0=0; p1=0; flags=0;};

		template <class F>
		void
	Hermite (
		F			 t,
		F			 h[4]) {
		F			 t2, t3, z;

		t2 = t * t;
		t3 = t * t2;
		z = 3.0 * t2 - t3 - t3;

		h[0] = 1.0 - z;
		h[1] = z;
		h[2] = t3 - t2 - t2 + t;
		h[3] = t3 - t2;
	}



		void
	SetShape (
		int			 shape)
	{
		int			 sp0[] = { 1, 2, 0, 0 };
		int			 sp1[] = { 1, 0, 2, 0 };

		if ((shape >= 0) && (shape < 4)) {
			p0 = sp0[shape];
			p1 = sp1[shape];
		}
	}


		template <class F>
		F
	Evaluate (
		F			x) 
	{
		F			 hc[4];

		if (flags & EASEf_DOUBLE) {
			x = LXxABS (x);
			//x *= 2.0;
			//if (x > 1.0)
			//	x = 2.0 - x;
		}

		if (flags & EASEf_NEGATIVE)
			x = 1.0 - x;

		if (x >= 1.0)
			return 1.0;
		else if (x <= 0.0)
			return 0.0;

		Hermite (x, hc);
		return hc[1] + p0 * hc[2] + p1 * hc[3];
	}

};




static LXtTextValueHint hint_Mode[] = {
	RF_LINEAR,		"linear",
	RF_EASE_IN,		"easeIn",
	RF_EASE_OUT,		"easeOut",
	RF_SMOOTH,		"smooth",
	-1,			"=linear-decay-type",
	-1,			 0
};


	LxResult
CPackage::pkg_SetupChannels (
	ILxUnknownID		 addChan)
{
	CLxUser_AddChannel	 ac (addChan);

	ac.NewChannel  (Cs_RANGESETUP,	LXsTYPE_BOOLEAN);
	ac.SetDefault  (0.0, 0);

	ac.NewChannel  (Cs_RANGE,	LXsTYPE_DISTANCE);
	ac.SetDefault  (1.0, 0);

	ac.NewChannel  (Cs_MODE,	LXsTYPE_INTEGER);
	ac.SetDefault  (0.0, RF_LINEAR);
	ac.SetHint     (hint_Mode);

	ac.NewChannel  (Cs_AXIS,	LXsTYPE_AXIS);
	ac.SetDefault  (0.0, 2);

	//ac.NewChannel  (Cs_FLIP,	LXsTYPE_BOOLEAN);
	//ac.SetDefault  (0.0, 0);

	ac.NewChannel  (Cs_MIRROR,	LXsTYPE_BOOLEAN);
	ac.SetDefault  (0.0, 0);
	return LXe_OK;
}
 
/*
 * TestInterface() is required so that nexus knows what interfaces instance
 * of this package support. Necessary to prevent query loops.
 */
	LxResult
CPackage::pkg_TestInterface (
	const LXtGUID		*guid)
{
	return spawn.TestInterfaceRC (guid);
}

/*
 * Attach is called to create a new instance of this item. The returned
 * object implements a specific item of this type in the scene.
 */
	LxResult
CPackage::pkg_Attach (
	void		       **ppvObj)
{
	CInstance		*inst = spawn.Alloc (ppvObj);

	inst->src_pkg = this;
	return LXe_OK;
}


/*
 * The instance is the implementation of the item, and there will be one
 * allocated for each item in the scene. It can respond to a set of
 * events.
 */
	LxResult
CInstance::pins_Initialize (
	ILxUnknownID		 item,
	ILxUnknownID		 super)
{
	m_item.set (item);
	return LXe_OK;
}

	void
CInstance::pins_Cleanup (void)
{
	m_item.clear ();
}


	LxResult
CInstance::vitm_Draw (
	ILxUnknownID		 itemChanRead,
	ILxUnknownID		 viewStrokeDraw,
	int			 selectionFlags,
	const LXtVector		 itemColor)
{
	CLxUser_ChannelRead	 chan (itemChanRead);
	CLxUser_StrokeDraw	 strokeDraw (viewStrokeDraw);
	LXtVector		 rad, v;
	CLxUser_ValueService	 vS;
	CLxUser_Value		 val;
	CLxUser_Matrix		 xfrm;
	double			 range, alpha;
	int			 ix=2, iy=0, axis, mode, mir, invrt;

	axis   = chan.IValue (m_item, Cs_AXIS);
	axis = LXxCLAMP (axis, 0, 2);
	ix = (axis+1)%3;
	iy = (axis+2)%3;
	range = chan.FValue (m_item, Cs_RANGE);
	mir   = chan.IValue (m_item, Cs_MIRROR);
	mode   = chan.IValue (m_item, Cs_MODE);
	invrt   = chan.IValue (m_item, "invert");
	alpha = (selectionFlags&LXiSELECTION_SELECTED) ? 1.0 : 0.5;

	LXx_VCLR (rad);
	rad[axis] = invrt ? 0 : range;

	LXx_VCLR (v); // draw box
	v[axis] = invrt ? range : 0;
	strokeDraw.BeginW (LXiSTROKE_LINE_STRIP, itemColor, alpha, 2*alpha);
	v[ix] = range;
	v[iy] = range;
	strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
	v[iy] = -range;
	strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
	v[ix] = -range;
	strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
	v[iy] = range;
	strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
	v[ix] = range;
	strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);

	LXx_VCLR (v);

	strokeDraw.BeginW (LXiSTROKE_LINES, itemColor, alpha, 2*alpha);
	strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
	strokeDraw.Vertex (rad, LXiSTROKE_RELATIVE);
	if (mir) {
		strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
		rad[axis] = -range;
		strokeDraw.Vertex (rad, LXiSTROKE_RELATIVE);
		rad[axis] = range;
	}
	v[axis] = invrt ? range : 0;
	strokeDraw.BeginWD (LXiSTROKE_CIRCLES, itemColor, alpha, 1.0, LXiLPAT_DOTSLONG);
	strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
	strokeDraw.Vertex (rad, LXiSTROKE_RELATIVE);
	if ((selectionFlags&LXiSELECTION_SELECTED)) {
		double			 w=0, pw, px, d, x, dx = range / 10;
		EaseFraction		 easer;

		easer = EaseFraction ();
		easer.SetShape (mode);
		easer.flags = (mir ? EASEf_DOUBLE : 0); //(flip ? EASEf_NEGATIVE : 0) |

		for (int i=1; i<=10; i++) {
			x = dx * i;
			d = x / range;
			if(invrt)
				d = 1.0 - d;

			w = 1.0 - easer.Evaluate (d);
			rad[axis] = range * w;
			v[axis] = x;
			strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
			strokeDraw.Vertex (rad, LXiSTROKE_RELATIVE);
			if (mir) {
				v[axis] = -x;
				strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
				strokeDraw.Vertex (rad, LXiSTROKE_RELATIVE);
			}
		}

		LXx_VCLR (v);
		LXx_VCLR (rad);
		pw = invrt ? 0 : 1;
		px = 0;
		strokeDraw.Begin (LXiSTROKE_LINES, itemColor, alpha);
		for (int i=1; i<=10; i++) {
			x = dx * i;
			d = x / range;
			if(invrt) {
				d = 1.0 - d;
			}
			w = 1.0 - easer.Evaluate (d);

			rad[iy] = 0;
			v[iy] = 0;
			rad[ix] = range * pw;
			rad[axis] = px;
			v[ix] = range * w;
			v[axis] = x;
			strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
			strokeDraw.Vertex (rad, LXiSTROKE_ABSOLUTE);
			if (mir) {
				v[axis] = -x;
				rad[axis] = -px;
				strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
				strokeDraw.Vertex (rad, LXiSTROKE_ABSOLUTE);
			}
			rad[ix] = -range * pw;
			rad[axis] = px;
			v[ix] = -range * w;
			v[axis] = x;
			strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
			strokeDraw.Vertex (rad, LXiSTROKE_ABSOLUTE);
			if (mir) {
				v[axis] = -x;
				rad[axis] = -px;
				strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
				strokeDraw.Vertex (rad, LXiSTROKE_ABSOLUTE);
			}

			rad[ix] = 0;
			v[ix] = 0;
			rad[iy] = range * pw;
			rad[axis] = px;
			v[iy] = range * w;
			v[axis] = x;
			strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
			strokeDraw.Vertex (rad, LXiSTROKE_ABSOLUTE);
			if (mir) {
				v[axis] = -x;
				rad[axis] = -px;
				strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
				strokeDraw.Vertex (rad, LXiSTROKE_ABSOLUTE);
			}
			rad[iy] = -range * pw;
			rad[axis] = px;
			v[iy] = -range * w;
			v[axis] = x;
			strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
			strokeDraw.Vertex (rad, LXiSTROKE_ABSOLUTE);
			if (mir) {
				v[axis] = -x;
				rad[axis] = -px;
				strokeDraw.Vertex (v, LXiSTROKE_ABSOLUTE);
				strokeDraw.Vertex (rad, LXiSTROKE_ABSOLUTE);
			}

			pw = w;
			px = x;
		}

	}


	//if(mode==RF_DECAY_SMOOTH) {
	//	if ((selectionFlags&LXiSELECTION_SELECTED) && (core>0)) {
	//	}
	//}
	//else {
	//	if ((selectionFlags&LXiSELECTION_SELECTED)) {
	//	}
	//}

	return LXe_OK;
}

class CFalloff :
		public CLxImpl_Falloff
{
    public:
	CLxUser_Matrix		 scratch_mat;
	CLxUser_Value		 scratch_val;
	LXtMatrix4		 w_xfrm; 
	LXtMatrix4		 inv_w_xfrm; 
	double			 core, range;
	int			 mode;
	int			 axis;
	int			 flip;
	int			 mirror;

		template <class F>
		F
	CalcWeight1D (
		F			 x) 
	{
		F			 w=0, d;
		EaseFraction		 easer;

		easer = EaseFraction ();

		d = x / range;
		easer.SetShape (mode);
		easer.flags = (flip ? EASEf_NEGATIVE : 0) | (mirror ? EASEf_DOUBLE : 0);

		w = easer.Evaluate (d);
		return 1.0 - w;
	}


		template <class F>
		F
	GetWeight (
		const F			*pos,
		const LXtPointID	 point,
		const LXtPolygonID	 polygon)
	{
		F			 x[3];

		LXtVector		p, wp;
		scratch_mat.Set4 (inv_w_xfrm);  // use inverse world matrix to transform 
		LXx_VCPY (wp, pos);		// world position to local coordinates
		scratch_mat.MultiplyVector (wp, p);
		LXx_VCPY (x, p);
		//LXx_VSUB3 (x, pos, cen);

		return CalcWeight1D (x[axis]);
	}

	LXxO_Falloff_WeightF
	{
		return GetWeight (position, point, polygon);
	}
	
	LXxO_Falloff_WeightRun
	{
		return lx::WeightRun (this, pos, points, polygons, weight, num);
	}
};

/*
 * The modifier spawns the falloff from the input parameters.
 */
class CModifier :
		public CLxObjectRefModifierCore
{
    public:
	const char *	ItemType ()			LXx_OVERRIDE
	{
		return SRVNAME_ITEMTYPE;
	}

	const char *	Channel  ()			LXx_OVERRIDE
	{
		return LXsICHAN_FALLOFF_FALLOFF;
	}

		void
	Attach (
		CLxUser_Evaluation	&eval,
		ILxUnknownID		 item)		LXx_OVERRIDE
	{
		eval.AddChan (item, Cs_RANGE);
		eval.AddChan (item, Cs_MODE);
		eval.AddChan (item, Cs_AXIS);
		//eval.AddChan (item, Cs_FLIP);
		eval.AddChan (item, Cs_MIRROR);
		eval.AddChan (item, Cs_RANGESETUP);
		eval.AddChan (item, LXsICHAN_XFRMCORE_WORLDMATRIX, LXfECHAN_READ | LXfECHAN_SETUP);
	}

		void
	Alloc (
		CLxUser_Evaluation	&eval,
		CLxUser_Attributes	&attr,
		unsigned		 index,
		ILxUnknownID		&obj)		LXx_OVERRIDE
	{
		CLxSpawner<CFalloff>	 sp (SPWNAME_FALLOFF);
		CLxUser_ValueService	 vS;
		CFalloff		*fall;
		CLxUser_Matrix		 xfrm;
		fall = sp.Alloc (obj);
		
		fall->range = attr.Float (index++);
		fall->range = LXxMAX (fall->range, 1E-12);
		fall->mode   = attr.Int (index++);
		fall->mode = LXxCLAMP (fall->mode, 0, RF_DECAY_LAST);
		fall->axis   = attr.Int (index++);
		fall->axis = LXxCLAMP (fall->axis, 0, 2);
		fall->flip = 0;
		//fall->flip = attr.Bool (index++);
		fall->mirror = attr.Bool (index++);
		if (attr.Bool (index++))
			eval.SetAlternateSetup ();

		attr.ObjectRO (index++, xfrm);
		xfrm.Get4 (fall->w_xfrm);
		vS.NewValue (fall->scratch_val, LXsTYPE_MATRIX4);
		fall->scratch_mat.set (fall->scratch_val); 
		fall->scratch_mat.Set4 (fall->w_xfrm); // use mutable matrix for invert!!!
		fall->scratch_mat.Invert ();
		fall->scratch_mat.Get4 (fall->inv_w_xfrm);
	}
};


/*
 * Export package server to define a new item type. Also create and destroy
 * the factories so they can persist while their objects are in use.
 */
	void
initialize ()
{
	CLxGenericPolymorph		*srv;

	srv = new CLxPolymorph<CPackage>;
	srv->AddInterface (new CLxIfc_Package   <CPackage>);
	srv->AddInterface (new CLxIfc_StaticDesc<CPackage>);
	lx::AddServer (SRVNAME_ITEMTYPE, srv);

	srv = new CLxPolymorph<CInstance>;
	srv->AddInterface (new CLxIfc_PackageInstance<CInstance>);
	srv->AddInterface (new CLxIfc_ViewItem3D     <CInstance>);
	lx::AddSpawner (SPWNAME_INSTANCE, srv);

	CLxSpawner_Falloff<CFalloff> (SPWNAME_FALLOFF);

	CLxExport_ItemModifierServer<CLxObjectRefModifier<CModifier> > (SRVNAME_MODIFIER);
}

	#endif

	};	// END namespace

