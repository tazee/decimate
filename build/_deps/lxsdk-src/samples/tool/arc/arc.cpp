/*
 * ARC.CPP  Plug-in Interactive Primitive Creation Tool.
 *
 *	Copyright 0000
 */
#include <lxsdk/lx_tool.hpp>
#include <lxsdk/lx_toolui.hpp>
#include <lxsdk/lx_vmodel.hpp>
#include <lxsdk/lx_vector.hpp>
#include <lxsdk/lxu_attributes.hpp>
#include <lxsdk/lx_plugin.hpp>
#include <lxsdk/lx_layer.hpp>
#include <lxsdk/lx_mesh.hpp>
#include <lxsdk/lx_log.hpp>
#include <lxsdk/lx_draw.hpp>
#include <lxsdk/lx_value.hpp>
#include <lxsdk/lx_handles.hpp>
#include <lxsdk/lx_message.hpp>
#include <lxsdk/lxu_math.hpp>
#include <lxsdk/lxvp.h>
#include <lxsdk/lx_select.hpp>
#include <lxsdk/lx_seltypes.hpp>
#include <lxsdk/lx_package.hpp>
#include <lxsdk/lxu_select.hpp>


class CArcTool :
		public CLxImpl_Tool,
		public CLxImpl_ToolModel,
		public CLxImpl_Attributes,
		public CLxImpl_AttributesUI
{
    public:
			CArcTool ();

	void		tool_Reset      () LXx_OVERRIDE;
	LXtObjectID	tool_VectorType () LXx_OVERRIDE;
	const char *	tool_Order      () LXx_OVERRIDE;
	LXtID4		tool_Task       () LXx_OVERRIDE;
	void		tool_Evaluate   (ILxUnknownID vts) LXx_OVERRIDE;

	unsigned	tmod_Flags      () LXx_OVERRIDE;
	void		tmod_Initialize (ILxUnknownID vts, ILxUnknownID adjust, unsigned flags) LXx_OVERRIDE;
	const char *	tmod_Haul       (unsigned index) LXx_OVERRIDE;
	void		tmod_Draw       (ILxUnknownID vts, ILxUnknownID stroke, int flags) LXx_OVERRIDE;
	void		tmod_Test       (ILxUnknownID vts, ILxUnknownID stroke, int flags) LXx_OVERRIDE;
	LxResult	tmod_Down       (ILxUnknownID vts, ILxUnknownID adjust) LXx_OVERRIDE;
	void		tmod_Move       (ILxUnknownID vts, ILxUnknownID adjust) LXx_OVERRIDE;
	void		tmod_Up         (ILxUnknownID vts, ILxUnknownID adjust) LXx_OVERRIDE;
	LxResult	tmod_TestType   (LXtID4 type) LXx_OVERRIDE;
	LxResult	tmod_Enable     (ILxUnknownID msg) LXx_OVERRIDE;
	LxResult	tmod_AllowOverride (const char *name, unsigned buttonIdx, unsigned axis) LXx_OVERRIDE;

	unsigned int	attr_Count	() LXx_OVERRIDE;
	LxResult	attr_Name       (unsigned int index, const char **name) LXx_OVERRIDE;
	LxResult	attr_Lookup     (const char *name, unsigned int *index)  LXx_OVERRIDE;
	LxResult	attr_Type       (unsigned int index, unsigned int *type) LXx_OVERRIDE;
	LxResult	attr_TypeName   (unsigned int index, const char **tname) LXx_OVERRIDE;
	const LXtTextValueHint *
			attr_Hints      (unsigned int index) LXx_OVERRIDE;
	LxResult	attr_GetInt     (unsigned int index, int *val) LXx_OVERRIDE;
	LxResult	attr_SetInt     (unsigned int index, int val) LXx_OVERRIDE;
	LxResult	attr_GetFlt     (unsigned int index, double *val) LXx_OVERRIDE;
	LxResult	attr_SetFlt     (unsigned int index, double val) LXx_OVERRIDE;

	LxResult	atrui_UIHints   (unsigned int index, ILxUnknownID hints) LXx_OVERRIDE;

	void		DrawHandles (ILxUnknownID vts, ILxUnknownID stroke, int flags);
	void		SetRadius ();
	void		SetAngle ();
	void		SetRotHandles ();
	void		SetEndHandle ();
	void		GetPos (LXtVector pos, double t, double scale = 1.0);
	void		PlaneMatrix (LXtMatrix m);
	void		GetConstrain (ILxUnknownID vts, LXtVector pos);

	CLxUser_LayerService	 s_layer;
	CLxUser_VectorType	 v_type;
	CLxUser_Value		 val_dist, val_angl;
	CLxUser_StringConversionNice
				 str_dist, str_angl;

	CLxUser_SelectionService	 selSvc;
	CLxUser_ScenePacketTranslation	 scenePktTrans;
	LXtID4				 sceneCode;

	unsigned		 offset_view;
	unsigned		 offset_input;
	unsigned		 offset_event;
	unsigned		 offset_center;
	unsigned		 offset_axis;
	unsigned		 offset_xfrm;

	LXtVector		 m_center;		// Center Handle
	LXtVector		 m_start;		// Start Handle
	LXtVector		 m_end;			// End Handle
	double			 m_radius;		// Radius
	double			 m_angle;		// Angle
	int			 m_segments;		// Segments
	int			 m_reverse;		// Reverse;

	int			 m_part;		// Current part
	int			 m_init;		// Initial Drag

	LXtVector		 m_vecS;		// Initial start vector
	LXtVector		 m_vecE;		// Initial end vector

	LXtVector		 m_vecAxis;		// Initial axis vector

	CLxUser_Item		 m_primary;

	int			 m_lockAngle;
};

namespace Util {

		int
	Dnint (
		double			 val)
	{
		return (int) (val + (val > 0.0 ? 0.5 : -0.5));
	}

		void
	MatrixAxisRotation (
		LXtMatrix		 m,
		const LXtVector		 a,
		double			 s,
		double			 c)
	{
		double			 t;

		t = 1.0 - c;
		m[0][0] = t * a[0] * a[0] + c;
		m[0][1] = t * a[0] * a[1] - s * a[2];
		m[0][2] = t * a[0] * a[2] + s * a[1];
		m[1][0] = t * a[1] * a[0] + s * a[2];
		m[1][1] = t * a[1] * a[1] + c;
		m[1][2] = t * a[1] * a[2] - s * a[0];
		m[2][0] = t * a[2] * a[0] - s * a[1];
		m[2][1] = t * a[2] * a[1] + s * a[0];
		m[2][2] = t * a[2] * a[2] + c;
	}

		double
	AngleVectors (
		const LXtVector		 v0,
		const LXtVector		 v1)
	{
		double			 vlen0, vlen1, x;

		vlen0 = LXx_VLEN (v0);
		vlen1 = LXx_VLEN (v1);
		if (vlen0 < lx::Tolerance (vlen0) || vlen1 < lx::Tolerance (vlen1))
			return 0.0;
		x = LXx_VDOT (v0, v1) / vlen0 / vlen1;
		x = LXxCLAMP (x, -1.0, 1.0);
		return acos (x);
	}
};

#define ATTRs_CENX		"centerX"
#define ATTRs_CENY		"centerY"
#define ATTRs_CENZ		"centerZ"
#define ATTRs_SRTX		"startX"
#define ATTRs_SRTY		"startY"
#define ATTRs_SRTZ		"startZ"
#define ATTRs_ENDX		"endX"
#define ATTRs_ENDY		"endY"
#define ATTRs_ENDZ		"endZ"
#define ATTRs_RADI		"radius"
#define ATTRs_ANGL		"angle"
#define ATTRs_SEGS		"segments"
#define ATTRs_REVS		"reverse"

#define ATTRa_CENX		0
#define ATTRa_CENY		1
#define ATTRa_CENZ		2
#define ATTRa_SRTX		3
#define ATTRa_SRTY		4
#define ATTRa_SRTZ		5
#define ATTRa_ENDX		6
#define ATTRa_ENDY		7
#define ATTRa_ENDZ		8
#define ATTRa_RADI		9
#define ATTRa_ANGL		10
#define ATTRa_SEGS		11
#define ATTRa_REVS		12
#define ATTRa_TOTAL		13

#define DEF_RADI		1.0
#define DEF_ANGL		LXx_HALFPI
#define DEF_SEGS		15
#define DEF_REVS		0

#define ARC_HANDLE_CENTER	10000
#define ARC_HANDLE_START	10001
#define ARC_HANDLE_END		10002

#define IQ_CONSTRAIN		0x08


/*
 * On create we allocate a vector type and look up some tool packet offsets. We
 * also allocate values and their string conversion interface for nice encoding.
 */
CArcTool::CArcTool ()
{
	CLxUser_PacketService	 sPkt;
	CLxUser_ValueService	 sVal;

	sPkt.NewVectorType (LXsCATEGORY_TOOL, v_type);
	sPkt.AddPacket (v_type, LXsP_TOOL_VIEW_EVENT,  LXfVT_GET);
	sPkt.AddPacket (v_type, LXsP_TOOL_INPUT_EVENT, LXfVT_GET);
	sPkt.AddPacket (v_type, LXsP_TOOL_EVENTTRANS,  LXfVT_GET);
	sPkt.AddPacket (v_type, LXsP_TOOL_ACTCENTER,   LXfVT_GET);
	sPkt.AddPacket (v_type, LXsP_TOOL_AXIS,        LXfVT_GET);
	sPkt.AddPacket (v_type, LXsP_TOOL_XFRM,        LXfVT_GET);

	offset_view   = sPkt.GetOffset (LXsCATEGORY_TOOL, LXsP_TOOL_VIEW_EVENT);
	offset_input  = sPkt.GetOffset (LXsCATEGORY_TOOL, LXsP_TOOL_INPUT_EVENT);
	offset_event  = sPkt.GetOffset (LXsCATEGORY_TOOL, LXsP_TOOL_EVENTTRANS);
	offset_center = sPkt.GetOffset (LXsCATEGORY_TOOL, LXsP_TOOL_ACTCENTER);
	offset_axis   = sPkt.GetOffset (LXsCATEGORY_TOOL, LXsP_TOOL_AXIS);
	offset_xfrm   = sPkt.GetOffset (LXsCATEGORY_TOOL, LXsP_TOOL_XFRM);

	sVal.NewValue (val_dist, LXsTYPE_DISTANCE);
	str_dist.set  (val_dist);

	sVal.NewValue (val_angl, LXsTYPE_ANGLE);
	str_angl.set  (val_angl);

	sceneCode = selSvc.LookupType (LXsSELTYP_SCENE);
	scenePktTrans.autoInit ();

	tool_Reset ();
}


/*
 * Reset sets the attributes back to defaults.
 */
	void
CArcTool::tool_Reset ()
{
	LXx_VCLR (m_center);
	LXx_VSET3 (m_start, DEF_RADI, 0.0, 0.0);
	LXx_VSET3 (m_end  , 0.0, DEF_RADI, 0.0);
	m_radius   = DEF_RADI;
	m_angle    = DEF_ANGL;
	m_segments = DEF_SEGS;
	m_reverse  = DEF_REVS;
}

/*
 * Boilerplate methods that identify this as an action (state altering) tool.
 */
	LXtObjectID
CArcTool::tool_VectorType ()
{
	return v_type.m_loc;
}

	const char *
CArcTool::tool_Order ()
{
	return LXs_ORD_ACTR;
}

	LXtID4
CArcTool::tool_Task ()
{
	return LXi_TASK_ACTR;
}


/*
 * We employ a combination of evant-based input and hauling. I0 input events
 * (left mouse button) are handled through event callbacks. I1 input events
 * (right mouse button) are treating as hauling, which affects the radius.
 */
	unsigned
CArcTool::tmod_Flags ()
{
	return LXfTMOD_I1_ATTRHAUL | LXfTMOD_DRAW_3D | LXfTMOD_I0_INPUT;
}

	const char *
CArcTool::tmod_Haul (
	unsigned		 index)
{
	if (index == 0)
		return ATTRs_RADI;
	else
		return 0;
}

	void
CArcTool::tmod_Initialize (
	ILxUnknownID		 vts,
	ILxUnknownID		 adjust,
	unsigned int		 flags)
{
	CLxUser_AdjustTool	 at (adjust);
	CLxUser_VectorStack	 vec (vts);
	LXpToolActionCenter	*acen;
	LXpToolAxis		*axis;

	acen = (LXpToolActionCenter *) vec.Read (offset_center);
	axis = (LXpToolAxis         *) vec.Read (offset_axis);

	/*
	 * Set the initial center position to the action center.
	 */
	LXx_VCPY (m_center, acen->v);
	LXx_VCPY (m_vecS, axis->right);
	LXx_VCPY (m_vecE, axis->up);
	LXx_VCPY (m_vecAxis, axis->axis);
	LXx_VADDS3 (m_start, acen->v, axis->right, m_radius);
	LXx_VADDS3 (m_end, acen->v, axis->up, m_radius);
	if (m_radius > 0.0)
		GetPos (m_end, 1.0, 1.0);

	m_init = 1;
}

	LxResult
CArcTool::tmod_Down (
	ILxUnknownID		 vts,
	ILxUnknownID		 adjust)
{
	CLxUser_AdjustTool	 at (adjust);
	CLxUser_VectorStack	 vec (vts);
	CLxUser_EventTranslatePacket
				 epkt;
	LXpToolInputEvent	*ipkt;


	ipkt = (LXpToolInputEvent *) vec.Read (offset_input);
	vec.ReadObject (offset_event, epkt);

	/*
	 * Test if the primary mesh is in the scene. (#56533)
	 */
	CLxSceneSelection	 scene_sel;
	CLxUser_Scene		 scene;
	unsigned		 i, count = 0;
	int			 flags;

	m_primary.clear ();
	scene_sel.Get (scene);
	if (scene.test ())
		s_layer.SetScene (scene);

	if (LXx_OK (s_layer.Count (&count))) {
		for (i = 0; i < count; i++) {
			if (LXx_OK (s_layer.Flags (i, &flags)) && (flags & LXf_LAYER_MAIN)) {
				s_layer.Item (i, m_primary);
				break;
			}
		}
	}

	/*
	 * The first drag.
	 */
	if (m_init) {
		epkt.HitHandle (vts, m_center);
		at.SetFlt (ATTRa_RADI, 1.0e-6);
		return LXe_TRUE;
	}

	/*
	 * Set the handle position if a handle is selected.
	 */
	m_part = ipkt->part;
	m_lockAngle = 0;

	switch (ipkt->part) {
	    case ARC_HANDLE_CENTER:
		epkt.HitHandle (vts, m_center);
		break;

	    case ARC_HANDLE_START:
		epkt.HitHandle (vts, m_start);
		break;

	    case ARC_HANDLE_END:
		epkt.HitHandle (vts, m_end);
		break;
	}

	return LXe_TRUE;
}

	void
CArcTool::tmod_Move (
	ILxUnknownID		 vts,
	ILxUnknownID		 adjust)
{
	CLxUser_AdjustTool	 at (adjust);
	CLxUser_VectorStack	 vec (vts);
	CLxUser_EventTranslatePacket
				 epkt;
	LXpToolInputEvent	*ipkt;
	LXtVector		 pos, delta, vec1;
	double			 scl;

	ipkt = (LXpToolInputEvent *) vec.Read (offset_input);
	vec.ReadObject (offset_event, epkt);

	/*
	 * The first drag.
	 */
	if (m_init) {
		epkt.ModelDelta (vts, m_center, delta);
		at.SetFlt (ATTRa_RADI, LXx_VLEN (delta));
		return;
	}

	if (ipkt->part < 0)
		return;

	/*
	 * Get the new handle position.
	 */
	epkt.GetNewPosition (vts, pos);

	switch (ipkt->part) {
	    case ARC_HANDLE_CENTER:
	    	/*
	    	 * Move the three handles together.
	    	 */
		LXx_VSUB3 (delta, pos, m_center);
		at.SetFlt (ATTRa_CENX, pos[0]);
		at.SetFlt (ATTRa_CENY, pos[1]);
		at.SetFlt (ATTRa_CENZ, pos[2]);
		LXx_VADD3 (pos, m_start, delta);
		at.SetFlt (ATTRa_SRTX, pos[0]);
		at.SetFlt (ATTRa_SRTY, pos[1]);
		at.SetFlt (ATTRa_SRTZ, pos[2]);
		LXx_VADD3 (pos, m_end, delta);
		at.SetFlt (ATTRa_ENDX, pos[0]);
		at.SetFlt (ATTRa_ENDY, pos[1]);
		at.SetFlt (ATTRa_ENDZ, pos[2]);
		break;

	    case ARC_HANDLE_START:
	    	/*
	    	 * Move the rotation handles together.
	    	 */
		at.SetFlt (ATTRa_SRTX, pos[0]);
		at.SetFlt (ATTRa_SRTY, pos[1]);
		at.SetFlt (ATTRa_SRTZ, pos[2]);
		if (m_radius > 0.0) {
			LXx_VCPY (m_start, pos);
			LXx_VSUB3 (vec1, m_end, m_center);
			scl = LXx_VLEN (vec1) / m_radius;
			GetPos (pos, 1.0, scl);
		}
		m_lockAngle = 1;
		at.SetFlt (ATTRa_ENDX, pos[0]);
		at.SetFlt (ATTRa_ENDY, pos[1]);
		at.SetFlt (ATTRa_ENDZ, pos[2]);
		break;

	    case ARC_HANDLE_END:
	    	/*
	    	 * Move the end handle alone.
	    	 */
	    	GetConstrain (vts, pos);
		at.SetFlt (ATTRa_ENDX, pos[0]);
		at.SetFlt (ATTRa_ENDY, pos[1]);
		at.SetFlt (ATTRa_ENDZ, pos[2]);
		break;
	}
}

	void
CArcTool::tmod_Up (
	ILxUnknownID		 vts,
	ILxUnknownID		 adjust)
{
	m_part = -1;
	m_init = 0;
	m_lockAngle = 0;
}

	void
CArcTool::GetConstrain (
	ILxUnknownID		 vts,
	LXtVector		 pos)
{
	CLxUser_VectorStack	 vec (vts);
	LXpToolInputEvent	*ipak = (LXpToolInputEvent *) vec.Read (offset_input);
	LXtVector		 v, x0, x1, axis;
	LXtMatrix		 m;
	double			 d, a;

	if (ipak->mode & IQ_CONSTRAIN) {
		LXx_VSUB3 (x0, m_start, m_center);
		LXx_VSUB3 (x1, pos, m_center);
		LXx_VCROSS (axis, x0, x1);
		d = LXx_VLEN (x1);
		a = Util::AngleVectors (x0, x1) * LXx_RAD2DEG;
		a = 15.0 * Util::Dnint (a / 15.0) * LXx_DEG2RAD;
		lx::VectorNormalize (axis);
		lx::MatrixIdent (m);
		lx::MatrixAxisRotation (m, axis, sin (a), cos (a));
		lx::MatrixMultiply (v, m, x0);
		lx::VectorNormalize (v);
		LXx_VADDS3 (pos, m_center, v, d);
		LXx_VSUB3 (x1, pos, m_center);
	}
}

	void
CArcTool::tmod_Draw (
	ILxUnknownID		 vts,
	ILxUnknownID		 stroke,
	int			 flags)
{
	DrawHandles (vts, stroke, flags);
}

	void
CArcTool::tmod_Test (
	ILxUnknownID		 vts,
	ILxUnknownID		 stroke,
	int			 flags)
{
	DrawHandles (vts, stroke, flags);
}

	LxResult
CArcTool::tmod_Enable (
	ILxUnknownID		 msg)
{
	CLxSceneSelection	 scene_sel;
	CLxUser_LayerService	 _lyr_svc;
	CLxUser_Message		 message (msg);
	CLxUser_Scene		 scene;
	CLxUser_Item		 primary;
	LxResult		 result = LXe_OK;
	unsigned		 i = 0, count = 0;
	int			 flags = 0, primary_index = -1;
	
	/*
	 *	Search the mesh layers for the primary mesh.
	 */
	
	scene_sel.Get (scene);
	
	if (scene.test ())
		_lyr_svc.SetScene (scene);

	if (LXx_OK (_lyr_svc.Count (&count)))
	{
		for (i = 0; i < count; i++)
		{
			if (LXx_OK (_lyr_svc.Flags (i, &flags)) && (flags & LXf_LAYER_MAIN))
			{
				_lyr_svc.Item (i, primary);
				primary_index = i;
				break;
			}
		}
	}
	
	/*
	 *	If there is no primary mesh, or the mesh is procedural,
	 *	the tool is disabled.
	 */
	
	if (!primary.test ())
	{
		if (message.test ())
		{
			message.SetCode (LXe_DISABLED);
			message.SetMsg ("prim.arc", "noMesh");
		}
		
		result = LXe_DISABLED;
	}
	else if (primary_index >= 0 && _lyr_svc.IsProcedural ((unsigned) primary_index) == LXe_TRUE)
	{
		if (message.test ())
		{
			message.SetCode (LXe_DISABLED);
			message.SetMsg ("prim.arc", "proceduralMesh");
		}
		
		result = LXe_DISABLED;
	}
	
	return result;
}

/* The arc tool hauls radius with the RMB.  Everything else is fair game */
	LxResult
CArcTool::tmod_AllowOverride (
	const char 		*attrName,
	unsigned		 mouseIdx,
	unsigned		 axisIdx)
{
	if (!strcmp (attrName, ATTRs_RADI) && mouseIdx == 1)
		return LXe_FALSE;
	
	return LXe_TRUE;
}

	void
CArcTool::DrawHandles (
	ILxUnknownID		 vts,
	ILxUnknownID		 stroke,
	int			 flags)
{
	CLxUser_VectorStack	 vec (vts);
	CLxUser_StrokeDraw	 draw (stroke);
	CLxUser_HandleDraw	 handle (stroke);
	LXtVector		 pos, color;
	LXtMatrix		 m;
	std::string		 label, num;
	double			 angle;
	int			 dFlags;

	LXpToolViewEvent	*view = (LXpToolViewEvent *) vec.Read (offset_view);
	if (!view || ((view->type != LXi_VIEWTYPE_3D) && (view->type != LXi_VIEWTYPE_2D)))
		return;

	if (!m_primary.test ())
		return;

	LXx_VSET3 (color, 0.8, 0.8, 0.8);
	PlaneMatrix (m);

	/*
	 * Draw the center handle.
	 */
	if (m_part == ARC_HANDLE_CENTER)
		dFlags = LXi_THANDf_SMALL | LXi_THANDf_HOT;
	else
		dFlags = LXi_THANDf_SMALL;

	handle.Handle (m_center, 0, ARC_HANDLE_CENTER, dFlags);

	/*
	 * Draw the start handle and the text at the start position.
	 */
	if (m_part == ARC_HANDLE_START)
		dFlags = LXi_THANDf_SMALL | LXi_THANDf_HOT;
	else
		dFlags = LXi_THANDf_SMALL;

	handle.RotateMouseHandle (m_center, m_start, m, 2, ARC_HANDLE_START, dFlags);

	CLxUser_MessageService   svMsg;
	CLxUser_Message          message;

	svMsg.NewMessage (message);
	message.SetMsg ("prim.arc", "rad");

	val_dist.SetFlt (m_radius);
	str_dist.EncodeStr (num);
	message.SetArg (1, num.c_str());

	svMsg.GetText (message, label);
	GetPos (pos, 0.0, 1.1);

	draw.SetPart (LXiHITPART_INVIS);
	draw.Begin   (LXiSTROKE_TEXT, color, 1.0);
	draw.Text    (label.c_str(), LXiTEXT_LEFT);
	draw.Vert    (pos);

	/*
	 * Draw the end handle and the text at the center of the arc.
	 */
	if (m_part == ARC_HANDLE_END)
		dFlags = LXi_THANDf_SMALL | LXi_THANDf_HOT;
	else
		dFlags = LXi_THANDf_SMALL;

	handle.RotateMouseHandle (m_center, m_end, m, 2, ARC_HANDLE_END, dFlags);

	svMsg.NewMessage (message);
	message.SetMsg ("prim.arc", "ang");

	angle = m_reverse ? (LXx_TWOPI - m_angle) : m_angle;
	val_angl.SetFlt (angle);
	str_angl.EncodeStr (num);
	message.SetArg (1, num.c_str());

	svMsg.GetText (message, label);
	GetPos (pos, 0.5, 1.1);

	draw.SetPart (LXiHITPART_INVIS);
	draw.Begin   (LXiSTROKE_TEXT, color, 1.0);
	draw.Text    (label.c_str(), LXiTEXT_LEFT);
	draw.Vert    (pos);
}

	LxResult
CArcTool::tmod_TestType (
	LXtID4			 type)
{
	switch (type) {
	    case LXi_VPSPACE_MODEL:
	    case LXi_VPSPACE_WORLD:
	    case LXi_VPSPACE_MODEL2D:
		return LXe_TRUE;
	}
	return LXe_FALSE;
}


/*
 * ATTRIBUTES Interface
 *
 * This class implements the attributes interface directly, which can be a bit
 * involved. It would be better to port to CLxDynamicAttributes.
 */
	unsigned int
CArcTool::attr_Count ()
{
	return ATTRa_TOTAL;
}

	LxResult
CArcTool::attr_Name (
	unsigned int		 index, 
	const char 		**name)
{
	switch (index) {
	    case ATTRa_CENX:
		name[0] = ATTRs_CENX;
		break;

	    case ATTRa_CENY:
		name[0] = ATTRs_CENY;
		break;

	    case ATTRa_CENZ:
		name[0] = ATTRs_CENZ;
		break;

	    case ATTRa_SRTX:
		name[0] = ATTRs_SRTX;
		break;

	    case ATTRa_SRTY:
		name[0] = ATTRs_SRTY;
		break;

	    case ATTRa_SRTZ:
		name[0] = ATTRs_SRTZ;
		break;

	    case ATTRa_ENDX:
		name[0] = ATTRs_ENDX;
		break;

	    case ATTRa_ENDY:
		name[0] = ATTRs_ENDY;
		break;

	    case ATTRa_ENDZ:
		name[0] = ATTRs_ENDZ;
		break;

	    case ATTRa_RADI:
		name[0] = ATTRs_RADI;
		break;

	    case ATTRa_ANGL:
		name[0] = ATTRs_ANGL;
		break;

	    case ATTRa_SEGS:
		name[0] = ATTRs_SEGS;
		break;

	    case ATTRa_REVS:
		name[0] = ATTRs_REVS;
		break;

	    default:
		return LXe_OUTOFBOUNDS;
	}
	return LXe_OK;
}

	LxResult
CArcTool::attr_Type (
	unsigned int		 index,
	unsigned int		*type)
{
	switch (index) {
	    case ATTRa_SEGS:
	    case ATTRa_REVS:
		type[0] = LXi_TYPE_INTEGER;
		break;

	    case ATTRa_CENX:
	    case ATTRa_CENY:
	    case ATTRa_CENZ:
	    case ATTRa_SRTX:
	    case ATTRa_SRTY:
	    case ATTRa_SRTZ:
	    case ATTRa_ENDX:
	    case ATTRa_ENDY:
	    case ATTRa_ENDZ:
	    case ATTRa_RADI:
	    case ATTRa_ANGL:
		type[0] = LXi_TYPE_FLOAT;
		break;

	    default:
		return LXe_OUTOFBOUNDS;
	}
	return LXe_OK;
}

	LxResult
CArcTool::attr_TypeName (
	unsigned int		 index,
	const char	       **type)
{
	switch (index) {
	    case ATTRa_SEGS:
		type[0] = LXsTYPE_INTEGER;
		break;

	    case ATTRa_REVS:
		type[0] = LXsTYPE_BOOLEAN;
		break;

	    case ATTRa_ANGL:
		type[0] = LXsTYPE_ANGLE;
		break;

	    case ATTRa_CENX:
	    case ATTRa_CENY:
	    case ATTRa_CENZ:
	    case ATTRa_SRTX:
	    case ATTRa_SRTY:
	    case ATTRa_SRTZ:
	    case ATTRa_ENDX:
	    case ATTRa_ENDY:
	    case ATTRa_ENDZ:
	    case ATTRa_RADI:
		type[0] = LXsTYPE_DISTANCE;
		break;

	    default:
		return LXe_OUTOFBOUNDS;
	}
	return LXe_OK;
}

	LxResult
CArcTool::attr_Lookup (
	const char		*name,
	unsigned int		*index)
{
	if (strcmp (name, ATTRs_CENX) == 0)
		index[0] = ATTRa_CENX;
	else if (strcmp (name, ATTRs_CENY) == 0)
		index[0] = ATTRa_CENY;
	else if (strcmp (name, ATTRs_CENZ) == 0)
		index[0] = ATTRa_CENZ;
	else if (strcmp (name, ATTRs_SRTX) == 0)
		index[0] = ATTRa_SRTX;
	else if (strcmp (name, ATTRs_SRTY) == 0)
		index[0] = ATTRa_SRTY;
	else if (strcmp (name, ATTRs_SRTZ) == 0)
		index[0] = ATTRa_SRTZ;
	else if (strcmp (name, ATTRs_ENDX) == 0)
		index[0] = ATTRa_ENDX;
	else if (strcmp (name, ATTRs_ENDY) == 0)
		index[0] = ATTRa_ENDY;
	else if (strcmp (name, ATTRs_ENDZ) == 0)
		index[0] = ATTRa_ENDZ;
	else if (strcmp (name, ATTRs_RADI) == 0)
		index[0] = ATTRa_RADI;
	else if (strcmp (name, ATTRs_ANGL) == 0)
		index[0] = ATTRa_ANGL;
	else if (strcmp (name, ATTRs_SEGS) == 0)
		index[0] = ATTRa_SEGS;
	else if (strcmp (name, ATTRs_REVS) == 0)
		index[0] = ATTRa_REVS;
	else
		return LXe_NOTFOUND;

	return LXe_OK;
}

	LxResult
CArcTool::attr_GetInt (
	unsigned int		 index,
	int			*val)
{
	switch (index) {
	    case ATTRa_SEGS:
		((int *) val)[0] = m_segments;
		break;

	    case ATTRa_REVS:
		((int *) val)[0] = m_reverse;
		break;

	    default:
		return LXe_OUTOFBOUNDS;
	}

	return LXe_OK;
}

	LxResult
CArcTool::attr_SetInt (
	unsigned int		 index,
	int			 val)
{
	switch (index) {
	    case ATTRa_SEGS:
		m_segments = val;
		break;

	    case ATTRa_REVS:
		m_reverse = val;
		break;

	    default:
		return LXe_OUTOFBOUNDS;
	}

	return LXe_OK;
}

	LxResult
CArcTool::attr_GetFlt (
	unsigned int		 index,
	double			*val)
{
	switch (index) {
	    case ATTRa_CENX:
		((double *) val)[0] = m_center[0];
		break;

	    case ATTRa_CENY:
		((double *) val)[0] = m_center[1];
		break;

	    case ATTRa_CENZ:
		((double *) val)[0] = m_center[2];
		break;

	    case ATTRa_SRTX:
		((double *) val)[0] = m_start[0];
		break;

	    case ATTRa_SRTY:
		((double *) val)[0] = m_start[1];
		break;

	    case ATTRa_SRTZ:
		((double *) val)[0] = m_start[2];
		break;

	    case ATTRa_ENDX:
		((double *) val)[0] = m_end[0];
		break;

	    case ATTRa_ENDY:
		((double *) val)[0] = m_end[1];
		break;

	    case ATTRa_ENDZ:
		((double *) val)[0] = m_end[2];
		break;

	    case ATTRa_RADI:
		((double *) val)[0] = m_radius;
		break;

	    case ATTRa_ANGL:
		((double *) val)[0] = m_angle;
		break;

	    default:
		return LXe_OUTOFBOUNDS;
	}

	return LXe_OK;
}

	LxResult
CArcTool::attr_SetFlt (
	unsigned int		 index,
	double			 val)
{
	switch (index) {
	    case ATTRa_CENX:
		m_center[0] = val;
		break;

	    case ATTRa_CENY:
		m_center[1] = val;
		break;

	    case ATTRa_CENZ:
		m_center[2] = val;
		break;

	    case ATTRa_SRTX:
		m_start[0] = val;
		SetRadius ();
		break;

	    case ATTRa_SRTY:
		m_start[1] = val;
		SetRadius ();
		break;

	    case ATTRa_SRTZ:
		m_start[2] = val;
		SetRadius ();
		break;

	    case ATTRa_ENDX:
		m_end[0] = val;
		SetAngle ();
		break;

	    case ATTRa_ENDY:
		m_end[1] = val;
		SetAngle ();
		break;

	    case ATTRa_ENDZ:
		m_end[2] = val;
		SetAngle ();
		break;

	    case ATTRa_RADI:
		m_radius = val;
		SetRotHandles ();
		break;

	    case ATTRa_ANGL:
		m_angle = val;
		SetEndHandle ();
		break;

	    default:
		return LXe_OUTOFBOUNDS;
	}

	return LXe_OK;
}

	const LXtTextValueHint *
CArcTool::attr_Hints (
	unsigned int		 index)
{
	return 0;
}


/*
 * ATTRIBUTES UI Interface
 */
	LxResult
CArcTool::atrui_UIHints (
	unsigned int		 index,
	ILxUnknownID		 hints)
{
	CLxLoc_UIHints		 uiHints(hints);

	switch (index) {
	    case ATTRa_RADI:
		uiHints.MinFloat (0.0);
		break;

	    case ATTRa_ANGL:
		uiHints.MinFloat (0.0);
		uiHints.MaxFloat (180.0 * LXx_DEG2RAD);
		break;

	    case ATTRa_SEGS:
		uiHints.MinInt (1);
		break;
	}
	return LXe_OK;
}


/*
 * Tool evaluation gets the primary mesh and creates the arc shape on the mesh.
 */
	void
CArcTool::tool_Evaluate (
	ILxUnknownID		 vts)
{
	CLxUser_VectorStack	 vec (vts);
	LXpToolViewEvent	*view;

	view = (LXpToolViewEvent *) vec.Read (offset_view);
	if (!view || ((view->type != LXi_VIEWTYPE_3D) && (view->type != LXi_VIEWTYPE_2D)))
		return;

	LXpToolXfrm		*xfrm;
	xfrm = (LXpToolXfrm *) vec.Read (offset_xfrm);

	/*
	 * Start the scan to the primary mesh layer.
	 */
	CLxUser_LayerScan	 scan;

	s_layer.BeginScan (LXf_LAYERSCAN_PRIMARY | LXf_LAYERSCAN_WRITEMESH | LXf_LAYERSCAN_MARKVERTS, scan);

	/*
	 * Check the primary mesh. If no active mesh in the layer, 
	 * we abort the evaluation. 
	 */
	if (scan.NumLayers () == 0)
		return;

	/*
	 * Set the primary mesh to edit.
	 */
	CLxUser_Mesh		 e_mesh;
	CLxUser_Polygon		 e_poly;
	CLxUser_Point		 e_vert;
	CLxUser_MeshMap		 e_mmap;

	scan.EditMeshByIndex (0, e_mesh);
	e_vert.fromMeshObj (e_mesh);
	e_poly.fromMeshObj (e_mesh);
	e_mmap.fromMeshObj (e_mesh);

	/*
	 * Create the arc shape as a face polygon.
	 */
	LXtPointID		*points;
	LXtVector		 pos, v;
	LXtPolygonID		 pol;
	double			 t;
	int			 i, numPnts;

	if (m_segments < 1)
		m_segments = 1;
	numPnts = m_segments + 2;
	points  = new LXtPointID[numPnts];

	LXx_VSUB3 (v, m_start, xfrm->v);
	lx::MatrixMultiply (pos, xfrm->mInv, v);
	e_vert.New (pos, &points[0]);

	for (i = 0; i < m_segments; i++) {
		t = (i + 1) / (double) m_segments;
		GetPos (pos, t);
		LXx_VSUB3 (v, pos, xfrm->v);
		lx::MatrixMultiply (pos, xfrm->mInv, v);
		e_vert.New (pos, &points[i+1]);
	}
	LXx_VSUB3 (v, m_center, xfrm->v);
	lx::MatrixMultiply (pos, xfrm->mInv, v);
	e_vert.New (pos, &points[numPnts-1]);

	e_poly.New (LXiPTYP_FACE, points, numPnts, 0, &pol);

	/*
	 * Apply the edit to the base mesh.
	 */
	scan.SetMeshChange (0, LXf_MESHEDIT_GEOMETRY);
	scan.Apply ();

	delete [] points;
}

/*
 * Set the radius from the start and end handles.
 */
	void
CArcTool::SetRadius ()
{
	LXtVector		 vec;

	LXx_VSUB3 (vec, m_start, m_center);
	m_radius = LXx_VLEN (vec);
}

/*
 * Set the analge from the three handles.
 */
	void
CArcTool::SetAngle ()
{
	LXtVector		 v0, v1;

	LXx_VSUB3 (v0, m_start, m_center);
	LXx_VSUB3 (v1, m_end  , m_center);
	if (!m_lockAngle) {
		m_angle = Util::AngleVectors (v0, v1);
	}
}

/*
 * Set the start and end handle positions from the center and radius.
 */
	void
CArcTool::SetRotHandles ()
{
	LXtVector		 vec;
	double			 len;

	LXx_VSUB3 (vec, m_start, m_center);
	len = LXx_VLEN (vec);
	if (len > 0.0) {
		LXx_VSCL (vec, m_radius / len);
		LXx_VADD3 (m_start, m_center, vec);
	} else {
		LXx_VSCL3 (vec, m_vecS, m_radius);
		LXx_VADD3 (m_start, m_center, vec);
	}

	LXx_VSUB3 (vec, m_end, m_center);
	len = LXx_VLEN (vec);
	if (len > 0.0) {
		LXx_VSCL (vec, m_radius / len);
		LXx_VADD3 (m_end, m_center, vec);
	} else {
		LXx_VSCL3 (vec, m_vecE, m_radius);
		LXx_VADD3 (m_end, m_center, vec);
		GetPos (m_end, 1.0, 1.0);
	}
}

/*
 * Set the end handle position from the center and radius.
 */
	void
CArcTool::SetEndHandle ()
{
	LXtVector		 vec;
	double			 scl;

	LXx_VSUB3 (vec, m_end, m_center);
	scl = LXx_VLEN (vec) / m_radius;
	GetPos (m_end, 1.0, scl);
}

/*
 * Compute the arc position for the given fractional t.
 */
	void
CArcTool::GetPos (LXtVector pos, double t, double scale)
{
	LXtMatrix		 m;
	LXtVector		 vec0, vec1, vec2;
	double			 scl, len, angle;

	LXx_VSUB3 (vec0, m_start, m_center);
	LXx_VSUB3 (vec1, m_end  , m_center);
	LXx_VCROSS (vec2, vec0, vec1);
	len = LXx_VLEN (vec2);
	if (!len) {
		LXx_VCPY (vec2, m_vecAxis);
		len = LXx_VLEN (vec2);
	}
	if (len > 0.0) {
		scl = 1.0 / len;
		LXx_VSCL (vec2, scl);
		if (m_reverse) {
			LXx_VNEG (vec2);
			angle = (LXx_TWOPI - m_angle) * t;
		} else
			angle = m_angle * t;

		lx::MatrixAxisRotation (m, vec2, sin (angle), cos (angle));
		LXx_VSCL (vec0, scale);
		lx::MatrixMultiply (pos, m, vec0);
		LXx_VADD (pos, m_center);
	} else
		LXx_VCPY (pos, m_start);
}

/*
 * Compute the matrix of the arc plane.
 */
	void
CArcTool::PlaneMatrix (LXtMatrix m)
{
	LXtVector		 x, y, z;
	double			 len, scl;

	LXx_VSUB3 (x, m_start, m_center);
	LXx_VSUB3 (y, m_end  , m_center);
	LXx_VCROSS (z, x, y);
	len = LXx_VLEN (z);
	if (len > 0.0) {
		scl = 1.0 / len;
		LXx_VSCL (z, scl);
		scl = 1.0 / LXx_VLEN (x);
		LXx_VSCL (x, scl);
		LXx_VCROSS (y, z, x);
		m[0][0] = x[0];
		m[1][0] = x[1];
		m[2][0] = x[2];
		m[0][1] = y[0];
		m[1][1] = y[1];
		m[2][1] = y[2];
		m[0][2] = z[0];
		m[1][2] = z[1];
		m[2][2] = z[2];
	} else {	
		m[0][0] = m[1][1] = m[2][2] = 1.0;
		m[0][1] = m[0][2] = 0.0;
		m[1][0] = m[1][2] = 0.0;
		m[2][0] = m[2][1] = 0.0;
	}
}


/*
 * Export tool server with all of its many interfaces.
 */
	void
initialize ()
{
	CLxGenericPolymorph		*srv;

	srv = new CLxPolymorph<CArcTool>;
	srv->AddInterface (new CLxIfc_Tool        <CArcTool>);
	srv->AddInterface (new CLxIfc_ToolModel   <CArcTool>);
	srv->AddInterface (new CLxIfc_Attributes  <CArcTool>);
	srv->AddInterface (new CLxIfc_AttributesUI<CArcTool>);
	lx::AddServer ("prim.arc", srv);
}

