/*
 * Plug-in component shader: cel edge shader
 *
 *   Copyright 0000
 */

#include <lxsdk/lx_shade.hpp>
#include <lxsdk/lx_vector.hpp>
#include <lxsdk/lx_package.hpp>
#include <lxsdk/lx_action.hpp>
#include <lxsdk/lx_value.hpp>
#include <lxsdk/lx_log.hpp>
#include <lxsdk/lx_channelui.hpp>
#include <lxsdk/lx_item.hpp>
#include <lxsdk/lxcommand.h>
#include <lxsdk/lxidef.h>
#include <lxsdk/lx_raycast.hpp>
#include <lxsdk/lx_shdr.hpp>
#include <lxsdk/lx_tableau.hpp>

#include <math.h>
#include <string>

/*
 * Vector Packet definition: CelEdge Packet
 * Here we define the packet that will be used to carry the shading parameters through the shading pipe
 */
	
class CelEdgePacket : public CLxImpl_VectorPacket
{
    public:
	CelEdgePacket () {}

	static LXtTagInfoDesc	descInfo[];

	unsigned int	vpkt_Size () LXx_OVERRIDE;
	const LXtGUID * vpkt_Interface (void) LXx_OVERRIDE;
	LxResult	vpkt_Initialize (void	*packet) LXx_OVERRIDE;
	LxResult	vpkt_Blend (void	*packet,void	*p0,void	*p1,float	t,int	mode) LXx_OVERRIDE;
};

#define SRVs_CEL_VPACKET		"cel.edge.packet"
#define LXsP_SAMPLE_CELEDGES		SRVs_CEL_VPACKET

LXtTagInfoDesc	 CelEdgePacket::descInfo[] = {
        { LXsSRV_USERNAME,	"Cel Edge Packet" },
	{ LXsSRV_LOGSUBSYSTEM,	"vector-packet"},
	{ LXsVPK_CATEGORY,	LXsCATEGORY_SAMPLE},
	{ 0 }
};

typedef struct st_LXpCelEdges {
	float		incidenceEdge;
	float		angleEdge, minAngle, maxAngle;
	float		distEdge, distThrs;
	float		edgeWidth;
	float		segmentEdge;
	float		surfaceEdge;
	LXtFVector	color;
} LXpCelEdges;


	unsigned int
CelEdgePacket::vpkt_Size (void) 
{
	return	sizeof (LXpCelEdges);
}
	
	const LXtGUID *
CelEdgePacket::vpkt_Interface (void)
{
	return NULL;
}

	LxResult
CelEdgePacket::vpkt_Initialize (
	void			*p) 
{
	LXpCelEdges		*csp = (LXpCelEdges *)p;

	csp->edgeWidth = 1.0;
	return LXe_OK;
}

	LxResult
CelEdgePacket::vpkt_Blend (
	void			*p, 
	void			*p0, 
	void			*p1,
	float			 t,
	int			 mode)
{
/*	LXpCelEdges		*csp = (LXpCelEdges *)p;
	LXpCelEdges		*csp0 = (LXpCelEdges *)p0;
	LXpCelEdges		*csp1 = (LXpCelEdges *)p1;
	CLxLoc_ShaderService	 shdrSrv;

	csp->edgeWidth = shdrSrv.ScalarBlendValue (csp0->edgeWidth, csp1->edgeWidth, t, mode);
	shdrSrv.ColorBlendValue (csp->color, csp0->color, csp1->color, t, mode);
	*/
	return LXe_OK;
}


/*
 * Cel material & shader
 * The cel shader is implemented as a custom material. Custom materials have the ability to set material attributes 
 * and to contribute to shading after the base shader has been evaluated
 */
class CelEdgeMaterial : public CLxImpl_CustomMaterial,
                public CLxImpl_ChannelUI
{
    public:
	CelEdgeMaterial () {}

	static LXtTagInfoDesc	descInfo[];

	int 			cmt_Flags (void) LXx_OVERRIDE;
	LxResult		cmt_SetupChannels (ILxUnknownID addChan) LXx_OVERRIDE;
	LxResult		cmt_LinkChannels  (ILxUnknownID eval, ILxUnknownID item) LXx_OVERRIDE;
	LxResult		cmt_ReadChannels  (ILxUnknownID attr, void **ppvData) LXx_OVERRIDE;
	LxResult		cmt_CustomPacket  (const char	**) LXx_OVERRIDE;
	void			cmt_MaterialEvaluate      (
					ILxUnknownID		 etor,
					int			*idx,
					ILxUnknownID		 vector,
					void			*data) LXx_OVERRIDE;
	void			cmt_ShaderEvaluate      (
					ILxUnknownID		 vector,
					ILxUnknownID		 rayObj,
					LXpShadeComponents	*sCmp,
					LXpShadeOutput		*sOut,
					void			*data) LXx_OVERRIDE;
	void			cmt_Cleanup       (void *data) LXx_OVERRIDE;

	LxResult		cmt_SetOpaque       (int *opaque) LXx_OVERRIDE;

	LxResult		cui_UIHints	      (const char *channelName, ILxUnknownID hints) LXx_OVERRIDE;
		
        LXtItemType		MyType ();

	CLxUser_PacketService	pkt_service;
	
	unsigned		ray_offset;
	unsigned		nrm_offset;
	unsigned		pos_offset;
	unsigned		pix_offset;
	unsigned		pkt_offset;
	
  	LXtItemType		my_type; 	
	
	LXtSampleIndex          samp_idx[16];     // indices to each data channel in RendData
	int			cmt_IsSampleDriven (int *idx) LXx_OVERRIDE;
	LxResult		cmt_LinkSampleChannels (ILxUnknownID eval, ILxUnknownID item, int *idx) LXx_OVERRIDE;
	CLxUser_NodalService	nodalSvc;
	
	class RendData {
	    public:
		float		incidenceEdge;
		float		angleEdge, minAngle, maxAngle;
		float		distEdge, distThrs;
		float		edgeWidth;
		float		segmentEdge;
		float		surfaceEdge;
		int		edgeQuality;
		LXtFVector	color;
	};
};

#define SRVs_CEL_MATR			"celEdges"
#define SRVs_CEL_MATR_ITEMTYPE		"material." SRVs_CEL_MATR

LXtTagInfoDesc	 CelEdgeMaterial::descInfo[] = {
        { LXsSRV_USERNAME,	"Cel Edges Material" },
	{ LXsSRV_LOGSUBSYSTEM,	"comp-shader"	},
	{ 0 }
};

/* 
 * clean up render data
 */
	void
CelEdgeMaterial::cmt_Cleanup (
	void			*data)
{
	RendData*       rd = (RendData*)data;

	delete rd;
}

/*
 * Specify item special behaviors if any.
 */
	int
CelEdgeMaterial::cmt_Flags ()
{
	return 0;
}

/*
 * Setup channels for the item type.
 */
	LxResult
CelEdgeMaterial::cmt_SetupChannels (
	ILxUnknownID		 addChan)
{
	CLxUser_AddChannel	 ac (addChan);
	LXtVector zero = {0, 0, 0};
     
	ac.NewChannel ("incidenceEdges",       LXsTYPE_PERCENT);
        ac.SetDefault (1.0, 0);

	ac.NewChannel ("angleEdges",           LXsTYPE_PERCENT);
        ac.SetDefault (1.0, 0);

	ac.NewChannel ("distanceEdges",        LXsTYPE_PERCENT);
        ac.SetDefault (1.0, 0);

	ac.NewChannel ("minAngle",        LXsTYPE_ANGLE);
        ac.SetDefault (0.0, 0);

	ac.NewChannel ("maxAngle",        LXsTYPE_ANGLE);
        ac.SetDefault (0.25*LXx_PI, 0);

	ac.NewChannel ("distThreshold",        LXsTYPE_PERCENT);
        ac.SetDefault (0.02, 0);

	ac.NewChannel ("edgeWidth",        LXsTYPE_FLOAT);
	ac.SetDefault (2.0, 0);

	ac.NewChannel ("segmentEdges",       LXsTYPE_PERCENT);
        ac.SetDefault (0.0, 0);

	ac.NewChannel ("edgeColor", LXsTYPE_COLOR1);
	ac.SetVector  (LXsCHANVEC_RGB);
        ac.SetDefaultVec (zero);
	
	ac.NewChannel ("surfaceEdges",       LXsTYPE_PERCENT);
	ac.SetDefault (0.0, 0);

	ac.NewChannel ("edgeQuality",        LXsTYPE_INTEGER);
	ac.SetDefault (0, 2);
	return LXe_OK;
}

/*
 * Attach to channel evaluations.
 * This gets the indices for the channels in attributes.
 */
	LxResult
CelEdgeMaterial::cmt_LinkChannels (
	ILxUnknownID		 eval,
	ILxUnknownID		 item)
{
	CLxUser_Evaluation	 ev (eval);
	CLxUser_Item		 it (item);
	int			 i = 0;

	samp_idx[i++].chan = it.ChannelIndex ("incidenceEdges");
	samp_idx[i++].chan = it.ChannelIndex ("angleEdges");
	samp_idx[i++].chan = it.ChannelIndex ("distanceEdges");
	samp_idx[i++].chan = it.ChannelIndex ("minAngle");
	samp_idx[i++].chan = it.ChannelIndex ("maxAngle");
	samp_idx[i++].chan = it.ChannelIndex ("distThreshold");
	samp_idx[i++].chan = it.ChannelIndex ("edgeWidth");
	samp_idx[i++].chan = it.ChannelIndex ("segmentEdges");
	samp_idx[i++].chan = it.ChannelIndex ("edgeColor.R");
	samp_idx[i++].chan = it.ChannelIndex ("edgeColor.G");
	samp_idx[i++].chan = it.ChannelIndex ("edgeColor.B");
	samp_idx[i++].chan = it.ChannelIndex ("surfaceEdges");
	samp_idx[i++].chan = it.ChannelIndex ("edgeQuality");

	i = 0;
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"incidenceEdges");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"angleEdges");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"distanceEdges");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"minAngle");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"maxAngle");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"distThreshold");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"edgeWidth");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"segmentEdges");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"edgeColor.R");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"edgeColor.G");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"edgeColor.B");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"surfaceEdges");
	samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"edgeQuality");

	ray_offset  = pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SAMPLE_RAY);
	nrm_offset  = pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SURF_NORMAL);
	pos_offset  = pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SAMPLE_POSITION);
	pkt_offset  = pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SAMPLE_CELEDGES);
	return LXe_OK;
}

	LxResult
CelEdgeMaterial::cmt_LinkSampleChannels (
	ILxUnknownID		 eval,
	ILxUnknownID		 item,
	int			*idx)
{
	CLxUser_Evaluation	 ev (eval);
	int i = 0;
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"incidenceEdges");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"angleEdges");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"distanceEdges");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"minAngle");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"maxAngle");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"distThreshold");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"edgeWidth");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"segmentEdges");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"edgeColor.R");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"edgeColor.G");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"edgeColor.B");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"surfaceEdges");
	nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ);  //"edgeQuality");

	return LXe_OK;
}


	int			
CelEdgeMaterial::cmt_IsSampleDriven (
	int *idx)  
{
	 return nodalSvc.AnyDrivenChans (&idx[samp_idx[0].chan], 12); //up to edge quality
}


/*
 * Read channel values which may have changed.
 */
	LxResult
CelEdgeMaterial::cmt_ReadChannels (
	ILxUnknownID		 attr,
	void		       **ppvData)
{
	CLxUser_Attributes	 at (attr);
	RendData*                rd = new RendData;

	int			 i = 0;
   
	rd->incidenceEdge       = at.Float (samp_idx[i++].layer);
	rd->angleEdge           = at.Float (samp_idx[i++].layer);
	rd->distEdge            = at.Float (samp_idx[i++].layer);
	rd->minAngle            = at.Float (samp_idx[i++].layer);
	rd->maxAngle            = at.Float (samp_idx[i++].layer);
	rd->distThrs            = at.Float (samp_idx[i++].layer);
	rd->edgeWidth           = at.Float (samp_idx[i++].layer);
	rd->segmentEdge		= at.Float (samp_idx[i++].layer);
	rd->color[0]		= at.Float (samp_idx[i++].layer);
	rd->color[1]		= at.Float (samp_idx[i++].layer);
	rd->color[2]		= at.Float (samp_idx[i++].layer);
	rd->surfaceEdge		= at.Float (samp_idx[i++].layer);
	rd->edgeQuality		= at.Int   (samp_idx[i++].layer);
	
	ppvData[0] = rd;
	return LXe_OK;
}

/*	static double
FVectorNormalize (
	LXtFVector		v) 
{
	float		m, p;
	m = LXx_VDOT (v, v);
	if(m<=0)
		return -1;
	m = sqrt (m);
	p = 1.0 / m;
	LXx_VSCL (v, p);
	return m;
}

	static void
FVectorQuantize (
	LXtFVector		vec,
	int			bnd,
	float			level) 
{
	float			vec0[3], len;
	int			i;
	
	LXx_VCPY (vec0, vec);
	len = FVectorNormalize (vec0);
	
	for (i=0;i<3;i++)
		vec[i] = vec0[i]*(floor(len*bnd) + level)/(float)bnd;	
} */

	static inline int
MAX (
	int			a,
	int			b)
{
	return (a > b ? a : b);
}

	static inline float
MAX (
	float			a,
	float			b)
{
	return (a > b ? a : b);
}

	static inline float
MIN (
	float			a,
	float			b)
{
	return (a < b ? a : b);
}

/*
 * Since the cel shader is just modifying the results of another shader,
 * it cannot be opaque.
 */

	LxResult
CelEdgeMaterial::cmt_SetOpaque (
        int			*opaque)
{
	*opaque = 0;

	return LXe_OK;
}

	LxResult
CelEdgeMaterial::cmt_CustomPacket (
        const char		**packet)
{
	packet[0] = LXsP_SAMPLE_CELEDGES;
	return LXe_OK;
}

/*
 * Set custom material values at a spot
 */
	void
CelEdgeMaterial::cmt_MaterialEvaluate (
	ILxUnknownID		 etor,
	int			*idx,
        ILxUnknownID            vector,
	void			*data)
{
        LXpCelEdges		*sCel = (LXpCelEdges*) pkt_service.FastPacket (vector, pkt_offset);
	RendData*       	 rd = (RendData*)data;
	int			 i = 0;

	sCel->incidenceEdge     = nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->incidenceEdge );
	sCel->angleEdge         = nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->angleEdge  );
	sCel->distEdge          = nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->distEdge   );
	sCel->minAngle          = nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->minAngle   );
	sCel->maxAngle          = nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->maxAngle   );
	sCel->distThrs          = nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->distThrs   );
	sCel->edgeWidth         = nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->edgeWidth  );
	sCel->segmentEdge	= nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->segmentEdge);
	sCel->color[0]		= nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->color[0]);
	sCel->color[1]		= nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->color[1]);
	sCel->color[2]		= nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->color[2]);
	sCel->surfaceEdge	= nodalSvc.GetFloat (etor, idx, samp_idx[i++].chan, rd->surfaceEdge);
}

/*
 * Evaluate the color at a spot.
 */
	void
CelEdgeMaterial::cmt_ShaderEvaluate (
        ILxUnknownID            vector,
	ILxUnknownID		rayObj,
        LXpShadeComponents     *sCmp,
        LXpShadeOutput         *sOut,
        void                    *data)
{
        //LXpSamplePosition	*sPos = (LXpSamplePosition*) pkt_service.FastPacket (vector, pos_offset);
        LXpSampleSurfNormal	*sNrm = (LXpSampleSurfNormal*) pkt_service.FastPacket (vector, nrm_offset);
        LXpSampleRay		*sRay = (LXpSampleRay*) pkt_service.FastPacket (vector, ray_offset);
        LXpCelEdges		*sCel = (LXpCelEdges*) pkt_service.FastPacket (vector, pkt_offset);
        RendData		*rd   = (RendData *) data;
	float			 edge, s;//, black[3] = {0,0,0};
	int			 i;
	CLxLoc_Raycast		 raycast;
	CLxLoc_ShaderService	 shdrSrv;
	
	raycast.set (rayObj);

	/*
	 * Do edge evaluation only if the ray is of the right type,
	 * this is an optimization to avoid rendering edges on things like GI rays
	 */			
	if ((sRay->flags & (LXfRAY_TYPE_CAMERA | LXfRAY_TYPE_REFLECT | LXfRAY_TYPE_REFRACT | LXfRAY_EVAL_SHADING))) {
		edge = 0.0;
		
		if (sCel->incidenceEdge) {
			s = LXx_VDOT (sRay->dir, sNrm->wNorm);
			if (s>0)
				s = 0.0;
				
			s = pow (1 + s, 4)*sCel->incidenceEdge;	
			if (s>edge)
				edge = s;
		}
		
		/*
		 * Evaluate angle and distance edges by sampling the geometry around the current pixel
		 */
		if (sCel->angleEdge || sCel->distEdge || sCel->segmentEdge || sCel->surfaceEdge) {
			int				  rays = MAX(1, rd->edgeQuality);
			LXpPixelGeometrySeg		  pGeoCenter;
			LXpPixelGeometrySeg		  pGeoNew;
			float				  dot, dotMin = -1, u, v, cosMin, cosMax, edgeWidth, distanceThreshold;
			float				  dmin, dmax, s;
			float				  angle, radius;
	
			// we get the edge width from the CelShader packet, this is how you can texture parameters of the shader
			edgeWidth = sCel->edgeWidth;

			// generate an adaptive number of rays based on the area of the circle
			rays = edgeWidth*edgeWidth*rays;

			distanceThreshold = sCel->distThrs;

			// get the pixel geometry packet of this point
			raycast.PixelGeometrySeg (vector, LXi_PIXELRAY_RELATIVE, 0, 0, &pGeoCenter);
			dmin = dmax = pGeoCenter.dist;
					
			for (i = 0; i < rays; i++) {
				/* Here we generate the sample positions in screen space */
				shdrSrv.PoissonOffset (vector, &u, &v);
				
				// Convert u and v to be in the (-1, -1) to (1, 1) range
				u = 2.0*u - 1.0;
				v = 2.0*v - 1.0;
				// Map u and v onto a circle
				shdrSrv.SquareToCircle(&u, &v);
				// Convert to (-.5, -.5) to (.5, .5) range
				u /= 2.0;
				v /= 2.0;

				// Now, to get samples closer to the edge of,
				// since there are more likely to be edges there,
				// we convert to polar coordinates and bias
				// towards the edge of the unit radius circle
				angle = atan2(u, v);
				radius = sqrt(u*u + v*v)*2;
				radius = SCHLICK_BIAS (radius, .75);
				radius /= 2.0;
				// And convert back to cartesian coordinates
				u = cos(angle) * radius;
				v = sin(angle) * radius;

				/* Finally, we get the goemetry under the sample */
				raycast.PixelGeometrySeg (vector, LXi_PIXELRAY_RELATIVE, edgeWidth*u, edgeWidth*v,  &pGeoNew);
				if (pGeoCenter.gsrf != pGeoNew.gsrf) {
					edge = rd->surfaceEdge;
					if (edge>=1.0)
						break;
				}

				if (pGeoCenter.gseg != pGeoNew.gseg) {
					if (rd->segmentEdge > edge)
						edge = rd->segmentEdge;
				}
				if (sCel->angleEdge) {
					cosMin = cos (sCel->minAngle);
					cosMax = cos (sCel->maxAngle);

					dot = LXx_VDOT (pGeoCenter.nrm, pGeoNew.nrm);
					if (dotMin == -1 || dot < dotMin)
						dotMin = dot;
				
					if (dotMin < cosMin) {
						if (dotMin < cosMax)
							s = 1;
						else
							s = (cosMin - dotMin)/(cosMin - cosMax);
					
						s *= sCel->angleEdge;
					}
					else
						s = 0;
					
					if (s > edge)
						edge = s;
				}
				if (sCel->distEdge) {
					if (pGeoNew.dist < dmin)
						dmin = pGeoNew.dist;
					if (pGeoNew.dist > dmax)
						dmax = pGeoNew.dist;

					s = 0.0;
					if (dmax > (1 + distanceThreshold)*dmin)
						s = sCel->distEdge;
									
					if (s > edge)
						edge = s;
				}

				
					
				if (edge >= MAX (sCel->distEdge, sCel->angleEdge) || edge >= 1.0)
					break;
			}
		}
	}
	else
		edge = 0.0;


	// Update final output color		
	for (i=0;i<3;i++) 
		sOut->color[i] = sCmp->diff[i] + sCmp->spec[i] + sCmp->refl[i] + sCmp->tran[i] + sCmp->subs[i] + sCmp->lumi[i];

	// Modulate with edge
	if (edge > 0) {
		edge = MIN (edge, 1.0);
		LXx_VLERP (sOut->color, sOut->color, sCel->color, edge);
	}
}

/*
 * Utility to get the type code for this item type, as needed.
 */
        LXtItemType
CelEdgeMaterial::MyType ()
{
        if (my_type != LXiTYPE_NONE)
                return my_type;

        CLxUser_SceneService	 svc;

        my_type = svc.ItemType (SRVs_CEL_MATR_ITEMTYPE);
        return my_type;
}
	LxResult
CelEdgeMaterial::cui_UIHints (
        const char		*channelName,
        ILxUnknownID		 hints)
{
	if( !strcmp(channelName, "edgeWidth"		)  || 
	    !strcmp(channelName, "edgeColor.R"	)  ||
	    !strcmp(channelName, "edgeColor.G"	)  ||
	    !strcmp(channelName, "edgeColor.B"		) )
	{
		CLxUser_UIHints		 ui (hints);
		ui.ChannelFlags (LXfUIHINTCHAN_SUGGESTED);
		return LXe_OK;
	}
	
	return LXe_NOTIMPL;
}


/* --------------------------------- */

/*
 * Packet Effects definition: 
 * Here we define the texture effects on the CelShader packet, currently only one : edge width
 */
	
class CelEdgePFX : public CLxImpl_PacketEffect
{
    public:
	CelEdgePFX () {}

	static LXtTagInfoDesc	descInfo[];

	LxResult		pfx_Packet (const char **packet) LXx_OVERRIDE;
	unsigned int		pfx_Count (void) LXx_OVERRIDE;
	LxResult		pfx_ByIndex (int idx, const char **name, const char **typeName, int	*type) LXx_OVERRIDE;
	LxResult		pfx_Get (int idx,void *packet,float *val,void *item) LXx_OVERRIDE;
	LxResult		pfx_Set (int idx,void *packet,const float *val,void *item) LXx_OVERRIDE;
};

#define SRVs_CELEDGE_PFX	"celEdges.PFX"
#define SRVs_EDGE_WIDTH_TFX	"celEdgeWidth"
#define SRVs_EDGE_COLOR_TFX	"celEdgeColor"

LXtTagInfoDesc	 CelEdgePFX::descInfo[] = {
        { LXsSRV_USERNAME,	"Cel Edges Packet FX" },
	{ LXsSRV_LOGSUBSYSTEM,	"texture-effect"},
	{ LXsTFX_CATEGORY,	LXsSHADE_SURFACE},
	{ 0 }
};

	LxResult
CelEdgePFX::pfx_Packet (const char	**packet) 
{
	packet[0] = SRVs_CEL_VPACKET;
	return LXe_OK;
}

	unsigned int
CelEdgePFX::pfx_Count (void) 
{
	return	2;
}

	LxResult
CelEdgePFX::pfx_ByIndex (int	id, const char **name, const char **typeName, int *type) 
{
	switch (id) {
		case 0:
			name[0]     = SRVs_EDGE_WIDTH_TFX;
			type[0]     = LXi_TFX_SCALAR | LXf_TFX_READ | LXf_TFX_WRITE;
			typeName[0] = LXsTYPE_FLOAT;
			break;
		case 1:
			name[0]     = SRVs_EDGE_COLOR_TFX;
			type[0]     = LXi_TFX_COLOR | LXf_TFX_READ | LXf_TFX_WRITE;
			typeName[0] = LXsTYPE_COLOR1;
			break;
	}
			
	return	LXe_OK;
}

	LxResult
CelEdgePFX::pfx_Get (int  id, void *packet,float *val,void *item) 
{
	LXpCelEdges	*csp = (LXpCelEdges *) packet;

	switch (id) {
		case 0:
			val[0] = csp->edgeWidth;
			break;
		case 1:
			val[0] = csp->color[0];
			val[1] = csp->color[1];
			val[2] = csp->color[2];
			break;
	}
	
	return LXe_OK;
}
	LxResult
CelEdgePFX::pfx_Set (int  id, void *packet,const float *val,void *item) 
{
	LXpCelEdges	*csp = (LXpCelEdges *) packet;

	switch (id) {
		case 0:
			csp->edgeWidth = val[0];
			break;
		case 1:
			csp->color[0] = val[0];
			csp->color[1] = val[1];
			csp->color[2] = val[2];
			break;
	}
	
	return LXe_OK;
}

	void
initialize ()
{
    CLxGenericPolymorph*    srv1 = new CLxPolymorph<CelEdgeMaterial>;
    CLxGenericPolymorph*    srv2 = new CLxPolymorph<CelEdgePacket>;
    CLxGenericPolymorph*    srv3 = new CLxPolymorph<CelEdgePFX>;

    srv1->AddInterface (new CLxIfc_CustomMaterial<CelEdgeMaterial>);
    srv1->AddInterface (new CLxIfc_ChannelUI<CelEdgeMaterial>);
    srv1->AddInterface (new CLxIfc_StaticDesc<CelEdgeMaterial>);
    lx::AddServer (SRVs_CEL_MATR, srv1);

    srv2->AddInterface (new CLxIfc_VectorPacket<CelEdgePacket>);
    srv2->AddInterface (new CLxIfc_StaticDesc<CelEdgePacket>);
    lx::AddServer (SRVs_CEL_VPACKET, srv2);

    srv3->AddInterface (new CLxIfc_PacketEffect<CelEdgePFX>);
    srv3->AddInterface (new CLxIfc_StaticDesc<CelEdgePFX>);
    lx::AddServer (SRVs_CELEDGE_PFX, srv3);
}
