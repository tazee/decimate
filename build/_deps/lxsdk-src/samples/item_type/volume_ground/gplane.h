/*
 * Plug-in ground plane item type.
 *
 * Copyright 0000
 *
 */
#ifndef GPLANE_H
#define GPLANE_H

#include <lxsdk/lxlog.h>
#include <lxsdk/lx_action.hpp>
#include <lxsdk/lx_item.hpp>
#include <lxsdk/lx_package.hpp>
#include <lxsdk/lx_tableau.hpp>
#include <lxsdk/lx_vertex.hpp>
#include <lxsdk/lx_value.hpp>
#include <lxsdk/lxu_geometry.hpp>
#include <lxsdk/lxu_log.hpp>
#include <lxsdk/lx_select.hpp>
#include <lxsdk/lx_surface.hpp>
#include <lxsdk/lx_vmodel.hpp>
#include <lxsdk/lx_vector.hpp>
#include <lxsdk/lxvolume.h>
#include <lxsdk/lx_raycast.hpp>
#include <lxsdk/lx_volume.hpp>


class GPlaneLog : public CLxLuxologyLogMessage
{
    public:
	GPlaneLog () : CLxLuxologyLogMessage ("gplane-item") { }

	const char *	 GetFormat  () { return "GPlane Object"; }
};

/*
 * Class Declarations
 *
 * These have to come before their implementions because they reference each
 * other. Descriptions are attached to the implementations.
 */
class GPlanePackage;

class GPlaneElement :
	public CLxImpl_TableauVolume // ,
//	public CLxImpl_TableauInstance  // [TODO] Need tins_Properties and tins_GetTransform?
{
	GPlaneLog		 gpl_log;

    public:
	CLxUser_PacketService	 pkt_service;
	CLxUser_TableauVertex	 vrt_desc;
	int			 f_pos[4];
	LXtMatrix		 xfrm, invXfrm;	
	LXtVector		 itemPos;
	double			 size, fogHeight, nearClip, farClip, density, rate;
	char			*material;
	int			 ground, clipping, fog, axis;
		
	unsigned		ray_offset;
	unsigned		nrm_offset;
	unsigned		pos_offset;
	unsigned		vol_offset;	
	unsigned		den_offset;	
	
	void			RayTransform (  LXpSampleRay		*sRay,
						LXtVector 		 pos,
						LXtVector 		 opos, 	
						LXtVector 		 dir);
						
	float			PlaneHit ( 	LXpSampleRay		*sRay,
						LXpSampleSurfNormal	*sNrm,
						LXtVector 		 opos, 	
						LXtVector 		 dir);

	float			DensityFunc ( 	LXtVector 		 opos);

	int			InsideBox ( 	LXtVector 		 opos,
						double			 size);

	LxResult	 tvol_Bound (LXtTableauBox bbox) LXx_OVERRIDE;
	unsigned	 tvol_FeatureCount (LXtID4 type) LXx_OVERRIDE;
	LxResult	 tvol_FeatureByIndex (
				LXtID4		 type,
				unsigned int	 index,
				const char	**name) LXx_OVERRIDE;
	LxResult	 tvol_SetVertex (ILxUnknownID vdesc) LXx_OVERRIDE;


	int		tvol_Type  () LXx_OVERRIDE;
	
	LxResult	tvol_RenderInit  (ILxUnknownID vector) LXx_OVERRIDE;
	
	LxResult	tvol_RaySample  (
				ILxUnknownID densityShader, 
				ILxUnknownID volumeShader, 
				ILxUnknownID vector,
				ILxUnknownID raycastObj,
				ILxUnknownID raymarchObj) LXx_OVERRIDE;
				
	LxResult	tvol_RayCast  (
				ILxUnknownID densityShader, 
				ILxUnknownID vector, 
				ILxUnknownID raycastObj,
				double *dist,
				int	*localShader) LXx_OVERRIDE;
				
	LxResult	tvol_Density  (
				ILxUnknownID densityShader, 
				ILxUnknownID vector, 
				ILxUnknownID raycastObj,
				const LXtVector pos, 
				int wp, 
				double *dens) LXx_OVERRIDE;

	// [TODO] Does a volumetric implement CLxImpl_TableauInstance?
#if 0
	LxResult	 tins_Properties (
				ILxUnknownID	 vecstack) LXx_OVERRIDE;

	LxResult	 tins_GetTransform (
				unsigned	 endPoint,
				LXtVector	 offset,
				LXtMatrix	 xfrm) LXx_OVERRIDE;
#endif
};

class GPlanePackage;


class GPlaneInstance :
	public CLxImpl_PackageInstance,
	public CLxImpl_StringTag,
	public CLxImpl_TableauSource,
	public CLxImpl_ViewItem3D
{
	GPlaneLog		 	gpl_log;

    public:
	GPlanePackage	*src_pkg;
	CLxUser_Item	 m_item;
	ILxUnknownID	 inst_ifc;
	CLxUser_PacketService	 pkt_service;

	unsigned		ray_offset;
	unsigned		nrm_offset;
	unsigned		pos_offset;

	LxResult	 pins_Initialize (
				ILxUnknownID	 item,
				ILxUnknownID	 super) LXx_OVERRIDE;
	void		 pins_Cleanup (void) LXx_OVERRIDE;
	LxResult	 pins_SynthName (char *buf, unsigned len) LXx_OVERRIDE;
	unsigned	 pins_DupType (void) LXx_OVERRIDE;
	LxResult	 pins_TestParent (ILxUnknownID item) LXx_OVERRIDE;
	LxResult	 pins_Loading (void) LXx_OVERRIDE;
	LxResult	 pins_AfterLoad (void) LXx_OVERRIDE;
	void		 pins_Doomed (void) LXx_OVERRIDE;

	LxResult	 stag_Get (LXtID4 type, const char **tag) LXx_OVERRIDE;

	LxResult	 tsrc_Elements (ILxUnknownID tblx) LXx_OVERRIDE;
	LxResult	 tsrc_PreviewUpdate (
				int	 chanIndex,
				int	*update) LXx_OVERRIDE;

	LxResult	 vitm_Draw (
				ILxUnknownID	 itemChanRead,
				ILxUnknownID	 viewStrokeDraw,
				int		 selectionFlags,
				const LXtVector	 itemColor) LXx_OVERRIDE;

	LxResult	 vitm_HandleCount (
				int		*count) LXx_OVERRIDE;

	LxResult	 vitm_HandleMotion (
				int		 handleIndex,
				int		*motionType,
				double		*min,
				double		*max,
				LXtVector	 plane,
				LXtVector	 offset) LXx_OVERRIDE;

	LxResult	 vitm_HandleChannel (
				int		 handleIndex,
				int		*chanIndex) LXx_OVERRIDE;

	LxResult	 vitm_HandleValueToPosition (
				int		 handleIndex,
				const double		*chanValue,
				LXtVector	 position) LXx_OVERRIDE;

	LxResult	 vitm_HandlePositionToValue (
				int		 handleIndex,
				const LXtVector	 position,
				double		*chanValue) LXx_OVERRIDE;
};

class GPlanePackage :
	public CLxImpl_Package
{
    public:
	static LXtTagInfoDesc		 descInfo[];
	CLxPolymorph<GPlaneInstance>	 gpl_factory;
	CLxPolymorph<GPlaneElement>	 elt_factory;

	GPlanePackage ();

	LxResult		pkg_SetupChannels (
					ILxUnknownID addChan) LXx_OVERRIDE;
	LxResult		pkg_TestInterface (const LXtGUID *guid) LXx_OVERRIDE;
	LxResult		pkg_Attach (void **ppvObj) LXx_OVERRIDE;
};

#endif // GPLANE_H
