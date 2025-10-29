/*
 * Plug-in custom shader: 
 * Hair Shader - based off "An Artist-Friendly Hair Shading System"
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
#include <lxsdk/lx_tableau.hpp>
#include <lxsdk/lxidef.h>
#include <lxsdk/lx_raycast.hpp>
#include <lxsdk/lxtableau.h>
#include <lxsdk/lxlocator.h>
#include <lxsdk/lxvmath.h>
#include <lxsdk/lx_thread.hpp>
#include <lxsdk/lx_shdr.hpp>
#include <lxsdk/lx_log.hpp>

#include <math.h>
#include <string>
#include <stdlib.h>

#define TABLE_ENTRIES 32
#define HALF_TABLE_ENTRIES 16
#define INTEGRAL_SAMPLES 32

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b)) 
#define CLAMP(value, low, high) (((value)<(low))?(low):(((value)>(high))?(high):(value)))

        int
IEEEFloatTest (
        float			 val)  
{ 
        unsigned int		 n, exp, mant;

        n = ((unsigned int *) &val) [0];
        exp = (n & 0x7F800000) >> 23;
        if (exp != 255)
                return 0;

        mant = (n & 0x007FFFFF);
        if (mant != 0)
                return -1;	// NAN
        else
                return +1;	// INFINITY
}

        void
Desaturate (
        LXtFVector		col)
{
        float avg;
        avg = sqrtf((col[0]*col[0] + col[1]*col[1] + col[2]*col[2])/3.0f);
        LXx_VSET (col, avg);
}

        void
Desaturate (
        LXtVector		col)
{
        double avg;
        avg = sqrtf((col[0]*col[0] + col[1]*col[1] + col[2]*col[2])/3.0);
        LXx_VSET (col, avg);
}

        void
Resaturate (
        LXtFVector		col,
        LXtFVector		target)
{
        LXtFVector		targetCpy;
        float avg;

        LXx_VCPY (targetCpy, target);
        
        avg = (targetCpy[0] + targetCpy[1] + targetCpy[2])/3.0f;
        LXx_VSCL (targetCpy, avg);
        LXx_VMUL (col, targetCpy);
}

        void
Resaturate (
        LXtVector		col,
        LXtVector		target)
{
        LXtVector		targetCpy;
        double avg;

        LXx_VCPY (targetCpy, target);

        avg = (targetCpy[0] + targetCpy[1] + targetCpy[2])/3.0;
        LXx_VSCL (targetCpy, avg);
        LXx_VMUL (col, targetCpy);
}

static LXtTextValueHint hint_giType[] = {
                0,		"none", 
                1,		"recv",
                2,		"cast",
                3,		"both",
                3
};

#define SRVs_HAIR_VPACKET		"hair.shader.packet"
#define LXsP_SAMPLE_HAIRSHADER		SRVs_HAIR_VPACKET

/*------------------------------- Luxology LLC --------------------------- 8/31/11
 *
 * This defines all the data that exists in the hair material, which makes it
 * easy to pass around between the hair shader packet and hair material.
 *
 *----------------------------------------------------------------------------*/

class HairMaterialData {
    public:
                HairMaterialData () {
                        this->densityFactor = .7;
                }

                LXtFVector	primaryHighlightColor;
                float		primaryHighlightIntensity;
                float		primaryHighlightLongWidth;
                float		primaryHighlightLongShift;
        
                LXtFVector	secondaryHighlightColor;
                float		secondaryHighlightIntensity;
                float		secondaryHighlightLongWidth;
                float		secondaryHighlightLongShift;

                LXtFVector	rimLightColor;
                float		rimLightIntensity;
                float		rimLightLongWidth;
                float		rimLightLongShift;
                float		rimLightAzimuthalWidth;

                LXtFVector	glintsColor;
                float		glintsIntensity;
                float		glintsFrequency;

                LXtFVector	fwdScatteringColorAdj;
                float		fwdScatteringIntensityAdj;
                float		fwdScatteringSaturation;

                LXtFVector	bwdScatteringColorAdj;
                float		bwdScatteringIntensityAdj;
                float		bwdScatteringLongWidthAdj;
                float		bwdScatteringLongShift;
                float		bwdScatteringSaturation;

                float		globalScatteringSaturation;

                float		densityFactor;

                float		filterDist;

                int		giType;
                int		giRays;

                int		singleScatteringOn;
};

typedef LXtSampleIndex           LXtSampIndVector[LXdND];
struct HairMaterialDataSampleIndex{
	LXtSampIndVector	primaryHighlightColor;
	LXtSampleIndex		primaryHighlightIntensity;
	LXtSampleIndex		primaryHighlightLongWidth;
	LXtSampleIndex		primaryHighlightLongShift;

	LXtSampIndVector	secondaryHighlightColor;
	LXtSampleIndex		secondaryHighlightIntensity;
	LXtSampleIndex		secondaryHighlightLongWidth;
	LXtSampleIndex		secondaryHighlightLongShift;

	LXtSampIndVector	rimLightColor;
	LXtSampleIndex		rimLightIntensity;
	LXtSampleIndex		rimLightLongWidth;
	LXtSampleIndex		rimLightLongShift;
	LXtSampleIndex		rimLightAzimuthalWidth;

	LXtSampIndVector	glintsColor;
	LXtSampleIndex		glintsIntensity;
	LXtSampleIndex		glintsFrequency;

	LXtSampIndVector	fwdScatteringColorAdj;
	LXtSampleIndex		fwdScatteringIntensityAdj;
	LXtSampleIndex		fwdScatteringSaturation;

	LXtSampIndVector	bwdScatteringColorAdj;
	LXtSampleIndex		bwdScatteringIntensityAdj;
	LXtSampleIndex		bwdScatteringLongWidthAdj;
	LXtSampleIndex		bwdScatteringLongShift;
	LXtSampleIndex		bwdScatteringSaturation;

	LXtSampleIndex		globalScatteringSaturation;

	LXtSampleIndex		densityFactor;

	LXtSampleIndex		filterDist;

	LXtSampleIndex		giType;
	LXtSampleIndex		giRays;

	LXtSampleIndex		singleScatteringOn;
};
/*------------------------------- Luxology LLC --------------------------- 8/31/11
 *
 * The Hair Shader packet definition.
 *
 *----------------------------------------------------------------------------*/
        
class HairPacket : public CLxImpl_VectorPacket
{
    public:
        HairPacket () {}

        static LXtTagInfoDesc	descInfo[];

        unsigned int	vpkt_Size () LXx_OVERRIDE;
        const LXtGUID * vpkt_Interface (void) LXx_OVERRIDE;
        LxResult	vpkt_Initialize (void	*packet) LXx_OVERRIDE;
        LxResult	vpkt_Blend (void	*packet,void	*p0,void	*p1,float	t,int	mode) LXx_OVERRIDE;
};


LXtTagInfoDesc	 HairPacket::descInfo[] = {
        { LXsSRV_USERNAME,	"Hair Shader Packet" },
        { LXsSRV_LOGSUBSYSTEM,	"vector-packet"},
        { LXsVPK_CATEGORY,	LXsCATEGORY_SAMPLE},
        { 0 }
};

typedef struct st_LXpHairShader {
        HairMaterialData		matrData;
} LXpHairShader;


        unsigned int
HairPacket::vpkt_Size (void) 
{
        return	sizeof (LXpHairShader);
}
        
        const LXtGUID *
HairPacket::vpkt_Interface (void) 
{
        return NULL;
}

        LxResult
HairPacket::vpkt_Initialize (
        void			*p) 
{
        return LXe_OK;
}

        LxResult
HairPacket::vpkt_Blend (
        void			*p, 
        void			*p0, 
        void			*p1,
        float			 t,
        int			 mode)
{
        LXpHairShader		*hsp	= (LXpHairShader *)p;
        LXpHairShader		*hsp0	= (LXpHairShader *)p0;
        LXpHairShader		*hsp1	= (LXpHairShader *)p1;

        CLxLoc_ShaderService	 shdrSrv;

        HairMaterialData	*md	= &(hsp->matrData);
        HairMaterialData	*md0	= &(hsp0->matrData);
        HairMaterialData	*md1	= &(hsp1->matrData);

        /*
         * Color values
         */
        shdrSrv.ColorBlendValue (md->primaryHighlightColor,
                                        md0->primaryHighlightColor, 
                                        md1->primaryHighlightColor, 
                                        t, 
                                        mode);
        shdrSrv.ColorBlendValue (md->secondaryHighlightColor,
                                        md0->secondaryHighlightColor, 
                                        md1->secondaryHighlightColor, 
                                        t, 
                                        mode);
        shdrSrv.ColorBlendValue (md->rimLightColor,
                                        md0->rimLightColor, 
                                        md1->rimLightColor, 
                                        t, 
                                        mode);
        shdrSrv.ColorBlendValue (md->fwdScatteringColorAdj,
                                        md0->fwdScatteringColorAdj, 
                                        md1->fwdScatteringColorAdj, 
                                        t, 
                                        mode);
        shdrSrv.ColorBlendValue (md->bwdScatteringColorAdj,
                                        md0->bwdScatteringColorAdj, 
                                        md1->bwdScatteringColorAdj, 
                                        t, 
                                        mode);

        /*
         * Intensity values
         */
        md->primaryHighlightIntensity	= shdrSrv.ScalarBlendValue (md0->primaryHighlightIntensity,
                                                                        md1->primaryHighlightIntensity,
                                                                        t,
                                                                        mode);
        md->secondaryHighlightIntensity = shdrSrv.ScalarBlendValue (md0->secondaryHighlightIntensity,
                                                                        md1->secondaryHighlightIntensity,
                                                                        t,
                                                                        mode);
        md->rimLightIntensity		= shdrSrv.ScalarBlendValue (md0->rimLightIntensity,
                                                                        md1->rimLightIntensity,
                                                                        t,
                                                                        mode);
        md->glintsIntensity		= shdrSrv.ScalarBlendValue (md0->glintsIntensity,
                                                                        md1->glintsIntensity,
                                                                        t,
                                                                        mode);
        md->fwdScatteringIntensityAdj	= shdrSrv.ScalarBlendValue (md0->fwdScatteringIntensityAdj,
                                                                        md1->fwdScatteringIntensityAdj,
                                                                        t,
                                                                        mode);
        md->bwdScatteringIntensityAdj	= shdrSrv.ScalarBlendValue (md0->bwdScatteringIntensityAdj,
                                                                        md1->bwdScatteringIntensityAdj,
                                                                        t,
                                                                        mode);

        /*
         * Widths
         */
        md->primaryHighlightLongWidth	= shdrSrv.ScalarBlendValue (md0->primaryHighlightLongWidth,
                                                                        md1->primaryHighlightLongWidth,
                                                                        t,
                                                                        mode);
        md->secondaryHighlightLongWidth = shdrSrv.ScalarBlendValue (md0->secondaryHighlightLongWidth,
                                                                        md1->secondaryHighlightLongWidth,
                                                                        t,
                                                                        mode);
        md->rimLightLongWidth		= shdrSrv.ScalarBlendValue (md0->rimLightLongWidth,
                                                                        md1->rimLightLongWidth,
                                                                        t,
                                                                        mode);
        md->rimLightAzimuthalWidth	= shdrSrv.ScalarBlendValue (md0->rimLightAzimuthalWidth,
                                                                        md1->rimLightAzimuthalWidth,
                                                                        t,
                                                                        mode);
        md->glintsFrequency		= shdrSrv.ScalarBlendValue (md0->glintsFrequency,
                                                                        md1->glintsFrequency,
                                                                        t,
                                                                        mode);
        md->bwdScatteringLongWidthAdj	= shdrSrv.ScalarBlendValue (md0->bwdScatteringLongWidthAdj,
                                                                        md1->bwdScatteringLongWidthAdj,
                                                                        t,
                                                                        mode);

        /*
         * Shifts
         */
        md->primaryHighlightLongShift	= shdrSrv.ScalarBlendValue (md0->primaryHighlightLongShift,
                                                                        md1->primaryHighlightLongShift,
                                                                        t,
                                                                        mode);
        md->secondaryHighlightLongShift = shdrSrv.ScalarBlendValue (md0->secondaryHighlightLongShift,
                                                                        md1->secondaryHighlightLongShift,
                                                                        t,
                                                                        mode);
        md->rimLightLongShift		= shdrSrv.ScalarBlendValue (md0->rimLightLongShift,
                                                                        md1->rimLightLongShift,
                                                                        t,
                                                                        mode);
        md->bwdScatteringLongShift	= shdrSrv.ScalarBlendValue (md0->bwdScatteringLongShift,
                                                                        md1->bwdScatteringLongShift,
                                                                        t,
                                                                        mode);

        /*
         * Other things
         */
        md->densityFactor		= shdrSrv.ScalarBlendValue (md0->densityFactor,
                                                                        md1->densityFactor,
                                                                        t,
                                                                        mode);
        md->fwdScatteringSaturation	= shdrSrv.ScalarBlendValue (md0->fwdScatteringSaturation,
                                                                        md1->fwdScatteringSaturation,
                                                                        t,
                                                                        mode);
        md->bwdScatteringSaturation	= shdrSrv.ScalarBlendValue (md0->bwdScatteringSaturation,
                                                                        md1->bwdScatteringSaturation,
                                                                        t,
                                                                        mode);
        md->globalScatteringSaturation	= shdrSrv.ScalarBlendValue (md0->globalScatteringSaturation,
                                                                        md1->globalScatteringSaturation,
                                                                        t,
                                                                        mode);

        md->filterDist			= MAX (md0->filterDist, md1->filterDist);

        md->giType = md1->giType;
        md->giRays = MAX (md0->giRays, md1->giRays);

        md->singleScatteringOn = MAX (md0->singleScatteringOn, md1->singleScatteringOn);

        return LXe_OK;
}

/*------------------------------- Luxology LLC --------------------------- 8/31/11
 *
 * The Hair Material itself.
 *
 *----------------------------------------------------------------------------*/

#define NUM_CHANNELS 41

class HairMaterial : public CLxImpl_CustomMaterial, public CLxImpl_ChannelUI
{
    public:
        HairMaterial () {
                CLxUser_ThreadService	threadService;
                threadService.set();

                threadService.NewMutex(this->precomputeMutex);

                this->isPrecomputed = false;
        }

        static LXtTagInfoDesc	descInfo[];

        /*
         *  Custom Material Interface
         */
        int			cmt_Flags () LXx_OVERRIDE;
        LxResult		cmt_SetupChannels (ILxUnknownID addChan) LXx_OVERRIDE;
        LxResult		cmt_LinkChannels  (ILxUnknownID eval, ILxUnknownID item) LXx_OVERRIDE;
        LxResult		cmt_ReadChannels  (ILxUnknownID attr, void **ppvData) LXx_OVERRIDE;
        LxResult		cmt_CustomPacket  (const char	**) LXx_OVERRIDE;
		void			cmt_MaterialEvaluate(ILxUnknownID etor, int *idx, ILxUnknownID vector, void *) LXx_OVERRIDE;
        void			cmt_ShaderEvaluate      (ILxUnknownID vector, ILxUnknownID rayObj, LXpShadeComponents *sCmp, LXpShadeOutput *sOut, void *data) LXx_OVERRIDE;
        void			cmt_Cleanup       (void *data) LXx_OVERRIDE;
		LxResult		cmt_LinkSampleChannels(ILxUnknownID nodalEtor, ILxUnknownID item, int *idx) LXx_OVERRIDE;
		int 			cmt_IsSampleDriven(int *idx) LXx_OVERRIDE;
        /*
         * Channel UI Interface
         */
        LxResult		cui_Enabled (
                                        const char	*channelName,
                                        ILxUnknownID	 msg,
                                        ILxUnknownID	 item,
                                        ILxUnknownID	 read) LXx_OVERRIDE;

        LxResult		cui_DependencyCount (
                                        const char	*channelName,
                                        unsigned	*count) LXx_OVERRIDE;

        LxResult		cui_DependencyByIndex (
                                        const char	*channelName,
                                        unsigned	 index,
                                        LXtItemType	*depItemType,
                                        const char	**depChannelName) LXx_OVERRIDE;

        static double		GetLongAngle (LXtFVector hairTangent, LXtFVector rayDir);
        static double		GetAzimuthalAngle (LXtFVector hairTangent, LXtFVector rayInDir, LXtFVector rayOutDir);

        void			AdjustMaterialData (HairMaterialData &matrData);
        void			CopyHairPacket (LXpHairShader *sHair, HairMaterialData &target);

        /*
         * This is used just for debugging
         */
        void			PrintHairMaterialData (HairMaterialData *matrData);

        /*
         * Functions to do precomputations
         */

        void			PrecomputeNormFactors (HairMaterialData	*matrData);
        void			PrecomputeaBar_f ();
        void			PrecomputeaBar_b ();
        void			PrecomputeAlphaBar_f ();
        void			PrecomputeAlphaBar_b ();
        void			PrecomputeBetaBar_f ();
        void			PrecomputeBetaBar_b ();
        void			PrecomputeDeltaBar_b ();
        void			PrecomputeSigmaBar_b ();
        void			PrecomputeABar_b ();
        void			PrecomputeNG_R ();
        void			PrecomputeNG_TT ();
        void			PrecomputeNG_TRT ();
        void			PrecomputeShaderTables (HairMaterialData	*matrData);

        /*
         * These are the functions actually used in shading
         */

        void			ComputeGlobalScattering(ILxUnknownID		 vector, 
                                                        ILxUnknownID		 rayObj,
                                                        double			 importance,
                                                        LXtFVector		 rayDir,
                                                        int			 flags,
                                                        double			 lgtDist,
                                                        HairMaterialData	*matrData,
                                                        LXtVector		*T_f, 
                                                        LXtVector		*sigmaBarSqr_f, 
                                                        double			*directFraction,
                                                        LXtFVector		*shadeColor,
                                                        bool			 gi);
        void			ComputeGlobalScatteringGI(ILxUnknownID		 vector, 
                                                        ILxUnknownID		 rayObj,
                                                        double			 importance,
                                                        LXtFVector		 rayDir,
                                                        int			 flags,
                                                        HairMaterialData	*matrData,
                                                        LXtVector		*T_f, 
                                                        LXtVector		*sigmaBarSqr_f, 
                                                        double			*directFraction,
                                                        LXtFVector		*shadeColor);
        void			ComputeGlobalScatteringShadow(ILxUnknownID	 vector, 
                                                        ILxUnknownID		 rayObj,
                                                        double			 importance,
                                                        LXtFVector		 rayDir,
                                                        int			 flags,
                                                        double			 lgtDist,
                                                        HairMaterialData	*matrData,
                                                        LXtVector		*T_f, 
                                                        LXtVector		*sigmaBarSqr_f, 
                                                        double			*directFraction);
        void			ComputeFScatter_s (HairMaterialData		*matrData,
                                                        LXtVector		 sigmaBarSqr_f,
                                                        double			 theta_h,
                                                        double			 phi_d,
                                                        LXtVector		 FScatter_s);
        void			ComputeAmbientShading (LXtFVector		 w_i,
                                                        LXtFVector		 hairTangent,
                                                        LXtFVector		 ambientColor,
                                                        LXpShadeComponents	*sCmp);
        void			ComputeShadingForOneRay (LXtVector		 T_f, 
                                                        LXtVector		 sigmaBarSqr_f, 
                                                        double			 directFraction,
                                                        LXtFVector		 hairTangent,
                                                        LXtFVector		 w_i,
                                                        LXtFVector		 w_o,
                                                        double			 gAngle,
                                                        HairMaterialData	*matrData,
                                                        LXpShadeComponents	*sCmp);
        void			ComputeDirectShading (ILxUnknownID		 vector,
                                                        ILxUnknownID		 rayObj,
                                                        HairMaterialData	*matrData,
                                                        LXpShadeComponents	*sCmp);
        void			ComputeIndirectShading (ILxUnknownID		 vector,
                                                        ILxUnknownID		 rayObj,
                                                        HairMaterialData	*matrData,
                                                        LXpShadeComponents	*sCmp);

        // We use a mutex so that multiple threads don't precompute the tables
        // at once
        CLxLoc_ThreadMutex	precomputeMutex;

        /*
         * Functions to compute portions of the BSDF
         */

        double			M_R (double theta_h);
        double			M_TT (double theta_h);
        double			M_TRT (double theta_h);
        double			MG_R (double theta_h, double sigma);
        double			MG_TT (double theta_h, double sigma);
        double			MG_TRT (double theta_h, double sigma);
        double			N_R (double Phi);
        double			N_TT (double Phi);
        double			N_TRT (double Phi);
        double			N_GLINTS (double Phi, double gAngle);

        void			fPrime_s (double thetaIn, double thetaOut, 
                                        double phiIn, double phiOut, 
                                        double gAngle, HairMaterialData	*matrData, 
                                        LXtVector *outColor);
        void			fPrime_s (double thetaIn, double thetaOut, 
                                        double phiDiff, double gAngle, 
                                        HairMaterialData *matrData,
                                        LXtVector *outColor);
        void			fPrime_sNoGlints (double thetaIn, double thetaOut, 
                                        double phiIn, double phiOut, HairMaterialData *matrData,
                                        LXtVector *outColor);
        void			fPrime_sNoGlints (double thetaIn, double thetaOut, 
                                        double phiDiff, HairMaterialData *matrData,
                                        LXtVector *outColor);

        /*
         * Functions to lookup values in precomputed tables
         */

        double			lookupNormFactor (double thetaIn);
        void			lookupNormFactor (double theta_in, LXtVector *outColor);

        void			lookupValue (double theta, LXtVector *table, LXtVector *outColor);

        void			lookupaBar_f (double theta_in, LXtVector *outColor);
        void			lookupaBar_b (double theta_in, LXtVector *outColor);
        void			lookupAlphaBar_f (double theta_h, LXtVector *outColor);
        void			lookupAlphaBar_b (double theta_h, LXtVector *outColor);
        void			lookupBetaBar_f (double theta_h, LXtVector *outColor);
        void			lookupBetaBar_b (double theta_h, LXtVector *outColor);
        void			lookupDeltaBar_b (double theta_h, LXtVector *outColor);
        void			lookupSigmaBar_b (double theta_h, LXtVector *outColor);
        void			lookupABar_b (double theta_h, LXtVector *outColor);

        void			lookupNG_R (double phi_d, LXtVector *outColor);
        void			lookupNG_TT (double phi_d, LXtVector *outColor);
        void			lookupNG_TRT (double phi_d, LXtVector *outColor);

        void			ClearShadeComponents (LXpShadeComponents *sCmp);

        bool			isPrecomputed;

        // These are the precomputed tables the shader uses
        LXtVector		normFactors[TABLE_ENTRIES];
        LXtVector		aBar_f[TABLE_ENTRIES];
        LXtVector		aBar_b[TABLE_ENTRIES];
        LXtVector		AlphaBar_f[TABLE_ENTRIES];
        LXtVector		AlphaBar_b[TABLE_ENTRIES];
        LXtVector		BetaBar_f[TABLE_ENTRIES];
        LXtVector		BetaBar_b[TABLE_ENTRIES];
        LXtVector		DeltaBar_b[TABLE_ENTRIES];
        LXtVector		SigmaBar_b[TABLE_ENTRIES];
        LXtVector		ABar_b[TABLE_ENTRIES];

        // In the artist-friendly system, these are constant values
        LXtVector		NG_R[TABLE_ENTRIES];
        LXtVector		NG_TT[TABLE_ENTRIES];
        LXtVector		NG_TRT[TABLE_ENTRIES];

        HairMaterialData	matrData;
                
        LXtItemType		my_type; 
        LXtItemType		MyType ();

        CLxUser_PacketService	pkt_service;
		CLxUser_NodalService	nodalSvc;
		HairMaterialDataSampleIndex samplesIndex;

        unsigned		ray_offset, lcol_offset, nrm_offset, 
                                pos_offset, shade_opacity_offset, 
                                pix_offset, particle_sample_offset,
                                shade_flags_offset, global_indirect_offset, 
                                pkt_offset, global_lighting_offset, light_source_offset;
};

#define SRVs_HAIR_MATR		"hairMaterial"
#define SRVs_HAIR_MATR_ITEMTYPE	"material." SRVs_HAIR_MATR

LXtTagInfoDesc	 HairMaterial::descInfo[] = {
        { LXsSRV_USERNAME,	"Hair Material" },
        { LXsSRV_LOGSUBSYSTEM,	"comp-shader"	},
        { 0 }
};

const char* HAIR_MATERIAL_GI_TYPE = "gi";
const char* HAIR_MATERIAL_GI_RAYS = "giRays";

static LXtTextValueHint hint_PercentRange[] = {
        0,			"%min",		// float min 0.0
        -1,			NULL
        };
        
        int
HairMaterial::cmt_Flags ()
{
        return 0;
}
        LxResult
HairMaterial::cmt_SetupChannels (
        ILxUnknownID		 addChan)
{
        CLxUser_AddChannel	 ac (addChan);

        LXtVector defaultPrimColor = {.98, .92, .85};
        LXtVector defaultSecColor = {.53, .35, .17};
        LXtVector defaultRimColor = {.86, .54, .34};
        LXtVector defaultGlintColor = {.99, .75, .5};
        LXtVector white = {1.0, 1.0, 1.0};
     
        ac.NewChannel ("primHLCol", LXsTYPE_COLOR1);
        ac.SetVector  (LXsCHANVEC_RGB);
        ac.SetDefaultVec (defaultPrimColor);

        ac.NewChannel ("primHLIntensity", LXsTYPE_PERCENT);
        ac.SetDefault (.6, 0);
        ac.SetHint (hint_PercentRange);

        ac.NewChannel ("primHLLongWidth", LXsTYPE_PERCENT);
        ac.SetDefault (.03, 0);
        ac.SetHint (hint_PercentRange);

        ac.NewChannel ("primHLLongShift", LXsTYPE_PERCENT);
        ac.SetDefault (-.025, 0);

        ac.NewChannel ("secHLCol", LXsTYPE_COLOR1);
        ac.SetVector  (LXsCHANVEC_RGB);
        ac.SetDefaultVec (defaultSecColor);

        ac.NewChannel ("secHLIntensity", LXsTYPE_PERCENT);
        ac.SetDefault (.2, 0);
        ac.SetHint (hint_PercentRange);

        ac.NewChannel ("secHLLongWidth", LXsTYPE_PERCENT);
        ac.SetDefault (.2, 0);
        ac.SetHint (hint_PercentRange);

        ac.NewChannel ("secHLLongShift", LXsTYPE_PERCENT);
        ac.SetDefault (-.1, 0);

        ac.NewChannel ("rimCol", LXsTYPE_COLOR1);
        ac.SetVector  (LXsCHANVEC_RGB);
        ac.SetDefaultVec (defaultRimColor);

        ac.NewChannel ("rimIntensity", LXsTYPE_PERCENT);
        ac.SetDefault (.35, 0);
        ac.SetHint (hint_PercentRange);

        ac.NewChannel ("rimLongWidth", LXsTYPE_PERCENT);
        ac.SetDefault (.5, 0);
        ac.SetHint (hint_PercentRange);

        ac.NewChannel ("rimLongShift", LXsTYPE_PERCENT);
        ac.SetDefault (-.05, 0);

        ac.NewChannel ("rimAzWidth", LXsTYPE_PERCENT);
        ac.SetDefault (.5, 0);
        ac.SetHint (hint_PercentRange);

        ac.NewChannel ("glintsCol", LXsTYPE_COLOR1);
        ac.SetVector  (LXsCHANVEC_RGB);
        ac.SetDefaultVec (defaultGlintColor);

        ac.NewChannel ("glintsIntensity", LXsTYPE_PERCENT);
        ac.SetDefault (1.0, 0);
        ac.SetHint (hint_PercentRange);

        ac.NewChannel ("glintsFreq", LXsTYPE_PERCENT);
        ac.SetDefault (.001, 0);
        ac.SetHint (hint_PercentRange);

        ac.NewChannel ("fsColAdj", LXsTYPE_COLOR1);
        ac.SetVector  (LXsCHANVEC_RGB);
        ac.SetDefaultVec (white);

        ac.NewChannel ("fsIntAdj", LXsTYPE_FLOAT);
        ac.SetDefault (1.0, 0);

        ac.NewChannel ("fsSat", LXsTYPE_PERCENT);
        ac.SetDefault (1.0, 0);
        ac.SetHint (hint_PercentRange);

        ac.NewChannel ("bsColAdj", LXsTYPE_COLOR1);
        ac.SetVector  (LXsCHANVEC_RGB);
        ac.SetDefaultVec (white);

        ac.NewChannel ("bsIntAdj", LXsTYPE_FLOAT);
        ac.SetDefault (1.0, 0);

        ac.NewChannel ("bsSat", LXsTYPE_PERCENT);
        ac.SetDefault (1.0, 0);
        ac.SetHint (hint_PercentRange);

        ac.NewChannel ("bsLongWidthAdj", LXsTYPE_PERCENT);
        ac.SetDefault (0.0, 0);

        ac.NewChannel ("bsLongShift", LXsTYPE_PERCENT);
        ac.SetDefault (0.0, 0);

        ac.NewChannel ("gsSat", LXsTYPE_PERCENT);
        ac.SetDefault (1.0, 0);

        ac.NewChannel (HAIR_MATERIAL_GI_TYPE, LXsTYPE_INTEGER);
        ac.SetDefault (0.0, 3);
        ac.SetHint (hint_giType);

        ac.NewChannel (HAIR_MATERIAL_GI_RAYS, LXsTYPE_INTEGER);
        ac.SetDefault (0.0, 64);

        ac.NewChannel ("singleScattering", LXsTYPE_BOOLEAN);
        ac.SetDefault (0.0, 1);

        ac.NewChannel ("filterDist", LXsTYPE_DISTANCE);
        ac.SetDefault (.01, 0);

        return LXe_OK;
}

/*
 * Attach to channel evaluations.
 * This gets the indices for the channels in attributes.
 */
        LxResult
HairMaterial::cmt_LinkChannels (
        ILxUnknownID		 eval,
        ILxUnknownID		 item)
{
        CLxUser_Evaluation	 ev (eval);
		CLxUser_Item		 it(item);

		samplesIndex.primaryHighlightColor[0].chan = it.ChannelIndex("primHLCol.R");
		samplesIndex.primaryHighlightColor[1].chan = it.ChannelIndex("primHLCol.G");
		samplesIndex.primaryHighlightColor[2].chan = it.ChannelIndex("primHLCol.B");
		samplesIndex.primaryHighlightIntensity.chan = it.ChannelIndex("primHLIntensity");
		samplesIndex.primaryHighlightLongWidth.chan = it.ChannelIndex("primHLLongWidth");
		samplesIndex.primaryHighlightLongShift.chan = it.ChannelIndex("primHLLongShift"); //6

		samplesIndex.secondaryHighlightColor[0].chan = it.ChannelIndex("secHLCol.R");
		samplesIndex.secondaryHighlightColor[1].chan = it.ChannelIndex("secHLCol.G");
		samplesIndex.secondaryHighlightColor[2].chan = it.ChannelIndex("secHLCol.B");
        samplesIndex.secondaryHighlightIntensity.chan = it.ChannelIndex("secHLIntensity");
        samplesIndex.secondaryHighlightLongWidth.chan = it.ChannelIndex("secHLLongWidth");
        samplesIndex.secondaryHighlightLongShift.chan = it.ChannelIndex("secHLLongShift"); //12

		samplesIndex.rimLightColor[0].chan = it.ChannelIndex("rimCol.R");
		samplesIndex.rimLightColor[1].chan = it.ChannelIndex("rimCol.G");
		samplesIndex.rimLightColor[2].chan = it.ChannelIndex("rimCol.B");
        samplesIndex.rimLightIntensity.chan = it.ChannelIndex("rimIntensity");
        samplesIndex.rimLightLongWidth.chan = it.ChannelIndex("rimLongWidth");
        samplesIndex.rimLightLongShift.chan = it.ChannelIndex("rimLongShift");
        samplesIndex.rimLightAzimuthalWidth.chan = it.ChannelIndex("rimAzWidth"); //19

		samplesIndex.glintsColor[0].chan = it.ChannelIndex("glintsCol.R");
		samplesIndex.glintsColor[1].chan = it.ChannelIndex("glintsCol.G");
		samplesIndex.glintsColor[2].chan = it.ChannelIndex("glintsCol.B");
        samplesIndex.glintsIntensity.chan = it.ChannelIndex("glintsIntensity");
        samplesIndex.glintsFrequency.chan = it.ChannelIndex("glintsFreq"); //24

		samplesIndex.fwdScatteringColorAdj[0].chan = it.ChannelIndex("fsColAdj.R");
		samplesIndex.fwdScatteringColorAdj[1].chan = it.ChannelIndex("fsColAdj.G");
		samplesIndex.fwdScatteringColorAdj[2].chan = it.ChannelIndex("fsColAdj.B");
        samplesIndex.fwdScatteringIntensityAdj.chan = it.ChannelIndex("fsIntAdj"); //28
        samplesIndex.fwdScatteringSaturation.chan = it.ChannelIndex("fsSat");
        
		samplesIndex.bwdScatteringColorAdj[0].chan = it.ChannelIndex("bsColAdj.R");
		samplesIndex.bwdScatteringColorAdj[1].chan = it.ChannelIndex("bsColAdj.G");
		samplesIndex.bwdScatteringColorAdj[2].chan = it.ChannelIndex("bsColAdj.B");
        samplesIndex.bwdScatteringIntensityAdj.chan = it.ChannelIndex("bsIntAdj");
        samplesIndex.bwdScatteringLongWidthAdj.chan = it.ChannelIndex("bsLongWidthAdj");
        samplesIndex.bwdScatteringLongShift.chan = it.ChannelIndex("bsLongShift"); //35
        samplesIndex.bwdScatteringSaturation.chan = it.ChannelIndex("bsSat");

        samplesIndex.globalScatteringSaturation.chan = it.ChannelIndex("gsSat");

        samplesIndex.giType.chan = it.ChannelIndex("gi"); //38
        samplesIndex.giRays.chan = it.ChannelIndex("giRays"); //39

        samplesIndex.singleScatteringOn.chan = it.ChannelIndex("singleScattering"); //40

        samplesIndex.filterDist.chan = it.ChannelIndex("filterDist"); //41

		samplesIndex.primaryHighlightColor[0].layer = ev.AddChan(item, samplesIndex.primaryHighlightColor[0].chan);
		samplesIndex.primaryHighlightColor[1].layer = ev.AddChan(item, samplesIndex.primaryHighlightColor[1].chan);
		samplesIndex.primaryHighlightColor[2].layer = ev.AddChan(item, samplesIndex.primaryHighlightColor[2].chan);
		samplesIndex.primaryHighlightIntensity.layer = ev.AddChan(item, samplesIndex.primaryHighlightIntensity.chan);
		samplesIndex.primaryHighlightLongWidth.layer = ev.AddChan(item, samplesIndex.primaryHighlightLongWidth.chan);
		samplesIndex.primaryHighlightLongShift.layer = ev.AddChan(item, samplesIndex.primaryHighlightLongShift.chan); //6

		samplesIndex.secondaryHighlightColor[0].layer = ev.AddChan(item, samplesIndex.secondaryHighlightColor[0].chan);
		samplesIndex.secondaryHighlightColor[1].layer = ev.AddChan(item, samplesIndex.secondaryHighlightColor[1].chan);
		samplesIndex.secondaryHighlightColor[2].layer = ev.AddChan(item, samplesIndex.secondaryHighlightColor[2].chan);
		samplesIndex.secondaryHighlightIntensity.layer = ev.AddChan(item, samplesIndex.secondaryHighlightIntensity.chan);
		samplesIndex.secondaryHighlightLongWidth.layer = ev.AddChan(item, samplesIndex.secondaryHighlightLongWidth.chan);
		samplesIndex.secondaryHighlightLongShift.layer = ev.AddChan(item, samplesIndex.secondaryHighlightLongShift.chan); //12

		samplesIndex.rimLightColor[0].layer = ev.AddChan(item, samplesIndex.rimLightColor[0].chan);
		samplesIndex.rimLightColor[1].layer = ev.AddChan(item, samplesIndex.rimLightColor[1].chan);
		samplesIndex.rimLightColor[2].layer = ev.AddChan(item, samplesIndex.rimLightColor[2].chan);
		samplesIndex.rimLightIntensity.layer = ev.AddChan(item, samplesIndex.rimLightIntensity.chan);
		samplesIndex.rimLightLongWidth.layer = ev.AddChan(item, samplesIndex.rimLightLongWidth.chan);
		samplesIndex.rimLightLongShift.layer = ev.AddChan(item, samplesIndex.rimLightLongShift.chan);
		samplesIndex.rimLightAzimuthalWidth.layer = ev.AddChan(item, samplesIndex.rimLightAzimuthalWidth.chan); //19

		samplesIndex.glintsColor[0].layer = ev.AddChan(item, samplesIndex.glintsColor[0].chan);
		samplesIndex.glintsColor[1].layer = ev.AddChan(item, samplesIndex.glintsColor[1].chan);
		samplesIndex.glintsColor[2].layer = ev.AddChan(item, samplesIndex.glintsColor[2].chan);
		samplesIndex.glintsIntensity.layer = ev.AddChan(item, samplesIndex.glintsIntensity.chan);
		samplesIndex.glintsFrequency.layer = ev.AddChan(item, samplesIndex.glintsFrequency.chan); //24

		samplesIndex.fwdScatteringColorAdj[0].layer = ev.AddChan(item, samplesIndex.fwdScatteringColorAdj[0].chan);
		samplesIndex.fwdScatteringColorAdj[1].layer = ev.AddChan(item, samplesIndex.fwdScatteringColorAdj[1].chan);
		samplesIndex.fwdScatteringColorAdj[2].layer = ev.AddChan(item, samplesIndex.fwdScatteringColorAdj[2].chan);
		samplesIndex.fwdScatteringIntensityAdj.layer = ev.AddChan(item, samplesIndex.fwdScatteringIntensityAdj.chan); //28
		samplesIndex.fwdScatteringSaturation.layer = ev.AddChan(item, samplesIndex.fwdScatteringSaturation.chan);

		samplesIndex.bwdScatteringColorAdj[0].layer = ev.AddChan(item, samplesIndex.bwdScatteringColorAdj[0].chan);
		samplesIndex.bwdScatteringColorAdj[1].layer = ev.AddChan(item, samplesIndex.bwdScatteringColorAdj[1].chan);
		samplesIndex.bwdScatteringColorAdj[2].layer = ev.AddChan(item, samplesIndex.bwdScatteringColorAdj[2].chan);
		samplesIndex.bwdScatteringIntensityAdj.layer = ev.AddChan(item, samplesIndex.bwdScatteringIntensityAdj.chan);
		samplesIndex.bwdScatteringLongWidthAdj.layer = ev.AddChan(item, samplesIndex.bwdScatteringLongWidthAdj.chan);
		samplesIndex.bwdScatteringLongShift.layer = ev.AddChan(item, samplesIndex.bwdScatteringLongShift.chan); //35
		samplesIndex.bwdScatteringSaturation.layer = ev.AddChan(item, samplesIndex.bwdScatteringSaturation.chan);

		samplesIndex.globalScatteringSaturation.layer = ev.AddChan(item, samplesIndex.globalScatteringSaturation.chan);

		samplesIndex.giType.layer = ev.AddChan(item, samplesIndex.giType.chan); //38
		samplesIndex.giRays.layer = ev.AddChan(item, samplesIndex.giRays.chan); //39

		samplesIndex.singleScatteringOn.layer = ev.AddChan(item, samplesIndex.singleScatteringOn.chan); //40

		samplesIndex.filterDist.layer = ev.AddChan(item, samplesIndex.filterDist.chan); //41

        ray_offset		= pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SAMPLE_RAY);
        nrm_offset		= pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SURF_NORMAL);
        pos_offset		= pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SAMPLE_POSITION);
        shade_opacity_offset	= pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SHADE_OPACITY);
        particle_sample_offset	= pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_PARTICLE_SAMPLE);
        shade_flags_offset	= pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SHADE_FLAGS);
        global_indirect_offset	= pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_GLO_INDIRECT);
        light_source_offset	= pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_LGT_SOURCE);
        global_lighting_offset	= pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_GLO_LIGHTING);
        pkt_offset		= pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SAMPLE_HAIRSHADER);

        return LXe_OK;
}

/*
 * Read channel values which may have changed.
 */
        LxResult
HairMaterial::cmt_ReadChannels (
        ILxUnknownID		 attr,
        void		       **ppvData)
{
        CLxUser_Attributes	at (attr);
        HairMaterialData*       md = new HairMaterialData;
        HairMaterialData	adjustedData;
 
        bool			changed = false;
        for (int i = 0; i < 3; i++)
			md->primaryHighlightColor[i] = at.Float(samplesIndex.primaryHighlightColor[i].layer);
		md->primaryHighlightIntensity = at.Float(samplesIndex.primaryHighlightIntensity.layer);
		md->primaryHighlightLongWidth = at.Float(samplesIndex.primaryHighlightLongWidth.layer);
		md->primaryHighlightLongShift = at.Float(samplesIndex.primaryHighlightLongShift.layer); //6

        for (int i = 0; i < 3; i++)
			md->secondaryHighlightColor[i] = at.Float(samplesIndex.secondaryHighlightColor[i].layer);
		md->secondaryHighlightIntensity = at.Float(samplesIndex.secondaryHighlightIntensity.layer);
		md->secondaryHighlightLongWidth = at.Float(samplesIndex.secondaryHighlightLongWidth.layer);
		md->secondaryHighlightLongShift = at.Float(samplesIndex.secondaryHighlightLongShift.layer); //12

        for (int i = 0; i < 3; i++)
			md->rimLightColor[i] = at.Float(samplesIndex.rimLightColor[i].layer);
		md->rimLightIntensity = at.Float(samplesIndex.rimLightIntensity.layer);
		md->rimLightLongWidth = at.Float(samplesIndex.rimLightLongWidth.layer);
		md->rimLightLongShift = at.Float(samplesIndex.rimLightLongShift.layer);
		md->rimLightAzimuthalWidth = at.Float(samplesIndex.rimLightAzimuthalWidth.layer); //19
        
        for (int i = 0; i < 3; i++)
			md->glintsColor[i] = at.Float(samplesIndex.glintsColor[i].layer);
		md->glintsIntensity = at.Float(samplesIndex.glintsIntensity.layer);
		md->glintsFrequency = at.Float(samplesIndex.glintsFrequency.layer); //24

        for (int i = 0; i < 3; i++)
			md->fwdScatteringColorAdj[i] = at.Float(samplesIndex.fwdScatteringColorAdj[i].layer);
		md->fwdScatteringIntensityAdj = at.Float(samplesIndex.fwdScatteringIntensityAdj.layer); //28
		md->fwdScatteringSaturation = at.Float(samplesIndex.fwdScatteringSaturation.layer);

        for (int i = 0; i < 3; i++)
			md->bwdScatteringColorAdj[i] = at.Float(samplesIndex.bwdScatteringColorAdj[i].layer);
		md->bwdScatteringIntensityAdj = at.Float(samplesIndex.bwdScatteringIntensityAdj.layer);
		md->bwdScatteringLongWidthAdj = at.Float(samplesIndex.bwdScatteringLongWidthAdj.layer);
		md->bwdScatteringLongShift = at.Float(samplesIndex.bwdScatteringLongShift.layer);
		md->bwdScatteringSaturation = at.Float(samplesIndex.bwdScatteringSaturation.layer); //36

		md->globalScatteringSaturation = at.Float(samplesIndex.globalScatteringSaturation.layer);

		md->giType = at.Int(samplesIndex.giType.layer); //38
		md->giRays = at.Int(samplesIndex.giRays.layer); //39

		md->singleScatteringOn = at.Int(samplesIndex.singleScatteringOn.layer); //40

		md->filterDist = at.Float(samplesIndex.filterDist.layer); //41

        // Since the widths represent variances, by squaring the input value we make
        // the input values std. deviations, which are more intuitive
        md->primaryHighlightLongWidth		*= md->primaryHighlightLongWidth;
        md->secondaryHighlightLongWidth		*= md->secondaryHighlightLongWidth;
        md->rimLightLongWidth				*= md->rimLightLongWidth;
        md->rimLightAzimuthalWidth			*= md->rimLightAzimuthalWidth;
        md->bwdScatteringLongWidthAdj		*= md->bwdScatteringLongWidthAdj;

        /*
         * Now we need to see if anything has changed
         */

        adjustedData = *md;
        AdjustMaterialData(adjustedData);

        for (int i = 0; i < 3; i++)
                changed |= (adjustedData.primaryHighlightColor[i] != this->matrData.primaryHighlightColor[i]);
        changed |= (adjustedData.primaryHighlightIntensity != this->matrData.primaryHighlightIntensity);
        changed |= (adjustedData.primaryHighlightLongWidth != this->matrData.primaryHighlightLongWidth);
        changed |= (adjustedData.primaryHighlightLongShift != this->matrData.primaryHighlightLongShift);

        for (int i = 0; i < 3; i++)
                changed |= (adjustedData.secondaryHighlightColor[i] != this->matrData.secondaryHighlightColor[i]);
        changed |= (adjustedData.secondaryHighlightIntensity != this->matrData.secondaryHighlightIntensity);
        changed |= (adjustedData.secondaryHighlightLongWidth != this->matrData.secondaryHighlightLongWidth);
        changed |= (adjustedData.secondaryHighlightLongShift != this->matrData.secondaryHighlightLongShift);

        for (int i = 0; i < 3; i++)
                changed |= (adjustedData.rimLightColor[i] != this->matrData.rimLightColor[i]);
        changed |= (adjustedData.rimLightIntensity != this->matrData.rimLightIntensity);
        changed |= (adjustedData.rimLightLongWidth != this->matrData.rimLightLongWidth);
        changed |= (adjustedData.rimLightLongShift != this->matrData.rimLightLongShift);
        changed |= (adjustedData.rimLightAzimuthalWidth != this->matrData.rimLightAzimuthalWidth);

        for (int i = 0; i < 3; i++)
                changed |= (adjustedData.glintsColor[i] != this->matrData.glintsColor[i]);
        changed |= (adjustedData.glintsIntensity != this->matrData.glintsIntensity);
        changed |= (adjustedData.glintsFrequency != this->matrData.glintsFrequency);

        changed |= (adjustedData.fwdScatteringIntensityAdj != this->matrData.fwdScatteringIntensityAdj);

        changed |= (adjustedData.bwdScatteringIntensityAdj != this->matrData.bwdScatteringIntensityAdj);
        changed |= (adjustedData.bwdScatteringLongWidthAdj != this->matrData.bwdScatteringLongWidthAdj);
        changed |= (adjustedData.bwdScatteringLongShift != this->matrData.bwdScatteringLongShift);

        if (changed) {
                this->precomputeMutex.Enter();
                this->matrData = adjustedData;
                this->isPrecomputed = false;
                this->precomputeMutex.Leave();
        }

        ppvData[0] = md;
        return LXe_OK;
}

        LxResult
HairMaterial::cmt_CustomPacket (
        const char		**packet)
{
        packet[0] = LXsP_SAMPLE_HAIRSHADER;
        return LXe_OK;
}

        /*
 * These are used to set up the channel dependencies so that hair GI is
 * disabled if GI is disabled, and the rays is disabled unless hair GI
 * casting is enabled
 */

const char* HAIR_MATERIAL_MSG_TABLE = "material.hairMaterial";

const unsigned HAIR_MATERIAL_GI_RAYS_ENABLED = 1001;
const unsigned HAIR_MATERIAL_GI_ENABLED = 1002;

        LxResult
HairMaterial::cui_Enabled (
        const char	*channelName,
        ILxUnknownID	 msg,
        ILxUnknownID	 item,
        ILxUnknownID	 read)
{
        LxResult		result = LXe_OK;
        CLxUser_SceneService	svc;
        CLxUser_Scene		scene;
        CLxUser_Item		src (item);
        CLxUser_Item		rndr;

        LXtItemType		renderType = svc.ItemType (LXsITYPE_POLYRENDER);

        src.GetContext (scene);
        scene.GetItem (renderType, 0, rndr);

        if ((strcmp (channelName, HAIR_MATERIAL_GI_RAYS) == 0)){
                
                CLxUser_ChannelRead	 chan (read);

                int giType = chan.IValue (src, HAIR_MATERIAL_GI_TYPE);
                bool giEnabled = chan.IValue (rndr, LXsICHAN_RENDER_GLOBENABLE);
                if (!giEnabled) {
                        CLxUser_Message		 res (msg);

                        res.SetCode (LXe_CMD_DISABLED);
                        res.SetMsg  (HAIR_MATERIAL_MSG_TABLE, HAIR_MATERIAL_GI_ENABLED);

                        result = LXe_CMD_DISABLED;
                }
                else if (giType == 0 || giType == 2) {
                        CLxUser_Message		 res (msg);

                        res.SetCode (LXe_CMD_DISABLED);
                        res.SetMsg  (HAIR_MATERIAL_MSG_TABLE, HAIR_MATERIAL_GI_RAYS_ENABLED);

                        result = LXe_CMD_DISABLED;
                }
        }
        else if ((strcmp (channelName, HAIR_MATERIAL_GI_TYPE) == 0)){
                
                CLxUser_ChannelRead	 chan (read);

                bool giEnabled = chan.IValue (rndr, LXsICHAN_RENDER_GLOBENABLE);
                if (!giEnabled) {
                        CLxUser_Message		 res (msg);

                        res.SetCode (LXe_CMD_DISABLED);
                        res.SetMsg  (HAIR_MATERIAL_MSG_TABLE, HAIR_MATERIAL_GI_ENABLED);

                        result = LXe_CMD_DISABLED;
                }
        }

        return result;
}

        LxResult
HairMaterial::cui_DependencyCount (
        const char	*channelName,
        unsigned	*count)
{
        if ((strcmp (channelName, HAIR_MATERIAL_GI_RAYS) == 0) || 
                (strcmp (channelName, HAIR_MATERIAL_GI_TYPE) == 0)) {
                count[0] = 1;
        }
        else {
                count[0] = 0;
        }

        return LXe_OK;
}

        LxResult
HairMaterial::cui_DependencyByIndex (
        const char	*channelName,
        unsigned	 index,
        LXtItemType	*depItemType,
        const char	**depChannelName)
{
        LxResult	result = LXe_OUTOFBOUNDS;

        if ((strcmp (channelName, HAIR_MATERIAL_GI_RAYS) == 0)) {
                depItemType[0] = MyType ();
                switch (index) {
                    case 0:
                        depChannelName[0] = HAIR_MATERIAL_GI_TYPE;
                        result = LXe_OK;
                        break;

                    default:
                        result = LXe_OUTOFBOUNDS;
                        break;
                }
        }
        else if ((strcmp (channelName, HAIR_MATERIAL_GI_TYPE) == 0)) {
                depItemType[0] = MyType ();
                switch (index) {
                    case 0:
                        depChannelName[0] = HAIR_MATERIAL_GI_TYPE;
                        result = LXe_OK;
                        break;

                    default:
                        result = LXe_OUTOFBOUNDS;
                        break;
                }
        }

        return result;
}

	LxResult
HairMaterial::cmt_LinkSampleChannels(ILxUnknownID eval, ILxUnknownID item, int *idx)
{
	/*nodalSvc.AddSampleChan(eval, item, samplesIndex.primaryHighlightColor[0].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.primaryHighlightColor[1].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.primaryHighlightColor[2].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.primaryHighlightIntensity.chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.primaryHighlightLongWidth.chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.primaryHighlightLongShift.chan, idx, LXiNODAL_NOT_DRIVEN); //6

	nodalSvc.AddSampleChan(eval, item, samplesIndex.secondaryHighlightColor[0].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.secondaryHighlightColor[1].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.secondaryHighlightColor[2].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.secondaryHighlightIntensity.chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.secondaryHighlightLongWidth.chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.secondaryHighlightLongShift.chan, idx, LXiNODAL_NOT_DRIVEN); //12

	nodalSvc.AddSampleChan(eval, item, samplesIndex.rimLightColor[0].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.rimLightColor[1].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.rimLightColor[2].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.rimLightIntensity.chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.rimLightLongWidth.chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.rimLightLongShift.chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.rimLightAzimuthalWidth.chan, idx, LXiNODAL_NOT_DRIVEN); //19

	nodalSvc.AddSampleChan(eval, item, samplesIndex.glintsColor[0].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.glintsColor[1].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.glintsColor[2].chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.glintsIntensity.chan, idx, LXiNODAL_NOT_DRIVEN);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.glintsFrequency.chan, idx, LXiNODAL_NOT_DRIVEN); //24*/

	nodalSvc.AddSampleChan(eval, item, samplesIndex.fwdScatteringColorAdj[0].chan, idx, LXfECHAN_READ);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.fwdScatteringColorAdj[1].chan, idx, LXfECHAN_READ);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.fwdScatteringColorAdj[2].chan, idx, LXfECHAN_READ);
	//nodalSvc.AddSampleChan(eval, item, samplesIndex.fwdScatteringIntensityAdj.chan, idx, LXiNODAL_NOT_DRIVEN); //28
	nodalSvc.AddSampleChan(eval, item, samplesIndex.fwdScatteringSaturation.chan, idx, LXfECHAN_READ);

	nodalSvc.AddSampleChan(eval, item, samplesIndex.bwdScatteringColorAdj[0].chan, idx, LXfECHAN_READ);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.bwdScatteringColorAdj[1].chan, idx, LXfECHAN_READ);
	nodalSvc.AddSampleChan(eval, item, samplesIndex.bwdScatteringColorAdj[2].chan, idx, LXfECHAN_READ);
	//nodalSvc.AddSampleChan(eval, item, samplesIndex.bwdScatteringIntensityAdj.chan, idx, LXiNODAL_NOT_DRIVEN);
	//nodalSvc.AddSampleChan(eval, item, samplesIndex.bwdScatteringLongWidthAdj.chan, idx, LXiNODAL_NOT_DRIVEN);
	//nodalSvc.AddSampleChan(eval, item, samplesIndex.bwdScatteringLongShift.chan, idx, LXiNODAL_NOT_DRIVEN); //35
	nodalSvc.AddSampleChan(eval, item, samplesIndex.bwdScatteringSaturation.chan, idx, LXfECHAN_READ);

	nodalSvc.AddSampleChan(eval, item, samplesIndex.globalScatteringSaturation.chan, idx, LXfECHAN_READ);

	nodalSvc.AddSampleChan(eval, item, samplesIndex.giType.chan, idx, LXfECHAN_READ); //38
	nodalSvc.AddSampleChan(eval, item, samplesIndex.giRays.chan, idx, LXfECHAN_READ); //39

	nodalSvc.AddSampleChan(eval, item, samplesIndex.singleScatteringOn.chan, idx, LXfECHAN_READ); //40

	nodalSvc.AddSampleChan(eval, item, samplesIndex.filterDist.chan, idx, LXfECHAN_READ); //41

	return LXe_OK;
}

	int
HairMaterial::cmt_IsSampleDriven(int *idx)
{
	return nodalSvc.AnyDrivenChans(&idx[samplesIndex.fwdScatteringColorAdj[0].chan], 13);
}

/*
 * Evaluate the color at a spot.
 */
        void
HairMaterial::cmt_MaterialEvaluate (
		ILxUnknownID etor, 
		int *idx, 
		ILxUnknownID    vector,
        void			*data)
{
		LXpHairShader		*sHair = (LXpHairShader*) pkt_service.FastPacket (vector, pkt_offset);
		HairMaterialData*        md = (HairMaterialData*)data;

		// Copy the hair shader settings 
		sHair->matrData = *md; 
		 
		for (int i = 0; i < 3; i++)
			sHair->matrData.fwdScatteringColorAdj[i] = nodalSvc.GetFloat(etor, idx, samplesIndex.fwdScatteringColorAdj[i].chan, sHair->matrData.fwdScatteringColorAdj[i]);
		sHair->matrData.fwdScatteringSaturation = nodalSvc.GetFloat(etor, idx, samplesIndex.fwdScatteringSaturation.chan, sHair->matrData.fwdScatteringSaturation);

		for (int i = 0; i < 3; i++)
			sHair->matrData.bwdScatteringColorAdj[i] = nodalSvc.GetFloat(etor, idx, samplesIndex.bwdScatteringColorAdj[i].chan, sHair->matrData.bwdScatteringColorAdj[i]);
		sHair->matrData.bwdScatteringSaturation = nodalSvc.GetFloat(etor, idx, samplesIndex.bwdScatteringSaturation.chan, sHair->matrData.bwdScatteringSaturation); //36

		sHair->matrData.globalScatteringSaturation = nodalSvc.GetFloat(etor, idx, samplesIndex.globalScatteringSaturation.chan, sHair->matrData.globalScatteringSaturation);

		sHair->matrData.giType = nodalSvc.GetInt(etor, idx, samplesIndex.giType.chan, sHair->matrData.giType); //38
		sHair->matrData.giRays = nodalSvc.GetInt(etor, idx, samplesIndex.giRays.chan, sHair->matrData.giRays); //39

		sHair->matrData.singleScatteringOn = nodalSvc.GetInt(etor, idx, samplesIndex.singleScatteringOn.chan, sHair->matrData.singleScatteringOn); //40

		sHair->matrData.filterDist = nodalSvc.GetFloat(etor, idx, samplesIndex.filterDist.chan, sHair->matrData.filterDist); //41
}

		void
HairMaterial::cmt_Cleanup (
        void			*data)
{
        HairMaterialData*       md = (HairMaterialData*)data;

        delete md;
}


        LXtItemType
HairMaterial::MyType ()
{
        if (my_type != LXiTYPE_NONE)
                return my_type;

        CLxUser_SceneService	 svc;

        my_type = svc.ItemType (SRVs_HAIR_MATR_ITEMTYPE);
        return my_type;
}

/*------------------------------- Luxology LLC --------------------------------
 *
 * These are some ad-hoc adjustments to make the values more intuitive
 *
 *----------------------------------------------------------------------------*/
        void
HairMaterial::AdjustMaterialData (
        HairMaterialData	&matrData)
{
        matrData.primaryHighlightIntensity *= .5;
        matrData.secondaryHighlightIntensity *= .5;
        matrData.rimLightIntensity *= .5;

        matrData.primaryHighlightLongShift *= -1.0;
        matrData.secondaryHighlightLongShift *= -1.0;
        matrData.rimLightLongShift *= -1.0;
        matrData.bwdScatteringLongShift *= -1.0;
        matrData.glintsFrequency *= .1;
}
/*------------------------------- Luxology LLC --------------------------------
 *
 * This function copies all the values over from the hair packet, and determines
 * whether something has changed
 *
 *----------------------------------------------------------------------------*/
        void
HairMaterial::CopyHairPacket (
        LXpHairShader		*sHair,
        HairMaterialData	&target)
{
        bool			 changed = false;

        HairMaterialData	 matrData;

        matrData = sHair->matrData;

        AdjustMaterialData (matrData);

        changed |= (this->matrData.primaryHighlightIntensity != matrData.primaryHighlightIntensity);
        changed |= (this->matrData.primaryHighlightLongWidth != matrData.primaryHighlightLongWidth);
        changed |= (this->matrData.primaryHighlightLongShift != matrData.primaryHighlightLongShift);

        changed |= (this->matrData.secondaryHighlightIntensity != matrData.secondaryHighlightIntensity);
        changed |= (this->matrData.secondaryHighlightLongWidth != matrData.secondaryHighlightLongWidth);
        changed |= (this->matrData.secondaryHighlightLongShift != matrData.secondaryHighlightLongShift);

        changed |= (this->matrData.rimLightIntensity != matrData.rimLightIntensity);
        changed |= (this->matrData.rimLightLongWidth != matrData.rimLightLongWidth);
        changed |= (this->matrData.rimLightLongShift != matrData.rimLightLongShift);
        changed |= (this->matrData.rimLightAzimuthalWidth != matrData.rimLightAzimuthalWidth);

        changed |= (this->matrData.secondaryHighlightIntensity != matrData.secondaryHighlightIntensity);
        changed |= (this->matrData.secondaryHighlightLongWidth != matrData.secondaryHighlightLongWidth);
        changed |= (this->matrData.secondaryHighlightLongShift != matrData.secondaryHighlightLongShift);

        changed |= (this->matrData.glintsIntensity != matrData.glintsIntensity);
        changed |= (this->matrData.glintsFrequency != matrData.glintsFrequency);

        changed |= (this->matrData.fwdScatteringIntensityAdj != matrData.fwdScatteringIntensityAdj);

        changed |= (this->matrData.bwdScatteringIntensityAdj != matrData.bwdScatteringIntensityAdj);
        changed |= (this->matrData.bwdScatteringLongWidthAdj != matrData.bwdScatteringLongWidthAdj);
        changed |= (this->matrData.bwdScatteringLongShift != matrData.bwdScatteringLongShift);

        target = matrData;
}

/*------------------------------- Luxology LLC --------------------------------
 *
 * Useful utilities
 *
 *----------------------------------------------------------------------------*/

        static double
FVectorNormalize (
        LXtFVector		v) 
{
        double		m, p;
        m = LXx_VDOT (v, v);
        if(m<=0)
                return -1;
        m = sqrt (m);
        p = 1.0 / m;
        LXx_VSCL (v, p);
        return m;
}

        static double
UnitAreaGaussian (
        double			variance,
        double			x)
{
        if (variance <= 0.0)
                return 0.0;

        return exp (-(x*x/(2 * variance))) / (sqrtf (2.0 * LXx_PI * variance));
}

        static double
UnitHeightGaussian (
        double			variance,
        double			x)
{
        if (variance <= 0.0)
                return 0.0;

        return exp (-(x*x/(2 * variance)));
}

        double
HairMaterial::GetLongAngle (
        LXtFVector		hairTangent,
        LXtFVector		rayDir)
{
        double			dot, angle, tanLen, outLen;

        tanLen = LXx_VLEN (hairTangent);
        outLen = LXx_VLEN (rayDir);

        if (outLen > .001 && tanLen > .001) {
                dot = LXx_VDOT (hairTangent, rayDir) / (tanLen * outLen);
                dot = CLAMP (dot, -1.0, 1.0);
        }
        else {
                dot = 0.0;
        }

        angle = acos(dot);

        return angle - LXx_HALFPI;
}

        double
HairMaterial::GetAzimuthalAngle (
        LXtFVector		hairTangent,
        LXtFVector		rayInDir,
        LXtFVector		rayOutDir)
{
        double			dot, inLen, outLen;

        LXtFVector		crossIn, crossOut;

        LXx_VCROSS (crossIn, hairTangent, rayInDir);
        LXx_VCROSS (crossOut, hairTangent, rayOutDir);

        inLen = LXx_VLEN (crossIn);
        outLen = LXx_VLEN (crossOut);

        if (inLen > .001 && outLen > .001) {
                dot = LXx_VDOT (crossIn, crossOut) / (inLen * outLen);
                dot = CLAMP (dot, -1.0, 1.0);
        }
        else {
                dot = 0.0;
        }
        return acos(dot);
}

        void
Desaturate (
        LXtVector		color,
        double			saturation)
{
        double avgValue;
        LXtVector avgColor;

        avgValue = (color[0] + color[1] + color[2])/3.0;
        LXx_VSET (avgColor, avgValue);
        LXx_VLERP (color, avgColor, color, saturation);
}

/*------------------------------- Luxology LLC ---------------------------
 *
 * Functions to compute factors of the BSDF
 *
 *----------------------------------------------------------------------------*/
        double
HairMaterial::M_R (
        double			 theta_h)
{
        return UnitHeightGaussian (this->matrData.primaryHighlightLongWidth,
                                        theta_h - this->matrData.primaryHighlightLongShift);
}

        double
HairMaterial::M_TT (
        double			 theta_h)
{
        return UnitHeightGaussian (this->matrData.rimLightLongWidth,
                                        theta_h - this->matrData.rimLightLongShift);
}

        double
HairMaterial::M_TRT (
        double			 theta_h)
{
        return UnitHeightGaussian	(this->matrData.secondaryHighlightLongWidth,
                                        theta_h - this->matrData.secondaryHighlightLongShift);
}

        double
HairMaterial::MG_R (
        double			 theta_h,
        double			 sigma)
{
        return UnitHeightGaussian	(this->matrData.primaryHighlightLongWidth + sigma,
                                        theta_h - this->matrData.primaryHighlightLongShift);
}

        double
HairMaterial::MG_TT (
        double			 theta_h,
        double			 sigma)
{
        return UnitHeightGaussian	(this->matrData.rimLightLongWidth + sigma,
                                        theta_h - this->matrData.rimLightLongShift);
}

        double
HairMaterial::MG_TRT (
        double			 theta_h,
        double			 sigma)
{
        return UnitHeightGaussian	(this->matrData.secondaryHighlightLongWidth + sigma,
                                        theta_h - this->matrData.secondaryHighlightLongShift);
}

        double
HairMaterial::N_R (
        double			 Phi)
{
        Phi = Phi < -LXx_PI ? Phi + 2.0 * LXx_PI : Phi;		// Convert (-pi, -2pi) to (0, pi)
        Phi = Phi > LXx_PI ? Phi - 2.0 * LXx_PI : Phi;		// Convert (pi, 2pi) to (0, -pi) 

        return MAX(0.0, cos (Phi/2.0));
}

        double
HairMaterial::N_TT (
        double			 Phi)
{
        double g;

        Phi = Phi < -LXx_PI ? Phi + 2.0 * LXx_PI : Phi;		// Convert (-pi, -2pi) to (0, pi)
        Phi = Phi > LXx_PI ? Phi - 2.0 * LXx_PI : Phi;		// Convert (pi, 2pi) to (0, -pi) 

        // Reverse to be centered on pi
        Phi = LXx_PI - fabs(Phi);

        g = UnitHeightGaussian (this->matrData.rimLightAzimuthalWidth, Phi);

        return (g * (.5 *cos (Phi) + .5));
}

        double
HairMaterial::N_TRT (
        double			 Phi)
{
        Phi = Phi < -LXx_PI ? Phi + 2.0 * LXx_PI : Phi;		// Convert (-pi, -2pi) to (0, pi)
        Phi = Phi > LXx_PI ? Phi - 2.0 * LXx_PI : Phi;		// Convert (pi, 2pi) to (0, -pi) 

        return MAX(0.0, cos (Phi/2.0));
}

        double
HairMaterial::N_GLINTS (
        double			Phi,
        double			gAngle)
{
        Phi = Phi < -LXx_PI ? Phi + 2.0 * LXx_PI : Phi;		// Convert (-pi, -2pi) to (0, pi)
        Phi = Phi > LXx_PI ? Phi - 2.0 * LXx_PI : Phi;		// Convert (pi, 2pi) to (0, -pi) 

        return (UnitHeightGaussian (this->matrData.glintsFrequency, gAngle - Phi));
}

        void
HairMaterial::fPrime_s (
        double			thetaIn,
        double			thetaOut,
        double			phiIn,
        double			phiOut,
        double			gAngle,
        HairMaterialData	*matrData,
        LXtVector		*outColor)
{
        // fPrime_s is our BCSDF, taken from "An Artist Friendly Hair Shading System"
        LXtVector tempColor;
        double theta_D = (thetaOut - thetaIn)/2.0;
        double theta_H = (thetaOut + thetaIn)/2.0;
        double phi_D = (phiOut - phiIn);

        double cosTheta = cos (theta_D);
        double cosTheta2 = cosTheta * cosTheta;


        if (phi_D < 0) {
                phi_D = fabs (phi_D);
        }
        if (phi_D > LXx_PI) {
                phi_D = 2.0 * LXx_PI - phi_D;
        }

        LXx_VCLR(*outColor);

        LXx_VCPY(tempColor, matrData->primaryHighlightColor);
        LXx_VSCL(tempColor, this->N_R (phi_D));
        LXx_VSCL(tempColor, this->M_R (theta_H));
        LXx_VSCL(tempColor, matrData->primaryHighlightIntensity);
        LXx_VADD(*outColor, tempColor);
        
        LXx_VCPY(tempColor, matrData->secondaryHighlightColor);
        LXx_VSCL(tempColor, this->N_TRT (phi_D));
        LXx_VSCL(tempColor, this->M_TRT (theta_H));
        LXx_VSCL(tempColor, matrData->secondaryHighlightIntensity);
        LXx_VADD(*outColor, tempColor);

        LXx_VCPY(tempColor, matrData->glintsColor);
        LXx_VSCL(tempColor, this->N_GLINTS (phi_D, gAngle));
        LXx_VSCL(tempColor, this->M_TRT (theta_H));
        LXx_VSCL(tempColor, matrData->glintsIntensity);
        // This is in the paper, but it's unintuitive so I'm taking it out
        // LXx_VSCL(tempColor, matrData->secondaryHighlightIntensity); 
        LXx_VADD(*outColor, tempColor);

        LXx_VCPY(tempColor, matrData->rimLightColor);
        LXx_VSCL(tempColor, this->N_TT (phi_D));
        LXx_VSCL(tempColor, this->M_TT (theta_H));
        LXx_VSCL(tempColor, matrData->rimLightIntensity);
        LXx_VADD(*outColor, tempColor);

        LXx_VSCL(*outColor, 1.0/cosTheta2);
}

        void
HairMaterial::fPrime_sNoGlints (
        double			thetaIn,
        double			thetaOut,
        double			phiIn,
        double			phiOut,
        HairMaterialData	*matrData,
        LXtVector		*outColor)
{
        // fPrime_s is our BCSDF, taken from "An Artist Friendly Hair Shading System"
        LXtVector tempColor;
        double theta_D = (thetaOut - thetaIn)/2.0;
        double theta_H = (thetaOut + thetaIn)/2.0;
        double phi_D = (phiOut - phiIn);

        double cosTheta = cos (theta_D);
        double cosTheta2 = cosTheta * cosTheta;


        if (phi_D < 0) {
                phi_D = fabs (phi_D);
        }
        if (phi_D > LXx_PI) {
                phi_D = 2.0 * LXx_PI - phi_D;
        }

        LXx_VCLR(*outColor);

        LXx_VCPY(tempColor, matrData->primaryHighlightColor);
        LXx_VSCL(tempColor, this->N_R (phi_D));
        LXx_VSCL(tempColor, this->M_R (theta_H));
        LXx_VSCL(tempColor, matrData->primaryHighlightIntensity);
        LXx_VADD(*outColor, tempColor);
        
        LXx_VCPY(tempColor, matrData->secondaryHighlightColor);
        LXx_VSCL(tempColor, this->N_TRT (phi_D));
        LXx_VSCL(tempColor, this->M_TRT (theta_H));
        LXx_VSCL(tempColor, matrData->secondaryHighlightIntensity);
        LXx_VADD(*outColor, tempColor);

        LXx_VCPY(tempColor, matrData->rimLightColor);
        LXx_VSCL(tempColor, this->N_TT (phi_D));
        LXx_VSCL(tempColor, this->M_TT (theta_H));
        LXx_VSCL(tempColor, matrData->rimLightIntensity);
        LXx_VADD(*outColor, tempColor);

        LXx_VSCL(*outColor, 1.0/cosTheta2);
}

        void
HairMaterial::fPrime_s (
        double			thetaIn,
        double			thetaOut,
        double			phiDiff,
        double			gAngle,
        HairMaterialData	*matrData,
        LXtVector		*outColor)
{
        /*
         * Phi in and Phi out for the BSDF don't actually matter, just the difference,
         * since we assume hairs have circular cross-sections. So this function acts
         * as a wrapper for the version that takes in the input and output Phis.
         */
        this->fPrime_s(thetaIn, thetaOut, 0.0, phiDiff, gAngle, matrData, outColor);
}

        void
HairMaterial::fPrime_sNoGlints (
        double			thetaIn,
        double			thetaOut,
        double			phiDiff,
        HairMaterialData	*matrData,
        LXtVector		*outColor)
{
        /*
         * Phi in and Phi out for the BSDF don't actually matter, just the difference,
         * since we assume hairs have circular cross-sections. So this function acts
         * as a wrapper for the version that takes in the input and output Phis.
         */
        this->fPrime_sNoGlints(thetaIn, thetaOut, 0.0, phiDiff, matrData, outColor);
}

/*------------------------------- Luxology LLC ---------------------------
 *
 * Precomputation Functions
 *
 *----------------------------------------------------------------------------*/

        void
HairMaterial::PrintHairMaterialData (HairMaterialData *matrData)
{
        CLxUser_LogService log_S;

        log_S.DebugOut (LXi_DBLOG_NORMAL, "Hair Material Data: \n");
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Primary Hightlight Color: (%f, %f, %f) \n", 
                                                matrData->primaryHighlightColor[0],
                                                matrData->primaryHighlightColor[1],
                                                matrData->primaryHighlightColor[2]);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Primary Hightlight Intensity: %f \n", 
                                                matrData->primaryHighlightIntensity);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Primary Hightlight Long Width: %f \n", 
                                                matrData->primaryHighlightLongWidth);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Primary Hightlight Long Shift: %f \n", 
                                                matrData->primaryHighlightLongShift);

        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Secondary Hightlight Color: (%f, %f, %f) \n", 
                                                matrData->secondaryHighlightColor[0],
                                                matrData->secondaryHighlightColor[1],
                                                matrData->secondaryHighlightColor[2]);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Secondary Hightlight Intensity: %f \n", 
                                                matrData->secondaryHighlightIntensity);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Secondary Hightlight Long Width: %f \n", 
                                                matrData->secondaryHighlightLongWidth);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Secondary Hightlight Long Shift: %f \n", 
                                                matrData->secondaryHighlightLongShift);

        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Rimlight Color: (%f, %f, %f) \n", 
                                                matrData->rimLightColor[0],
                                                matrData->rimLightColor[1],
                                                matrData->rimLightColor[2]);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Rimlight Intensity: %f \n", 
                                                matrData->rimLightIntensity);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Rimlight Long Width: %f \n", 
                                                matrData->rimLightLongWidth);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Rimlight Shift: %f \n", 
                                                matrData->rimLightLongShift);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Rimlight Azimuthal Width: %f \n", 
                                                matrData->rimLightAzimuthalWidth);

        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Glints Color: (%f, %f, %f) \n", 
                                                matrData->glintsColor[0],
                                                matrData->glintsColor[1],
                                                matrData->glintsColor[2]);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Glints Intensity: %f \n", 
                                                matrData->glintsIntensity);
        log_S.DebugOut (LXi_DBLOG_NORMAL, "\t Glints Frequency: %f \n", 
                                                matrData->glintsFrequency);
}

/*
 * For deterministic integration, we use Lebedev quadrature with 146 values.
 */

#define LEBEDEV_COUNT 146

// lebAngles are the phi and theta angles for the lebedev set

static const double lebAngles[LEBEDEV_COUNT][2] = {	
        {-1.570796326794897, 0.000000000000000},
        { 1.570796326794897, 0.000000000000000},
        { 0.000000000000000, 1.570796326794897},
        { 0.000000000000000,-1.570796326794897},
        { 3.141592653589793, 0.000000000000000},
        { 0.000000000000000, 0.000000000000000},
        { 3.141592653589793, 0.785398163397448},
        { 3.141592653589793,-0.785398163397448},
        { 0.000000000000000, 0.785398163397448},
        { 0.000000000000000,-0.785398163397448},
        {-2.356194490192345, 0.000000000000000},
        { 2.356194490192345, 0.000000000000000},
        {-0.785398163397448, 0.000000000000000},
        { 0.785398163397448, 0.000000000000000},
        {-1.570796326794897, 0.785398163397448},
        { 1.570796326794897, 0.785398163397448},
        {-1.570796326794897,-0.785398163397448},
        { 1.570796326794897,-0.785398163397448},
        {-2.356194490192345, 0.615479708670387},
        { 2.356194490192345, 0.615479708670387},
        {-2.356194490192345,-0.615479708670387},
        { 2.356194490192345,-0.615479708670387},
        {-0.785398163397448, 0.615479708670387},
        { 0.785398163397448, 0.615479708670387},
        {-0.785398163397448,-0.615479708670387},
        { 0.785398163397448,-0.615479708670387},
        {-1.977429791373607, 0.742919562629720},
        { 1.977429791373607, 0.742919562629720},
        {-1.977429791373607,-0.742919562629720},
        { 1.977429791373607,-0.742919562629720},
        {-1.164162862216187, 0.742919562629720},
        { 1.164162862216187, 0.742919562629720},
        {-1.164162862216187,-0.742919562629720},
        { 1.164162862216187,-0.742919562629720},
        {-2.356194490192345, 0.295584323265131},
        { 2.356194490192345, 0.295584323265131},
        {-2.356194490192345,-0.295584323265131},
        { 2.356194490192345,-0.295584323265131},
        {-0.785398163397448, 0.295584323265131},
        { 0.785398163397448, 0.295584323265131},
        {-0.785398163397448,-0.295584323265131},
        { 0.785398163397448,-0.295584323265131},
        {-2.734959189011083, 0.742919562629720},
        { 2.734959189011083, 0.742919562629720},
        {-2.734959189011083,-0.742919562629720},
        { 2.734959189011083,-0.742919562629720},
        {-0.406633464578709, 0.742919562629720},
        { 0.406633464578710, 0.742919562629720},
        {-0.406633464578709,-0.742919562629720},
        { 0.406633464578710,-0.742919562629720},
        {-2.664212207036785, 0.430688055732060},
        { 2.664212207036785, 0.430688055732060},
        {-2.664212207036785,-0.430688055732060},
        { 2.664212207036785,-0.430688055732060},
        {-0.477380446553008, 0.430688055732060},
        { 0.477380446553008, 0.430688055732060},
        {-0.477380446553008,-0.430688055732060},
        { 0.477380446553008,-0.430688055732060},
        {-2.356194490192345, 0.939206447393399},
        { 2.356194490192345, 0.939206447393399},
        {-2.356194490192345,-0.939206447393399},
        { 2.356194490192345,-0.939206447393399},
        {-0.785398163397447, 0.939206447393399},
        { 0.785398163397448, 0.939206447393399},
        {-0.785398163397447,-0.939206447393399},
        { 0.785398163397448,-0.939206447393399},
        {-2.048176773347905, 0.430688055732060},
        { 2.048176773347905, 0.430688055732060},
        {-2.048176773347905,-0.430688055732060},
        { 2.048176773347905,-0.430688055732060},
        {-1.093415880241889, 0.430688055732060},
        { 1.093415880241888, 0.430688055732060},
        {-1.093415880241889,-0.430688055732060},
        { 1.093415880241888,-0.430688055732060},
        {-2.981452032263868, 0.158125800702240},
        { 2.981452032263868, 0.158125800702240},
        {-2.981452032263868,-0.158125800702240},
        { 2.981452032263868,-0.158125800702240},
        {-0.160140621325925, 0.158125800702240},
        { 0.160140621325925, 0.158125800702240},
        {-0.160140621325925,-0.158125800702240},
        { 0.160140621325925,-0.158125800702240},
        {-2.356194490192344, 1.346220448647921},
        { 2.356194490192344, 1.346220448647921},
        {-2.356194490192344,-1.346220448647921},
        { 2.356194490192344,-1.346220448647921},
        {-0.785398163397449, 1.346220448647921},
        { 0.785398163397449, 1.346220448647921},
        {-0.785398163397449,-1.346220448647921},
        { 0.785398163397449,-1.346220448647921},
        {-1.730936948120822, 0.158125800702240},
        { 1.730936948120822, 0.158125800702240},
        {-1.730936948120822,-0.158125800702240},
        { 1.730936948120822,-0.158125800702240},
        {-1.410655705468971, 0.158125800702240},
        { 1.410655705468971, 0.158125800702240},
        {-1.410655705468971,-0.158125800702240},
        { 1.410655705468971,-0.158125800702240},
        {-2.983830280119793, 0.466018395180603},
        { 2.983830280119793, 0.466018395180603},
        {-2.983830280119793,-0.466018395180603},
        { 2.983830280119793,-0.466018395180603},
        {-0.157762373470000, 0.466018395180603},
        { 0.157762373470000, 0.466018395180603},
        {-0.157762373470000,-0.466018395180603},
        { 0.157762373470000,-0.466018395180603},
        {-2.838831730243162, 1.080662803573478},
        { 2.838831730243162, 1.080662803573478},
        {-2.838831730243162,-1.080662803573478},
        { 2.838831730243162,-1.080662803573478},
        {-0.302760923346631, 1.080662803573478},
        { 0.302760923346631, 1.080662803573478},
        {-0.302760923346631,-1.080662803573478},
        { 0.302760923346631,-1.080662803573478},
        {-2.670539310507518, 0.140820339778975},
        { 2.670539310507518, 0.140820339778975},
        {-2.670539310507518,-0.140820339778975},
        { 2.670539310507518,-0.140820339778975},
        {-0.471053343082275, 0.140820339778975},
        { 0.471053343082275, 0.140820339778975},
        {-0.471053343082275,-0.140820339778975},
        { 0.471053343082275,-0.140820339778975},
        {-1.873557250141527, 1.080662803573478},
        { 1.873557250141527, 1.080662803573478},
        {-1.873557250141527,-1.080662803573478},
        { 1.873557250141527,-1.080662803573478},
        {-1.268035403448265, 1.080662803573478},
        { 1.268035403448265, 1.080662803573478},
        {-1.268035403448265,-1.080662803573478},
        { 1.268035403448265,-1.080662803573478},
        {-2.041849669877172, 0.140820339778975},
        { 2.041849669877172, 0.140820339778975},
        {-2.041849669877172,-0.140820339778975},
        { 2.041849669877172,-0.140820339778975},
        {-1.099742983712622, 0.140820339778975},
        { 1.099742983712622, 0.140820339778975},
        {-1.099742983712622,-0.140820339778975},
        { 1.099742983712622,-0.140820339778975},
        {-1.728558700264896, 0.466018395180603},
        { 1.728558700264897, 0.466018395180603},
        {-1.728558700264896,-0.466018395180603},
        { 1.728558700264897,-0.466018395180603},
        {-1.413033953324897, 0.466018395180603},
        { 1.413033953324896, 0.466018395180603},
        {-1.413033953324897,-0.466018395180603},
        { 1.413033953324896,-0.466018395180603}};

// lebWeights are the weights used for the lebedev set

static const double lebWeights[LEBEDEV_COUNT] = {
        0.0005996313688621381, 0.0005996313688621381, 0.0005996313688621381, 
        0.0005996313688621381, 0.0005996313688621381, 0.0005996313688621381, 
        0.007372999718620756, 0.007372999718620756, 0.007372999718620756, 
        0.007372999718620756, 0.007372999718620756, 0.007372999718620756, 
        0.007372999718620756, 0.007372999718620756, 0.007372999718620756, 
        0.007372999718620756, 0.007372999718620756, 0.007372999718620756, 
        0.007210515360144488, 0.007210515360144488, 0.007210515360144488, 
        0.007210515360144488, 0.007210515360144488, 0.007210515360144488, 
        0.007210515360144488, 0.007210515360144488, 0.007116355493117555, 
        0.007116355493117555, 0.007116355493117555, 0.007116355493117555, 
        0.007116355493117555, 0.007116355493117555, 0.007116355493117555, 
        0.007116355493117555, 0.007116355493117555, 0.007116355493117555, 
        0.007116355493117555, 0.007116355493117555, 0.007116355493117555, 
        0.007116355493117555, 0.007116355493117555, 0.007116355493117555, 
        0.007116355493117555, 0.007116355493117555, 0.007116355493117555, 
        0.007116355493117555, 0.007116355493117555, 0.007116355493117555, 
        0.007116355493117555, 0.007116355493117555, 0.006753829486314477, 
        0.006753829486314477, 0.006753829486314477, 0.006753829486314477, 
        0.006753829486314477, 0.006753829486314477, 0.006753829486314477, 
        0.006753829486314477, 0.006753829486314477, 0.006753829486314477, 
        0.006753829486314477, 0.006753829486314477, 0.006753829486314477, 
        0.006753829486314477, 0.006753829486314477, 0.006753829486314477, 
        0.006753829486314477, 0.006753829486314477, 0.006753829486314477, 
        0.006753829486314477, 0.006753829486314477, 0.006753829486314477, 
        0.006753829486314477, 0.006753829486314477, 0.007574394159054034, 
        0.007574394159054034, 0.007574394159054034, 0.007574394159054034, 
        0.007574394159054034, 0.007574394159054034, 0.007574394159054034, 
        0.007574394159054034, 0.007574394159054034, 0.007574394159054034, 
        0.007574394159054034, 0.007574394159054034, 0.007574394159054034, 
        0.007574394159054034, 0.007574394159054034, 0.007574394159054034, 
        0.007574394159054034, 0.007574394159054034, 0.007574394159054034, 
        0.007574394159054034, 0.007574394159054034, 0.007574394159054034, 
        0.007574394159054034, 0.007574394159054034, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262, 0.006991087353303262, 
        0.006991087353303262, 0.006991087353303262};

        void
HairMaterial::PrecomputeNormFactors(
        HairMaterialData		*matrData)
{
        // In order to make sure the artist-friendly model is energy conserving for backward
        // scattering, we need to compute the normalization factors for every possible incoming
        // Long inclination. See "An Artist Friendly Hair Shading System" equation 13
        double				 thetaOut, phiDiff, thetaIn;
        int				 j;
        LXtVector			 tempColor;

        // We need to compute the normalization factor for each possible in-direction
        for (int i = 0; i < TABLE_ENTRIES; i++) {
                thetaIn = ((double)i + .5)/TABLE_ENTRIES * LXx_PI - LXx_HALFPI;

                LXx_VCLR (this->normFactors[i]);

                for (j = 0; j < LEBEDEV_COUNT; j++) {
                        phiDiff = lebAngles[j][0];
                        thetaOut = lebAngles[j][1];

                        this->fPrime_sNoGlints (thetaIn, thetaOut, phiDiff, matrData, &tempColor);

                        LXx_VSCL (tempColor, lebWeights[j]);
                        LXx_VADD (this->normFactors[i], tempColor);
                }
                
                LXx_VSCL (this->normFactors[i], 4.0 * LXx_PI);
        }

}

        void
HairMaterial::PrecomputeaBar_f ()
{
        // See "Dual Scattering Approximation for Fast Multiple Scattering in Hair" equation 6
        double theta_Df, thetaOutf, phiInf, phiOutf, phiDiff, v, sinThetaOut;
        double cosThetaD;
        LXtVector tempColor, tempColor2, tempColor3;

        int sampleCount = INTEGRAL_SAMPLES/2;

        double invSamples = 1.0/sampleCount;

        for (int i = 0; i < TABLE_ENTRIES; i++) {
                theta_Df = LXx_PI * ((.5 + i)/TABLE_ENTRIES) - LXx_HALFPI;
                cosThetaD = cos(theta_Df);

                LXx_VCLR (tempColor);

                for (int phiIn = 0; phiIn < sampleCount; phiIn++) {
                        phiInf = ((.5 + phiIn)/ sampleCount) * LXx_PI - LXx_HALFPI;

                        for (int phiOut = 0; phiOut < sampleCount; phiOut++) {
                                phiOutf = ((.5 + phiOut)/ sampleCount) * LXx_PI + LXx_HALFPI;

                                LXx_VCLR (tempColor2);

                                for (int thetaSample = 0; thetaSample < sampleCount; thetaSample++) {
                                        v = ((double)thetaSample + .5)/sampleCount;
                                        sinThetaOut = sqrtf (fabs (2.0*v - 1.0)) * (v >= .5 ? 1 : -1);

                                        thetaOutf = asin(sinThetaOut);

                                        phiDiff = phiOutf - phiInf;

                                        this->fPrime_sNoGlints(theta_Df, thetaOutf, phiDiff, 
                                                &(this->matrData), &tempColor3);
                                        
                                        LXx_VSCL (tempColor3, (2.0 * LXx_PI / this->lookupNormFactor(theta_Df)));

                                        LXx_VADD (tempColor2, tempColor3);
                                }

                                LXx_VSCL (tempColor2, invSamples);
                                LXx_VADD (tempColor, tempColor2);
                        }
                }
                LXx_VSCL (tempColor, invSamples * invSamples * cosThetaD);

                LXx_VCPY (this->aBar_f[i], tempColor);
        }
}

        void
HairMaterial::PrecomputeaBar_b ()
{
        // See "Dual Scattering Approximation for Fast Multiple Scattering in Hair" equation 12
        double theta_Df, thetaOutf, phiInf, phiOutf, phiDiff, v, sinThetaOut;
        double cosThetaD;
        LXtVector tempColor, tempColor2, tempColor3;

        int sampleCount = INTEGRAL_SAMPLES/2;

        double invSamples = 1.0/sampleCount;

        for (int i = 0; i < TABLE_ENTRIES; i++) {
                theta_Df = LXx_PI * ((.5 + i)/TABLE_ENTRIES) - LXx_HALFPI;
                cosThetaD = cos(theta_Df);

                LXx_VCLR (tempColor);

                for (int phiIn = 0; phiIn < sampleCount; phiIn++) {
                        phiInf = ((.5 + phiIn)/ sampleCount) * LXx_PI - LXx_HALFPI;

                        for (int phiOut = 0; phiOut < sampleCount; phiOut++) {
                                phiOutf = ((.5 + phiOut)/ sampleCount) * LXx_PI - LXx_HALFPI;

                                LXx_VCLR (tempColor2);

                                for (int thetaSample = 0; thetaSample < sampleCount; thetaSample++) {
                                        v = ((double)thetaSample + .5)/sampleCount;
                                        sinThetaOut = sqrtf (fabs (2.0*v - 1.0)) * (v >= .5 ? 1 : -1);

                                        thetaOutf = asin(sinThetaOut);

                                        phiDiff = phiOutf - phiInf;

                                        this->fPrime_sNoGlints (theta_Df, thetaOutf, phiDiff, 
                                                &(this->matrData), &tempColor3);
                                        
                                        LXx_VSCL (tempColor3, (2.0 * LXx_PI / this->lookupNormFactor(theta_Df)));

                                        LXx_VADD (tempColor2, tempColor3);
                                }

                                LXx_VSCL (tempColor2, invSamples);
                                LXx_VADD (tempColor, tempColor2);
                        }
                }
                LXx_VSCL (tempColor, invSamples * invSamples * cosThetaD);

                LXx_VCPY (this->aBar_b[i], tempColor);
        }
}

        void
HairMaterial::PrecomputeAlphaBar_f ()
{
        /*
         * See "Efficient Implementation of the Dual Scattering Model in Renderman" 
         * equation 13.
         */
        int sampleCount = INTEGRAL_SAMPLES/2;
        double phiInf, phiOutf, phi_d, thetaf, theta_h, thetaDf;
        LXtVector tempColor, tempColorNum, tempColorDenom;

        double sclFactor;

        LXtVector white = {1.0, 1.0, 1.0};

        double invSamples = 1.0/sampleCount;
        double invSamplesCube = invSamples * invSamples * invSamples;

        LXtVector numerColor, denomColor;

        for (int i = 0; i < TABLE_ENTRIES; i++) 
        {
                thetaDf = ((.5 + i)/TABLE_ENTRIES) * LXx_PI - LXx_HALFPI;

                sclFactor = LXx_PI / 2.0 * invSamplesCube;

                LXx_VCLR (numerColor);
                LXx_VCLR (denomColor);

                // Integrate over the forward scattering hemisphere. For the
                // denominator we are basically just getting the total weight,
                // and for the numerator we are multiply each component by its
                // shift
                for (int theta = 0; theta < sampleCount; theta++) 
                {
                        thetaf = ((.5 + theta)/sampleCount) * LXx_PI - LXx_HALFPI;	
                        theta_h = (thetaf + thetaDf)/2.0;

                                for (int phiIn = 0; phiIn < sampleCount; phiIn++) {
                                        phiInf = ((.5 + phiIn)/ sampleCount) * LXx_PI - LXx_HALFPI;

                                        for (int phiOut = 0; phiOut < sampleCount; phiOut++) {

                                                phiOutf = LXx_HALFPI + ((.5 + phiIn)/ sampleCount) * LXx_PI;

                                                // Compute the phi difference
                                                phi_d = phiOutf - phiInf;
                                                if (phi_d > LXx_PI) {
                                                        phi_d = 2.0 * LXx_PI - phi_d;
                                                }
                                                else if (phi_d < 0) {
                                                        phi_d = fabs (phi_d);
                                                        if (phi_d > LXx_PI) {
                                                                phi_d = 2.0 * LXx_PI - phi_d;
                                                        }
                                                }

                                                LXx_VCLR (tempColorNum);
                                                LXx_VCLR (tempColorDenom);

                                                LXx_VCPY(tempColor, white);
                                                LXx_VSCL(tempColor, this->N_R (phi_d));
                                                LXx_VSCL(tempColor, this->M_R (theta_h));
                                                LXx_VSCL(tempColor, this->matrData.primaryHighlightIntensity);
                                                LXx_VADD(tempColorDenom, tempColor);
                                                LXx_VSCL(tempColor, this->matrData.primaryHighlightLongShift);
                                                LXx_VADD(tempColorNum, tempColor);
        
                                                LXx_VCPY(tempColor, white);
                                                LXx_VSCL(tempColor, this->N_TRT (phi_d));
                                                LXx_VSCL(tempColor, this->M_TRT (theta_h));
                                                LXx_VSCL(tempColor, this->matrData.secondaryHighlightIntensity);
                                                LXx_VADD(tempColorDenom, tempColor);
                                                LXx_VSCL(tempColor, this->matrData.secondaryHighlightLongShift);
                                                LXx_VADD(tempColorNum, tempColor);

                                                LXx_VCPY(tempColor, white);
                                                LXx_VSCL(tempColor, this->N_TT (phi_d));
                                                LXx_VSCL(tempColor, this->M_TT (theta_h));
                                                LXx_VSCL(tempColor, this->matrData.rimLightIntensity);
                                                LXx_VADD(tempColorDenom, tempColor);
                                                LXx_VSCL(tempColor, this->matrData.rimLightLongShift);
                                                LXx_VADD(tempColorNum, tempColor);

                                                LXx_VSCL(tempColorNum, sclFactor * sin(thetaf + LXx_HALFPI));
                                                LXx_VADD(numerColor, tempColorNum);

                                                LXx_VSCL(tempColorDenom, sclFactor * sin(thetaf + LXx_HALFPI));
                                                LXx_VADD(denomColor, tempColorDenom);
                                        }
                                }
                }

                // Note: the denominator will also act as a normalization
                for (int j = 0; j < 3; j++) {
                        numerColor[j] = denomColor[j] > 0.0 
                                        ? (numerColor[j] / denomColor[j])
                                        : 0.0;
                }
                LXx_VCPY(this->AlphaBar_f[i], numerColor);
        }
}


        void
HairMaterial::PrecomputeAlphaBar_b ()
{
        /*
         * See "Efficient Implementation of the Dual Scattering Model in Renderman" 
         * equation 14.
         */
        int sampleCount = INTEGRAL_SAMPLES/2;
        double phiInf, phiOutf, phi_d, thetaf, theta_h, thetaDf;
        LXtVector tempColor, tempColorNum, tempColorDenom;

        double sclFactor;

        double invSamples = 1.0/sampleCount;
        double invSamplesCube = invSamples * invSamples * invSamples;

        LXtVector white = {1.0, 1.0, 1.0};

        LXtVector numerColor, denomColor;

        for (int i = 0; i < TABLE_ENTRIES; i++) 
        {
                thetaDf = ((.5 + i)/TABLE_ENTRIES) * LXx_PI - LXx_HALFPI;

                sclFactor = LXx_PI / 2.0 * invSamplesCube;

                LXx_VCLR (numerColor);
                LXx_VCLR (denomColor);

                // Integrate over the forward scattering hemisphere. For the
                // denominator we are basically just getting the total weight,
                // and for the numerator we are multiply each component by its
                // shift
                for (int theta = 0; theta < sampleCount; theta++) 
                {
                        thetaf = ((.5 + theta)/sampleCount) * LXx_PI - LXx_HALFPI;	
                        theta_h = (thetaf + thetaDf)/2.0;

                                for (int phiIn = 0; phiIn < sampleCount; phiIn++) {
                                        phiInf = ((.5 + phiIn)/ sampleCount) * LXx_PI - LXx_HALFPI;

                                        for (int phiOut = 0; phiOut < sampleCount; phiOut++) {

                                                phiOutf = ((.5 + phiIn)/ sampleCount) * LXx_PI - LXx_HALFPI;

                                                // Compute the phi difference
                                                phi_d = phiOutf - phiInf;
                                                if (phi_d > LXx_PI) {
                                                        phi_d = 2.0 * LXx_PI - phi_d;
                                                }
                                                else if (phi_d < 0) {
                                                        phi_d = fabs (phi_d);
                                                        if (phi_d > LXx_PI) {
                                                                phi_d = 2.0 * LXx_PI - phi_d;
                                                        }
                                                }

                                                LXx_VCLR (tempColorNum);
                                                LXx_VCLR (tempColorDenom);

                                                LXx_VCPY(tempColor, white);
                                                LXx_VSCL(tempColor, this->N_R (phi_d));
                                                LXx_VSCL(tempColor, this->M_R (theta_h));
                                                LXx_VSCL(tempColor, this->matrData.primaryHighlightIntensity);
                                                LXx_VADD(tempColorDenom, tempColor);
                                                LXx_VSCL(tempColor, this->matrData.primaryHighlightLongShift);
                                                LXx_VADD(tempColorNum, tempColor);
        
                                                LXx_VCPY(tempColor, white);
                                                LXx_VSCL(tempColor, this->N_TRT (phi_d));
                                                LXx_VSCL(tempColor, this->M_TRT (theta_h));
                                                LXx_VSCL(tempColor, this->matrData.secondaryHighlightIntensity);
                                                LXx_VADD(tempColorDenom, tempColor);
                                                LXx_VSCL(tempColor, this->matrData.secondaryHighlightLongShift);
                                                LXx_VADD(tempColorNum, tempColor);

                                                LXx_VCPY(tempColor, white);
                                                LXx_VSCL(tempColor, this->N_TT (phi_d));
                                                LXx_VSCL(tempColor, this->M_TT (theta_h));
                                                LXx_VSCL(tempColor, this->matrData.rimLightIntensity);
                                                LXx_VADD(tempColorDenom, tempColor);
                                                LXx_VSCL(tempColor, this->matrData.rimLightLongShift);
                                                LXx_VADD(tempColorNum, tempColor);

                                                LXx_VSCL(tempColorNum, sclFactor * sin(thetaf + LXx_HALFPI));
                                                LXx_VADD(numerColor, tempColorNum);

                                                LXx_VSCL(tempColorDenom, sclFactor * sin(thetaf + LXx_HALFPI));
                                                LXx_VADD(denomColor, tempColorDenom);
                                        }
                                }
                }

                // Note: the denominator will also act as a normalization
                for (int j = 0; j < 3; j++) {
                        numerColor[j] = denomColor[j] > 0.0 
                                        ? (numerColor[j] / denomColor[j])
                                        : 0.0;
                }
                LXx_VCPY(this->AlphaBar_b[i], numerColor);
        }
}

        void
HairMaterial::PrecomputeBetaBar_f ()
{
        /*
         * See "Efficient Implementation of the Dual Scattering Model in Renderman" 
         * equation 15.
         *
         * Right now we're only taking Long variance into account. This may
         * need to be fixed in the future, particularly with respect to the azimuthal
         * variance of the rim lighting.
         */
        int sampleCount = INTEGRAL_SAMPLES/2;
        double phi_d, thetaf, theta_h, thetaDf, v, sinThetaOut;
        LXtVector tempColor, tempColorNum, tempColorDenom;

        double sclFactor;

        double invSamples = 1.0/sampleCount;
        double invSamplesCube = invSamples * invSamples * invSamples;

        LXtVector numerColor, denomColor;

        LXtVector white = {1.0, 1.0, 1.0};

        for (int i = 0; i < TABLE_ENTRIES; i++) 
        {
                thetaDf = ((.5 + i)/TABLE_ENTRIES) * LXx_PI - LXx_HALFPI;

                sclFactor = LXx_PI / 2.0 * invSamplesCube;

                LXx_VCLR (numerColor);
                LXx_VCLR (denomColor);

                // Integrate over the forward scattering hemisphere. For the
                // denominator we are basically just getting the total weight,
                // and for the numerator we are multiply each component by its
                // variance
                for (int theta = 0; theta < sampleCount; theta++) 
                {
                        v = ((double)theta + .5)/sampleCount;
                        sinThetaOut = sqrt (fabs (2.0*v - 1.0)) * (v >= .5 ? 1 : -1);

                        thetaf = asin(sinThetaOut);	

                        theta_h = (thetaf - thetaDf)/2.0;

                                for (int phi = 0; phi < sampleCount; phi++) {
                                        phi_d = ((.5 + phi)/ sampleCount) * LXx_PI + LXx_HALFPI;

                                        if (phi_d > LXx_PI) {
                                                phi_d = 2.0 * LXx_PI - phi_d;
                                        }
                                        else if (phi_d < 0) {
                                                phi_d = fabs (phi_d);
                                                if (phi_d > LXx_PI) {
                                                        phi_d = 2.0 * LXx_PI - phi_d;
                                                }
                                        }

                                        // Compute the BCSDF for this set of angles
                                        LXx_VCLR (tempColorNum);
                                        LXx_VCLR (tempColorDenom);

                                        LXx_VCPY(tempColor, white);
                                        LXx_VSCL(tempColor, this->N_R (phi_d));
                                        LXx_VSCL(tempColor, this->M_R (theta_h));
                                        LXx_VSCL(tempColor, this->matrData.primaryHighlightIntensity);
                                        LXx_VADD(tempColorDenom, tempColor);
                                        LXx_VSCL(tempColor, this->matrData.primaryHighlightLongWidth);
                                        LXx_VADD(tempColorNum, tempColor);
        
                                        LXx_VCPY(tempColor, white);
                                        LXx_VSCL(tempColor, this->N_TRT (phi_d));
                                        LXx_VSCL(tempColor, this->M_TRT (theta_h));
                                        LXx_VSCL(tempColor, this->matrData.secondaryHighlightIntensity);
                                        LXx_VADD(tempColorDenom, tempColor);
                                        LXx_VSCL(tempColor, this->matrData.secondaryHighlightLongWidth);
                                        LXx_VADD(tempColorNum, tempColor);

                                        LXx_VCPY(tempColor, white);
                                        LXx_VSCL(tempColor, this->N_TT (phi_d));
                                        LXx_VSCL(tempColor, this->M_TT (theta_h));
                                        LXx_VSCL(tempColor, this->matrData.rimLightIntensity);
                                        LXx_VADD(tempColorDenom, tempColor);
                                        LXx_VSCL(tempColor, this->matrData.rimLightLongWidth);
                                        LXx_VADD(tempColorNum, tempColor);

                                        LXx_VSCL(tempColorNum, sclFactor * sin(thetaf + LXx_HALFPI));
                                        LXx_VADD(numerColor, tempColorNum);

                                        LXx_VSCL(tempColorDenom, sclFactor * sin(thetaf + LXx_HALFPI));
                                        LXx_VADD(denomColor, tempColorDenom);
                                }
                }

                // Note: the denominator will also act as a normalization
                for (int j = 0; j < 3; j++) {
                        numerColor[j] = denomColor[j] > 0.0 
                                        ? (numerColor[j] / denomColor[j])
                                        : 0.0;
                }
                LXx_VCPY(this->BetaBar_f[i], numerColor);
        }
}


        void
HairMaterial::PrecomputeBetaBar_b ()
{
        /*
         * See "Efficient Implementation of the Dual Scattering Model in Renderman" 
         * equation 16.
         *
         * Right now we're only taking Longituidnal variance into account. This may
         * need to be fixed in the future, particularly with respect to the azimuthal
         * variance of the rim lighting.
         */
        int sampleCount = INTEGRAL_SAMPLES/2;
        double phi_d, thetaf, theta_h, thetaDf, v, sinThetaOut;
        LXtVector tempColor, tempColorNum, tempColorDenom;

        double sclFactor;

        double invSamples = 1.0/sampleCount;
        double invSamplesCube = invSamples * invSamples * invSamples;

        LXtVector numerColor, denomColor;

        LXtVector white = {1.0, 1.0, 1.0};

        for (int i = 0; i < TABLE_ENTRIES; i++) 
        {
                thetaDf = ((.5 + i)/TABLE_ENTRIES) * LXx_PI - LXx_HALFPI;

                sclFactor = LXx_PI / 2.0 * invSamplesCube;

                LXx_VCLR (numerColor);
                LXx_VCLR (denomColor);

                // Integrate over the forward scattering hemisphere. For the
                // denominator we are basically just getting the total weight,
                // and for the numerator we are multiply each component by its
                // variance
                for (int theta = 0; theta < sampleCount; theta++) 
                {
                        v = ((double)theta + .5)/sampleCount;
                        sinThetaOut = sqrt (fabs (2.0*v - 1.0)) * (v >= .5 ? 1 : -1);

                        thetaf = asin(sinThetaOut);	

                        theta_h = (thetaf - thetaDf)/2.0;

                                for (int phi = 0; phi < sampleCount; phi++) {
                                        phi_d = ((.5 + phi)/ sampleCount) * LXx_PI - LXx_HALFPI;

                                        if (phi_d > LXx_PI) {
                                                phi_d = 2.0 * LXx_PI - phi_d;
                                        }
                                        else if (phi_d < 0) {
                                                phi_d = fabs (phi_d);
                                                if (phi_d > LXx_PI) {
                                                        phi_d = 2.0 * LXx_PI - phi_d;
                                                }
                                        }

                                        // Compute the BCSDF for this set of angles
                                        LXx_VCLR (tempColorNum);
                                        LXx_VCLR (tempColorDenom);

                                        LXx_VCPY(tempColor, white);
                                        LXx_VSCL(tempColor, this->N_R (phi_d));
                                        LXx_VSCL(tempColor, this->M_R (theta_h));
                                        LXx_VSCL(tempColor, this->matrData.primaryHighlightIntensity);
                                        LXx_VADD(tempColorDenom, tempColor);
                                        LXx_VSCL(tempColor, this->matrData.primaryHighlightLongWidth);
                                        LXx_VADD(tempColorNum, tempColor);
        
                                        LXx_VCPY(tempColor, white);
                                        LXx_VSCL(tempColor, this->N_TRT (phi_d));
                                        LXx_VSCL(tempColor, this->M_TRT (theta_h));
                                        LXx_VSCL(tempColor, this->matrData.secondaryHighlightIntensity);
                                        LXx_VADD(tempColorDenom, tempColor);
                                        LXx_VSCL(tempColor, this->matrData.secondaryHighlightLongWidth);
                                        LXx_VADD(tempColorNum, tempColor);

                                        LXx_VCPY(tempColor, white);
                                        LXx_VSCL(tempColor, this->N_TT (phi_d));
                                        LXx_VSCL(tempColor, this->M_TT (theta_h));
                                        LXx_VSCL(tempColor, this->matrData.rimLightIntensity);
                                        LXx_VADD(tempColorDenom, tempColor);
                                        LXx_VSCL(tempColor, this->matrData.rimLightLongWidth);
                                        LXx_VADD(tempColorNum, tempColor);

                                        LXx_VSCL(tempColorNum, sclFactor * sin(thetaf + LXx_HALFPI));
                                        LXx_VADD(numerColor, tempColorNum);

                                        LXx_VSCL(tempColorDenom, sclFactor * sin(thetaf + LXx_HALFPI));
                                        LXx_VADD(denomColor, tempColorDenom);
                                }
                }

                // Note: the denominator will also act as a normalization
                for (int j = 0; j < 3; j++) {
                        numerColor[j] = denomColor[j] > 0.0 
                                        ? (numerColor[j] / denomColor[j])
                                        : 0.0;
                }
                LXx_VCPY(this->BetaBar_b[i], numerColor);
        }
}


        void
HairMaterial::PrecomputeDeltaBar_b () 
{
        /*
         * See "Efficient Implementation of the Dual Scattering Model in Renderman" 
         * equation 11.
         */
        double thetaDf;
        LXtVector alphaBar_b, alphaBar_f, aBar_b, aBar_f, aBar_fSqr, aBar_bSqr;

        LXtVector one;
        LXtVector num1, num2, num2add1, num2add2, den1, den2;
        LXtVector den2sqrTemp;
        LXtVector add1, add2;

        LXx_VSET (one, 1.0);

        for (int i = 0; i < TABLE_ENTRIES; i++) 
        {
                thetaDf = ((.5 + i)/TABLE_ENTRIES) * LXx_PI - LXx_HALFPI;

                this->lookupAlphaBar_f (thetaDf, &alphaBar_f);
                this->lookupAlphaBar_b (thetaDf, &alphaBar_b);
                this->lookupaBar_f (thetaDf, &aBar_f);
                this->lookupaBar_b (thetaDf, &aBar_b);
                LXx_VMUL3 (aBar_fSqr, aBar_f, aBar_f);
                LXx_VMUL3 (aBar_bSqr, aBar_b, aBar_b);

                /*
                 * Compute the left portion of the addition in equation 11
                 */

                // Compute the interior numerator
                LXx_VSCL3 (num1, aBar_bSqr, 2.0);
                // Compute the interior denominator
                LXx_VSUB3 (den1, one, aBar_fSqr);
                LXx_VMUL (den1, den1); // this squares it
                // Compute the whole portion in parentheses
                LXx_VDIV (num1, den1);
                LXx_VSUB3 (add1, one, num1);
                // Multiply by alphaBar_b
                LXx_VMUL (add1, alphaBar_b);

                /*
                 * Compute the right portion
                 */

                // Compute the left portion of the addition in the numerator
                LXx_VSUB3 (num2add1, one, aBar_fSqr);
                LXx_VMUL (num2add1, num2add1);
                LXx_VSCL (num2add1, 2.0);
                // Compute the right portion of the subtraction in the numerator
                LXx_VMUL3 (num2add2, aBar_fSqr, aBar_bSqr);
                LXx_VSCL (num2add2, 4.0);
                // Add them for the numerator
                LXx_VADD3 (num2, num2add1, num2add2);
                // Compute the denominator
                LXx_VSUB3 (den2, one, aBar_fSqr);
                LXx_VMUL3 (den2sqrTemp, den2, den2); // This line and the next
                LXx_VMUL (den2, den2sqrTemp);	     // one compute the cube
                // Compute the whole portion in parentheses
                LXx_VDIV (num2, den2);
                // Multiply by alphaBar_f
                LXx_VMUL3 (add2, num2, alphaBar_f);

                // Finally add them and store the value in the table
                LXx_VADD3 (this->DeltaBar_b[i], add1, add2);

        }
}

        void
HairMaterial::PrecomputeSigmaBar_b () 
{
        /*
         * See "Efficient Implementation of the Dual Scattering Model in Renderman" 
         * equation 12.
         */
        double		thetaDf;
        LXtVector	betaBar_b, betaBar_f,
                        betaBar_bSqr, betaBar_fSqr,
                        aBar_b, aBar_f, 
                        aBar_fSqr, aBar_bSqr;

        LXtVector	one;
        LXtVector	prod1, prod2,
                        num, den, numadd1, numadd2,
                        denparen1, denparen2;

        LXx_VSET (one, 1.0);

        for (int i = 0; i < TABLE_ENTRIES; i++) 
        {
                thetaDf = ((.5 + i)/TABLE_ENTRIES) * LXx_PI - LXx_HALFPI;

                this->lookupBetaBar_f (thetaDf, &betaBar_f);
                this->lookupBetaBar_b (thetaDf, &betaBar_b);
                this->lookupaBar_f (thetaDf, &aBar_f);
                this->lookupaBar_b (thetaDf, &aBar_b);
                LXx_VMUL3 (aBar_fSqr, aBar_f, aBar_f);
                LXx_VMUL3 (aBar_bSqr, aBar_b, aBar_b);
                LXx_VMUL3 (betaBar_fSqr, betaBar_f, betaBar_f);
                LXx_VMUL3 (betaBar_bSqr, betaBar_b, betaBar_b);

                // Compute sigmaBar_b as in equation 12

                        // Compute the left side of the outermost product
                        LXx_VSCL3 (prod1, aBar_fSqr, this->matrData.densityFactor);
                        LXx_VADD (prod1, one);

                        // Compute the right side of the outermost product

                                // Compute the numerator

                                        // Compute the left side of the addition
                                        LXx_VSCL3 (numadd1, betaBar_fSqr, 2.0);
                                        LXx_VADD (numadd1, betaBar_bSqr);
                                        LXx_VUOP (numadd1, sqrtf);
                                        LXx_VMUL (numadd1, aBar_b);

                                        // Compute the right side 
                                        LXx_VMUL3 (numadd2, aBar_b, aBar_b);
                                        LXx_VMUL (numadd2, numadd1);

                                        // Add them together
                                        LXx_VADD3 (num, numadd1, numadd2);

                                // Compute the denominator

                                        // Compute the left side of the addition
                                        // within the parentheses
                                        LXx_VSCL3 (denparen1, betaBar_f, 2.0);

                                        // Compute the right side of the addition
                                        // within the parentheses
                                        LXx_VSCL3 (denparen2, betaBar_b, 3.0);

                                        // Add them together
                                        LXx_VADD3 (den, denparen1, denparen2);

                                        // Multiply by aBar_b cubed
                                        LXx_VMUL (den, aBar_bSqr);
                                        LXx_VMUL (den, aBar_b);

                                        // Add aBar_b
                                        LXx_VADD (den, aBar_b);

                                // Finish computing the outmost right side by dividing
                                for (int j = 0; j < 3; j++) {
                                        num[j] = den[j] > 0.0 
                                                        ? (num[j] / den[j])
                                                        : 0.0;
                                }
                                LXx_VCPY(prod2, num);

                        // Finally, multiply the two terms together and store the result
                        LXx_VMUL3 (this->SigmaBar_b[i], prod1, prod2);
        }
}

        void
HairMaterial::PrecomputeABar_b () 
{
        /*
         * See "Efficient Implementation of the Dual Scattering Model in Renderman" 
         * equation 10.
         */
        double		thetaDf;
        LXtVector	aBar_b, aBar_f, 
                        aBar_fSqr, aBar_bSqr;

        LXtVector	one;
        LXtVector	add1, add2, add1num, add1den,
                        add2num, add2den;

        LXx_VSET (one, 1.0);

        for (int i = 0; i < TABLE_ENTRIES; i++) 
        {
                thetaDf = ((.5 + i)/TABLE_ENTRIES) * LXx_PI - LXx_HALFPI;

                this->lookupaBar_f (thetaDf, &aBar_f);
                this->lookupaBar_b (thetaDf, &aBar_b);
                LXx_VMUL3 (aBar_fSqr, aBar_f, aBar_f);
                LXx_VMUL3 (aBar_bSqr, aBar_b, aBar_b);

                // Compute ABar_b as in equation 10

                        // Compute the left side of the outermost addition

                                // Compute the numerator
                                LXx_VMUL3 (add1num, aBar_b, aBar_fSqr);
                                // Compute the denominator
                                LXx_VSUB3 (add1den, one, aBar_fSqr);

                                // Divide to get the left side
                                LXx_VDIV (add1num, add1den);
                                LXx_VCPY (add1, add1num);

                        // Compute the right side of the outermost addition

                                // Compute the numerator
                                LXx_VMUL3 (add2num, aBar_bSqr, aBar_fSqr);
                                LXx_VMUL (add2num, aBar_b);
                                // Compute the denominator
                                LXx_VSUB3 (add2den, one, aBar_fSqr);
                                LXx_VMUL (add2den, add2den);

                                // Divide to get the right side
                                LXx_VDIV (add2num, add2den);
                                LXx_VCPY (add2, add2num);

                        // Finally, add the two terms together and store the result
                        LXx_VADD3 (this->ABar_b[i], add1, add2);
        }
}

        void
HairMaterial::PrecomputeNG_R () 
{
        /*
         * See "An Artist-Friendly Hair Shading System" pseudocode
         */
        LXtVector result, temp;

        double phiPrime, phi, phi_d;

        LXtVector white = {1.0, 1.0, 1.0};

        for (int i = 0; i < TABLE_ENTRIES; i++) {
                phi = ((.5 + i)/TABLE_ENTRIES)*LXx_PI;

                // To-do: This can probably be done analytically in the case of the artist-friendly system
                LXx_VCLR (result);
                for (int j = 0; j < INTEGRAL_SAMPLES; j++) {
                        phiPrime = ((.5 + j)/INTEGRAL_SAMPLES)*LXx_HALFPI + LXx_HALFPI;

                        if (phiPrime > LXx_PI)
                                phiPrime = phiPrime - 2.0 * LXx_PI;

                        phi_d = phiPrime - phi;
                        if (phi_d > LXx_PI) {
                                phi_d = 2.0 * LXx_PI - phi_d;
                        }
                        else if (phi_d < 0) {
                                phi_d = fabs (phi_d);
                                if (phi_d > LXx_PI) {
                                        phi_d = 2.0 * LXx_PI - phi_d;
                                }
                        }

                        LXx_VCPY (temp, white);
                        LXx_VSCL (temp, this->N_R (phi_d));
                
                        LXx_VADD (result, temp);
                }
                LXx_VSCL (result, 1.0/INTEGRAL_SAMPLES);
                LXx_VCPY (this->NG_R[i], result);
        }
}

        void
HairMaterial::PrecomputeNG_TT () 
{
        /*
         * See "An Artist-Friendly Hair Shading System" pseudocode
         */
        LXtVector result, temp;

        double phiPrime, phi, phi_d;

        LXtVector white = {1.0, 1.0, 1.0};

        for (int i = 0; i < TABLE_ENTRIES; i++) {
                phi = ((.5 + i)/TABLE_ENTRIES)*LXx_PI;

                // To-do: This can probably be done analytically in the case of the artist-friendly system
                LXx_VCLR (result);
                for (int j = 0; j < INTEGRAL_SAMPLES; j++) {
                        phiPrime = ((.5 + j)/INTEGRAL_SAMPLES)*LXx_HALFPI + LXx_HALFPI;

                        if (phiPrime > LXx_PI)
                                phiPrime = phiPrime - 2.0 * LXx_PI;

                        phi_d = phiPrime - phi;
                        if (phi_d > LXx_PI) {
                                phi_d = 2.0 * LXx_PI - phi_d;
                        }
                        else if (phi_d < 0) {
                                phi_d = fabs (phi_d);
                                if (phi_d > LXx_PI) {
                                        phi_d = 2.0 * LXx_PI - phi_d;
                                }
                        }

                        LXx_VCPY (temp, white);
                        LXx_VSCL (temp, this->N_TT (phi_d));
                
                        LXx_VADD (result, temp);
                }
                LXx_VSCL (result, 1.0/INTEGRAL_SAMPLES);
                LXx_VCPY (this->NG_TT[i], result);
        }
}

        void
HairMaterial::PrecomputeNG_TRT () 
{
        /*
         * See "An Artist-Friendly Hair Shading System" pseudocode
         */
        LXtVector result, temp;

        double phiPrime, phi, phi_d;

        LXtVector white = {1.0, 1.0, 1.0};

        for (int i = 0; i < TABLE_ENTRIES; i++) {
                phi = ((.5 + i)/TABLE_ENTRIES)*LXx_PI;

                // To-do: This can probably be done analytically in the case of the artist-friendly system
                LXx_VCLR (result);
                for (int j = 0; j < INTEGRAL_SAMPLES; j++) {
                        phiPrime = ((.5 + j)/INTEGRAL_SAMPLES)*LXx_HALFPI + LXx_HALFPI;

                        if (phiPrime > LXx_PI)
                                phiPrime = phiPrime - 2.0 * LXx_PI;

                        phi_d = phiPrime - phi;
                        if (phi_d > LXx_PI) {
                                phi_d = 2.0 * LXx_PI - phi_d;
                        }
                        else if (phi_d < 0) {
                                phi_d = fabs (phi_d);
                                if (phi_d > LXx_PI) {
                                        phi_d = 2.0 * LXx_PI - phi_d;
                                }
                        }

                        LXx_VCPY (temp, white);
                        LXx_VSCL (temp, this->N_TRT (phi_d));
                
                        LXx_VADD (result, temp);
                }
                LXx_VSCL (result, 1.0/INTEGRAL_SAMPLES);
                LXx_VCPY (this->NG_TRT[i], result);
        }
}

        void
HairMaterial::PrecomputeShaderTables (
        HairMaterialData	*matrData)
{
        // We don't want many threads doing all this precomputation
        // in parallel - although there wouldn't be any race
        // conditions, it seems like it may cause memory thrashing. 
        // Either way, it unnecessarily occupies all the threads doing
        // duplicate work.
        // So we lock the precomputation with a Mutex
                
        this->precomputeMutex.Enter();

        // Check again, in case another thread did set it
        if (!this->isPrecomputed) {
                PrecomputeNormFactors(matrData);
                PrecomputeaBar_f ();
                PrecomputeaBar_b ();
                PrecomputeAlphaBar_f ();
                PrecomputeAlphaBar_b ();
                PrecomputeBetaBar_f ();
                PrecomputeBetaBar_b ();
                PrecomputeDeltaBar_b ();
                PrecomputeSigmaBar_b ();
                PrecomputeABar_b ();
                PrecomputeNG_R ();
                PrecomputeNG_TT ();
                PrecomputeNG_TRT ();

                this->isPrecomputed = true;
        }

        this->precomputeMutex.Leave();
}

/*------------------------------- Luxology LLC --------------------------------
 *
 * Functions to lookup in the precomputed tables
 *
 *----------------------------------------------------------------------------*/

        double			
HairMaterial::lookupNormFactor (
        double			 thetaIn)
{
        double			 result = -1.0;
        LXtVector		 temp;

        int thetaIndex = (thetaIn + LXx_HALFPI)/LXx_PI * TABLE_ENTRIES - .5;

        thetaIndex = CLAMP (thetaIndex, 0, (TABLE_ENTRIES - 1));

        LXx_VCPY (temp, this->normFactors[thetaIndex]);

        for (int i = 0; i < 3; i++)
                result = MAX (result, temp[i]);

        if (result >= .8)
                result = .8 + (result - .8)*1.1;

        result = MAX (result, 1.0);

        return result;
}

        void		
HairMaterial::lookupNormFactor (
        double			 thetaIn,
        LXtVector		*outColor)
{
        this->lookupValue (thetaIn, this->normFactors, outColor);
}

        void			
HairMaterial::lookupValue (
        double			 theta, 
        LXtVector		*table,
        LXtVector		*outColor)
{
        // This is used for looking up a generic value in a theta-based table. Right now it does linear
        // interpolation between the two nearest values
        LXtVector		 temp;

        int index0 = (int)((theta + LXx_HALFPI)/LXx_PI * TABLE_ENTRIES);
        index0 = CLAMP (index0, 0, TABLE_ENTRIES - 1);
        int index1 = index0 + 1;
        index1 = CLAMP (index1, 0, TABLE_ENTRIES - 1);

        if (index0 == index1) {
                LXx_VCPY (*outColor, table[index0]);
                return;
        }

        double off0 = theta - (((double)index0 + .5)/TABLE_ENTRIES*LXx_PI - LXx_HALFPI);
        double off1 = (((double)index1 + .5)/TABLE_ENTRIES*LXx_PI - LXx_HALFPI) - theta;

        LXx_VSCL3 (temp, table[index0], 1.0 - off0/(off0 + off1));
        LXx_VADDS (temp, table[index1], off0/(off0 + off1));

        LXx_VCPY (*outColor, temp);
}

        void			
HairMaterial::lookupaBar_f (
        double			 theta_in, 
        LXtVector		*outColor)
{
        this->lookupValue (theta_in, this->aBar_f, outColor);
}

        void			
HairMaterial::lookupaBar_b (
        double			 theta_in, 
        LXtVector		*outColor)
{
        this->lookupValue (theta_in, this->aBar_b, outColor);
}

        void			
HairMaterial::lookupAlphaBar_f (
        double			 theta_h, 
        LXtVector		*outColor)
{
        this->lookupValue (theta_h, this->AlphaBar_f, outColor);
}

        void			
HairMaterial::lookupAlphaBar_b (
        double			 theta_h, 
        LXtVector		*outColor)
{
        this->lookupValue (theta_h, this->AlphaBar_b, outColor);
}

        void			
HairMaterial::lookupBetaBar_f (
        double			 theta_h, 
        LXtVector		*outColor)
{
        this->lookupValue (theta_h, this->BetaBar_f, outColor);
}

        void			
HairMaterial::lookupBetaBar_b (
        double			 theta_h, 
        LXtVector		*outColor)
{
        this->lookupValue (theta_h, this->BetaBar_b, outColor);
}

        void			
HairMaterial::lookupDeltaBar_b (
        double			 theta_h, 
        LXtVector		*outColor)
{
        this->lookupValue(theta_h, this->DeltaBar_b, outColor);
}

        void			
HairMaterial::lookupSigmaBar_b (
        double			 theta_h, 
        LXtVector		*outColor)
{
        this->lookupValue(theta_h, this->SigmaBar_b, outColor);
}

        void			
HairMaterial::lookupABar_b (
        double			 theta_h, 
        LXtVector		*outColor)
{
        this->lookupValue(theta_h, this->ABar_b, outColor);
}

        void			
HairMaterial::lookupNG_R (
        double			 phi_d, 
        LXtVector		*outColor)
{
        this->lookupValue(phi_d, this->NG_R, outColor);
}

        void			
HairMaterial::lookupNG_TT (
        double			 phi_d, 
        LXtVector		*outColor)
{
        this->lookupValue(phi_d, this->NG_TT, outColor);
}

        void			
HairMaterial::lookupNG_TRT (
        double			 phi_d, 
        LXtVector		*outColor)
{
        this->lookupValue(phi_d, this->NG_TRT, outColor);
}

        void
HairMaterial::ClearShadeComponents (
        LXpShadeComponents	*sCmp) 
{
        LXx_VCLR (sCmp->diff);
        LXx_VCLR (sCmp->diffDir);
        LXx_VCLR (sCmp->diffInd);
        LXx_VCLR (sCmp->diffUns);
        LXx_VCLR (sCmp->spec);
        LXx_VCLR (sCmp->refl);
        LXx_VCLR (sCmp->tran);
        LXx_VCLR (sCmp->subs);
        LXx_VCLR (sCmp->lumi);
        LXx_VCLR (sCmp->illum);
        LXx_VCLR (sCmp->illumDir);
        LXx_VCLR (sCmp->illumInd);
        LXx_VCLR (sCmp->illumUns);
        LXx_VCLR (sCmp->volLum);
        LXx_VCLR (sCmp->volOpa);
}

/*------------------------------- Luxology LLC -------------------------------
 *
 * Compute Global Scattering Essentially Fires a Shadow or GI Ray through hair
 * strands and accumulates the information along the ray, which is then used
 * for the hair shading.
 *
 *----------------------------------------------------------------------------*/

        void
JitterPosition (
        ILxUnknownID		 vector,
        CLxLoc_ShaderService	*shdrSrv,
        LXtVector		 pos,
        float			 rad)
{
        LXtVector		 offset;

        float			 phi, theta, r;
        static const float	 third		= 1.0f/3.0f;

        // r		= Uniform (0,1)
        r	= shdrSrv->NextRandom (vector);		
        // phi		= Uniform (0, 2*pi)
        phi	= shdrSrv->NextRandom (vector) * LXx_PI * 2.0f; 
        // theta	= Inverse sin from -pi/2 to pi/2
        theta	= asin (shdrSrv->NextRandom (vector) - 1.0f);	

        // Unbias the radius to get a uniform distribution, then multiply by the actual radius
        r	= powf (r, third);
        // Weight towards the middle using a linear PDF, which is zero at the edges
        r	= 1.0f - sqrtf(r);
        // Finally, multiply by the actual radius
        r	= r * rad;

        offset[0] = (r * cosf (phi) * sinf (theta));
        offset[1] = (r * sinf (phi) * sinf (theta));
        offset[2] = (r * cosf (theta));

        LXx_VADD (pos, offset);
}

        void
HairMaterial::ComputeGlobalScattering (
        ILxUnknownID		 vector, 
        ILxUnknownID		 rayObj, 
        double			 importance,
        LXtFVector		 rayDir,
        int			 flags,
        double			 lgtDist,
        HairMaterialData	*matrData,
        LXtVector		*T_f, 
        LXtVector		*sigmaBarSqr_f, 
        double			*directFraction,
        LXtFVector		*shadeColor,
        bool			 gi)
{
        ILxUnknownID		 shadeVector;

        CLxLoc_Raycast		 rayCast;

        LXpSamplePosition	*sPos0		= (LXpSamplePosition*) pkt_service.FastPacket (vector, pos_offset);
        LXpParticleSample	*prtSmp0;
        CLxLoc_ShaderService	 shdrSrv;

        LXpSampleRay		*sRay1 = NULL;

        void			*srf0 = NULL;

        LXtVector		 rayPos;
        double			 theta_i;
        double			 dist, giDist;
        int			 setFlags;
        float			 curID, hitID, impMul;

        rayCast.set (rayObj);

        // k will be the number of hits so far
        int k = 0;

        // Get the first surface, so we can check to make sure we are tracing the same surface
        rayCast.GetSurfaceID (vector, &srf0);

        // We need to set the first shadow ray to be exactly on the surface
        LXx_VCPY (rayPos, sPos0->wPos);

        // Set T_f to 1.0
        LXx_VSET(*T_f, 1.0);
        LXx_VSET(*sigmaBarSqr_f, 0.0);
        *directFraction = 1.0;

        if (flags == 0.0)
                return;

        shadeVector = rayCast.RayPush (vector);
        if (!shadeVector) {
                LXx_VCLR (*T_f);
                LXx_VCLR (*sigmaBarSqr_f);
                *directFraction = 0.0;
                return;
        }

        prtSmp0 = (LXpParticleSample*) pkt_service.FastPacket (vector, particle_sample_offset);
        curID = prtSmp0->idParm;

        /*
         * First we trace a ray against the other surfaces in the scene. If it's a shadow ray,
         * and it hits something, we don't need to bother tracing against the fur.
         */

        sRay1 = (LXpSampleRay*) pkt_service.FastPacket (shadeVector, ray_offset);

        // By setting the near clip to 0, modo will automatically adapt it
        // so that it doesn't intersect the current surface
        sRay1->nearClip = 0.0;
        sRay1->importance = importance;

        flags |= LXfRAY_SCOPE_OTHERSURF;
        if (gi) {
                flags |= LXfRAY_EVAL_SHADING;
                giDist = rayCast.Raytrace (shadeVector, rayPos, rayDir, flags);
                LXx_VCPY (*shadeColor, sRay1->color);
                if (LXx_VEQS (sRay1->color, 0.0f)) {
                        LXx_VCLR (*T_f);
                        LXx_VCLR (*sigmaBarSqr_f);
                        *directFraction = 0.0;
                        rayCast.RayPop (vector);
                        return;
                }
        }
        else {
                sRay1->farClip = lgtDist;
                dist = rayCast.Raytrace (shadeVector, rayPos, rayDir, flags);
                if (dist >= 0.0) {
                        // If we hit something else, clear the values and return
                        LXx_VCLR (*T_f);
                        LXx_VCLR (*sigmaBarSqr_f);
                        *directFraction = 0.0;
                        rayCast.RayPop (vector);
                        return;
                }
        }

        /*
         * Now we trace against all the fur polygons, accumulating hits as we go.
         */

        setFlags = LXfRAY_SCOPE_POLYGONS | LXfRAY_SCOPE_IMPLICITSURF  | LXfRAY_SCOPE_VOLUMETRICS | \
                                 LXfRAY_EVAL_OPACITY | LXfRAY_TYPE_SHADOW | LXfRAY_TYPE_SHADOW_INFO | LXfRAY_SCOPE_SAMESURF;

        JitterPosition (shadeVector, &shdrSrv, rayPos, matrData->filterDist);

        while (true) {
                LXpSampleSurfNormal	*sNrm1 = NULL;
                LXpSamplePosition	*sPos1 = NULL;
                LXpParticleSample	*prtSmp1 = NULL;

                sRay1->nearClip = 0.0;
                sRay1->importance = importance;

                if (!gi) // If it's a shadow ray, set its far clip to light
                        sRay1->farClip = lgtDist;
                else if (giDist > 0.0)
                        sRay1->farClip = giDist;

                dist = rayCast.Raytrace (shadeVector, rayPos, rayDir, setFlags);

                // We hit a strand in this surface
                if (dist > 0.0) {
                        LXtVector	 temp;

                        // Get information about the intersection
                        sNrm1 = (LXpSampleSurfNormal*) pkt_service.FastPacket (shadeVector, nrm_offset);
                        sPos1 = (LXpSamplePosition*) pkt_service.FastPacket (shadeVector, pos_offset);
                        prtSmp1 = (LXpParticleSample*) pkt_service.FastPacket (shadeVector, particle_sample_offset);
                        hitID = prtSmp1->idParm;

                        if (gi)
                                giDist -= dist;
                        else
                                lgtDist -= dist;

                        // Don't count if it's the same strand, this is most likely
                        // a capsule end hit
                        if (hitID == curID) {
                                // We need to set the next shadow ray to be at this ray
                                LXx_VCPY (rayPos, sPos1->wPos);
                                continue;
                        }

                        theta_i = this->GetLongAngle(sNrm1->tangent, rayDir);

                        // update T_f
                        lookupaBar_f (theta_i, &temp);
                        LXx_VMUL (*T_f, temp);

                        // update sigmaBarSqr_f
                        lookupBetaBar_f (theta_i, &temp);
                        LXx_VMUL (temp, temp); // need to square it
                        LXx_VADD (*sigmaBarSqr_f, temp);

                        *directFraction = 0.0;

                        // We need to set the next shadow ray to be at this ray
                        LXx_VCPY (rayPos, sPos1->wPos);
                }
                else
                        break;
        
                curID = hitID;
                k++;

                /* Perform Russian Roulette to stochastically stop the shooting process */
                impMul = importance * LXx_LUMAFV (*T_f);
                if (gi)
                        impMul *= LXx_LUMAFV (*shadeColor);
                impMul = shdrSrv.RussianRoulette (shadeVector, impMul);
                if (impMul == 0.0) {
                        LXx_VCLR (*T_f);
                        LXx_VCLR (*sigmaBarSqr_f);
                        *directFraction = 0.0;
                        break;
                }
                else
                        LXx_VSCL (*T_f, impMul);
        }

        Desaturate (*T_f, matrData->globalScatteringSaturation);

        rayCast.RayPop (vector);

        LXx_VSCL (*T_f, matrData->densityFactor);
}

/*
 * These are just basic wrapped functions for clarity.
 */

        void
HairMaterial::ComputeGlobalScatteringGI (
        ILxUnknownID		 vector, 
        ILxUnknownID		 rayObj, 
        double			 importance,
        LXtFVector		 rayDir,
        int			 flags,
        HairMaterialData	*matrData,
        LXtVector		*T_f, 
        LXtVector		*sigmaBarSqr_f, 
        double			*directFraction,
        LXtFVector		*shadeColor)
{
        ComputeGlobalScattering (vector, rayObj, importance, rayDir, flags, 0.0, matrData, T_f, sigmaBarSqr_f, directFraction, shadeColor, true);
}

        void
HairMaterial::ComputeGlobalScatteringShadow (
        ILxUnknownID		 vector, 
        ILxUnknownID		 rayObj, 
        double			 importance,
        LXtFVector		 rayDir,
        int			 flags,
        double			 lgtDist,
        HairMaterialData	*matrData,
        LXtVector		*T_f, 
        LXtVector		*sigmaBarSqr_f, 
        double			*directFraction)
{
        LXtFVector nothing;
        ComputeGlobalScattering (vector, rayObj, importance, rayDir, flags, lgtDist, matrData, T_f, sigmaBarSqr_f, directFraction, &nothing, false);
}

/*------------------------------- Luxology LLC -------------------------------
 *
 * These are the actual shading code, which is essentially a copy
 * of the pseudo-code provided in the Artist-Friendly Hair Shading paper
 * and the Efficient Implementation of the Dual Scattering Model paper.
 *
 *----------------------------------------------------------------------------*/

        void
HairMaterial::ComputeFScatter_s (
        HairMaterialData	*matrData,
        LXtVector		 sigmaBarSqr_f,
        double			 theta_h,
        double			 phi_d,
        LXtVector		 fScatter_s)
{
        LXtVector tempColor, tempColor2;

        LXx_VCLR (fScatter_s);

        LXx_VCPY(tempColor, matrData->primaryHighlightColor);
        this->lookupNG_R (phi_d, &tempColor2);
        LXx_VMUL(tempColor, tempColor2);
        for (int i = 0; i < 3; i++) {
                tempColor[i] *= this->MG_R (theta_h, sigmaBarSqr_f[i]);
        }
        LXx_VSCL(tempColor, matrData->primaryHighlightIntensity);
        LXx_VADD(fScatter_s, tempColor);

        LXx_VCPY(tempColor, matrData->secondaryHighlightColor);
        this->lookupNG_TRT (phi_d, &tempColor2);
        LXx_VMUL(tempColor, tempColor2);
        for (int i = 0; i < 3; i++) {
                tempColor[i] *= this->MG_TRT (theta_h, sigmaBarSqr_f[i]);
        }
        LXx_VSCL(tempColor, matrData->secondaryHighlightIntensity);
        LXx_VADD(fScatter_s, tempColor);

        // Not sure if glints should be here...

        LXx_VCPY(tempColor, matrData->rimLightColor);
        this->lookupNG_TT (phi_d, &tempColor2);
        LXx_VMUL(tempColor, tempColor2);
        for (int i = 0; i < 3; i++) {
                tempColor[i] *= this->MG_TT (theta_h, sigmaBarSqr_f[i]);
        }
        LXx_VSCL(tempColor, matrData->rimLightIntensity);
        LXx_VADD(fScatter_s, tempColor);

        LXx_VMUL(fScatter_s, matrData->fwdScatteringColorAdj);
        LXx_VSCL(fScatter_s, matrData->fwdScatteringIntensityAdj);
}

        void
HairMaterial::ComputeShadingForOneRay (
        LXtVector		 T_f, 
        LXtVector		 sigmaBarSqr_f, 
        double			 directFraction,
        LXtFVector		 hairTangent,
        LXtFVector		 w_i,
        LXtFVector		 w_o,
        double			 gAngle,
        HairMaterialData	*matrData,
        LXpShadeComponents	*sCmp)
{
        LXtVector		 fDirect_back, fScatter_back, fDirect_s, fScatter_s, FScatter;
        LXtVector		 ABar_b, deltaBar_b, sigmaBar_b, sigmaBarSqr_b;

        double			 theta_i, theta_o, theta_d, theta_h, phi;

        theta_o = this->GetLongAngle (hairTangent, w_o);
        theta_i = this->GetLongAngle (hairTangent, w_i);
        theta_d = (theta_o - theta_i)/2.0;
        theta_h = (theta_o + theta_i)/2.0;

        phi = this->GetAzimuthalAngle (hairTangent, w_i, w_o);
        if (phi > LXx_PI) {
                // remap to [-pi, pi]
                phi = phi - 2*LXx_PI;
        }

        // Get values
        this->lookupABar_b (theta_d, &ABar_b);
        this->lookupDeltaBar_b (theta_d, &deltaBar_b);
        this->lookupSigmaBar_b (theta_d, &sigmaBar_b);
        LXx_VMUL3 (sigmaBarSqr_b, sigmaBar_b, sigmaBar_b);

        if (directFraction <= 0.0) {
                // Compute fScatter_back
                for (int i = 0; i < 3; i++) {
                        fScatter_back[i] = UnitAreaGaussian(sigmaBarSqr_b[i] + sigmaBarSqr_f[i] + matrData->bwdScatteringLongWidthAdj,
                                                                theta_h - deltaBar_b[i] + matrData->bwdScatteringLongShift);
                }
                LXx_VMUL (fScatter_back, ABar_b);
                LXx_VSCL (fScatter_back, (2.0 / (LXx_PI * pow (cos (theta_d), 2))));

                LXx_VSCL (fScatter_back, matrData->bwdScatteringIntensityAdj);
                LXx_VMUL (fScatter_back, matrData->bwdScatteringColorAdj);
                Desaturate (fScatter_back, matrData->bwdScatteringSaturation);

                this->ComputeFScatter_s (matrData, sigmaBarSqr_f, theta_h, phi, fScatter_s);
                Desaturate (fScatter_s, matrData->fwdScatteringSaturation);

                // Add FScatter to the shade components
                LXx_VADDS3 (FScatter, fScatter_s, fScatter_back, (LXx_PI * matrData->densityFactor));
                LXx_VMUL (FScatter, T_f);
                LXx_VSCL (FScatter, matrData->densityFactor);

                LXx_VSCL3 (sCmp->subs, FScatter, cos(theta_i));
        }
        else {
                /*
                 * Compute fDirect_back
                 */
                for (int i = 0; i < 3; i++) {
                        fDirect_back[i] = UnitAreaGaussian(sigmaBarSqr_b[i] + matrData->bwdScatteringLongWidthAdj,
                                                                theta_h - deltaBar_b[i] + matrData->bwdScatteringLongShift);
                }
                LXx_VMUL (fDirect_back, ABar_b);
                LXx_VSCL (fDirect_back, (2.0 / (LXx_PI * pow (cos (theta_d), 2))));

                LXx_VSCL (fDirect_back, matrData->bwdScatteringIntensityAdj);
                LXx_VMUL (fDirect_back, matrData->bwdScatteringColorAdj);
                Desaturate (fDirect_back, matrData->bwdScatteringSaturation);

                /*
                 * Compute fDirect_s
                 */
                if (matrData->singleScatteringOn) {
                        this->fPrime_s (theta_i, theta_o, phi, gAngle, matrData, &fDirect_s);
                        LXx_VSCL (fDirect_s, pow(cos (theta_d), 2));
                }
                else {
                        LXx_VCLR (fDirect_s);
                }

                /*
                 * Add to shade components
                 */
                LXx_VSCL3 (sCmp->spec, fDirect_s, cos(theta_i));
                LXx_VSCL3 (sCmp->subs, fDirect_back, matrData->densityFactor * cos(theta_i));
        }
}

        void
HairMaterial::ComputeAmbientShading (
        LXtFVector		 w_i,
        LXtFVector		 hairTangent,
        LXtFVector		 ambientColor,
        LXpShadeComponents	*sCmp)
{
        double			 theta_i;
        LXtVector		 totalIntegrationColor;

        theta_i = this->GetLongAngle (hairTangent, w_i);
        this->lookupNormFactor(theta_i, &totalIntegrationColor);

        LXx_VMUL3 (sCmp->subs, ambientColor, totalIntegrationColor);
}

        void
HairMaterial::ComputeDirectShading (
        ILxUnknownID		 vector,
        ILxUnknownID		 rayObj,
        HairMaterialData	*matrData,
        LXpShadeComponents	*sCmp)
{
        LXpSamplePosition	*samplePosition		= (LXpSamplePosition*) pkt_service.FastPacket (vector, pos_offset);
        LXpSampleSurfNormal	*sNrm			= (LXpSampleSurfNormal*) pkt_service.FastPacket (vector, nrm_offset);
        LXpSampleRay		*sRay			= (LXpSampleRay*) pkt_service.FastPacket (vector, ray_offset);
        LXpParticleSample	*prtSmp			= (LXpParticleSample*) pkt_service.FastPacket (vector, particle_sample_offset);
        LXpLightSource		*lSrc			= (LXpLightSource*) pkt_service.FastPacket (vector, light_source_offset);	
        LXpSamplePosition	 initialSamplePosition	= *samplePosition;
        CLxLoc_ShaderService	 shdrSrv;

        LXpShadeComponents	 sCmpTemp;

        LxResult		 result;

        int			 lights, lightSamples;
        double			 gAngle, lgtDist;

        LXtVector		 T_f;
        LXtVector		 sigmaBarSqr_f;
        double			 directFraction;

        LXtFVector		 rayDir;

        int			 flags;
        int			 shadType;
        int			 castGI = (matrData->giType == 1) || (matrData->giType == 3);

        LXtFVector		 lColor;
        float			 lBrightness, importance;

        LXtFVector		 w_o, w_i;

        CLxLoc_Raycast1		 rayCast;
        rayCast.set (rayObj);

        // Get the number of lights
        lights = rayCast.LightCount (vector);

        // Set up Gangle - particle ID is a random number between 0.0 and 1.0
        gAngle = ((prtSmp->idParm) * LXx_HALFPI + LXx_PI)/6.0;

        // Set the outgoing ray direction to the flipped incoming ray
        LXx_VSCL3 (w_o, sRay->dir, -1.0);
        // Sample all the lights
        for (int i = 0; i < lights; i++) {

                lightSamples = rayCast.LightSampleCountByLight (vector, i);
                lightSamples *= sRay->importance;
                lightSamples = MAX (lightSamples, 1);

                // For a raytraced light, we need to compute shading for each shadow sample
                shadType = rayCast.LightShadowType (vector, i);
                if (shadType == LXiICVAL_LIGHT_SHADTYPE_RAYTRACE ||
                        (shadType == LXiICVAL_LIGHT_SHADTYPE_PORTAL && castGI)) 
                {
                        importance = sRay->importance/lightSamples;

                        // For each light sample
                        for (int j = 0; j < lightSamples; j++) {
                                // Get the first shadow ray
                                result = rayCast.GetNextShadowRay (vector, i, initialSamplePosition.wPos, &rayDir, &flags);
                                lgtDist = lSrc->len;
                                FVectorNormalize(rayDir);

                                if (result == LXe_OK || result == LXe_FALSE) {
                                        this->ComputeGlobalScatteringShadow(vector, rayObj, 
                                                                        importance, rayDir, flags,
                                                                        lgtDist, matrData,
                                                                        &T_f, &sigmaBarSqr_f, &directFraction);
                                }
                                else {
                                        LXx_VCLR (T_f);
                                        directFraction = 0.0;
                                }

                                // If T_f and directFraction are both 0.0, it means the shadow ray hit another surface
                                // and no shading needs to be done
                                if (LXx_VDOT (T_f, T_f) != 0.0 || directFraction != 0.0) {
                                        result = rayCast.GetLightSampleDirection (vector, i, initialSamplePosition.wPos, &rayDir);

                                        this->ClearShadeComponents (&sCmpTemp);

                                        if (result == LXe_OK) {
                                                LXx_VCPY (w_i, rayDir);

                                                ComputeShadingForOneRay (T_f, sigmaBarSqr_f, 
                                                                directFraction, sNrm->tangent, 
                                                                w_i, w_o, gAngle, matrData, &sCmpTemp);
                        
                                                LXx_VCPY (samplePosition->wPos, initialSamplePosition.wPos);

                                                // This has to be done for each sample
                                                rayCast.LightValue (vector, i, &lColor, &lBrightness);

                                                LXx_VMUL (sCmpTemp.spec, lColor);
                                                LXx_VSCL (sCmpTemp.spec, (lBrightness / lightSamples));

                                                LXx_VMUL (sCmpTemp.subs, lColor);
                                                LXx_VSCL (sCmpTemp.subs, (lBrightness / lightSamples));
                                        }

                                        // Add it to the overall shade components
                                        LXx_VADD(sCmp->spec, sCmpTemp.spec);
                                        LXx_VADD(sCmp->subs, sCmpTemp.subs);
                                }
                        }
                }
                else {
                        // Eventually Deep Shadow Mapping Code should go here, we need to compute shading once
                }
        }
}

        void
HairMaterial::ComputeIndirectShading (
        ILxUnknownID		 vector,
        ILxUnknownID		 rayObj,
        HairMaterialData	*matrData,
        LXpShadeComponents	*sCmp)
{
        LXpSamplePosition	*samplePosition		= (LXpSamplePosition*) pkt_service.FastPacket (vector, pos_offset);
        LXpSampleSurfNormal	*sNrm			= (LXpSampleSurfNormal*) pkt_service.FastPacket (vector, nrm_offset);
        LXpSampleRay		*sRay			= (LXpSampleRay*) pkt_service.FastPacket (vector, ray_offset);
        LXpParticleSample	*prtSmp			= (LXpParticleSample*) pkt_service.FastPacket (vector, particle_sample_offset);
        LXpGlobalIndirect	*globInd		= (LXpGlobalIndirect*) pkt_service.FastPacket (vector, global_indirect_offset);	
        LXpGlobalLighting	*globLgt		= (LXpGlobalLighting*) pkt_service.FastPacket (vector, global_lighting_offset);	
        LXpSamplePosition	 initialSamplePosition	= *samplePosition;

        LXpShadeComponents	 sCmpTemp;

        LxResult		 result;

        int			 giRays = 0;
        double			 gAngle;

        LXtVector		 T_f;
        LXtVector		 sigmaBarSqr_f;
        double			 directFraction;

        float			 importance;

        LXtFVector		 rayDir;

        LXtFVector		 lColor;

        LXtFVector		 w_o, w_i;

        CLxLoc_Raycast1		 rayCast;

        rayCast.set (rayObj);

        // Set up Gangle - particle ID is a random number between 0.0 and 1.0
        gAngle = ((prtSmp->idParm) * LXx_HALFPI + LXx_PI)/6.0;

        // Set the outgoing ray direction to the flipped incoming ray
        LXx_VSCL3 (w_o, sRay->dir, -1.0);

        /* Set the number of giRays */
        if ((matrData->giType == 1 || matrData->giType == 3) && globInd->enable) {
                int curBounce;

                rayCast.GetBucketGlobalBounce (vector, &curBounce);

                // We only receive GI if the current bounce is less than the
                // maximum bounces
                if (curBounce < globInd->limit) {
                        giRays = matrData->giRays * sRay->importance * pow (globInd->irrRays2, curBounce);
                        giRays = MAX (giRays, 1);
                }
        }

        importance = sRay->importance / giRays;

        // Sample Global Illumination (using monte-carlo)
        for (int i = 0; i < giRays; i++) {
                int flags;

                result = rayCast.GetNextGIRaySphere (vector, &rayDir, &flags);

                if (result == LXe_OK || result == LXe_FALSE) {
                        this->ComputeGlobalScatteringGI (vector, rayObj, 
                                                        importance, rayDir, flags,
                                                        matrData,
                                                        &T_f, &sigmaBarSqr_f, &directFraction, &lColor);
                }
                else {
                        LXx_VCLR (T_f);
                        directFraction = 0.0;
                }
                        
                // If the surface it hits is black, don't worry about shading it
                if (LXx_VDOT (lColor, lColor) != 0.0) {
                        LXx_VCPY (w_i, rayDir);

                        this->ClearShadeComponents (&sCmpTemp);

                        ComputeShadingForOneRay (T_f, sigmaBarSqr_f, 
                                        directFraction, sNrm->tangent, 
                                        w_i, w_o, gAngle, matrData, &sCmpTemp);
                        
                        LXx_VCPY (samplePosition->wPos, initialSamplePosition.wPos);

                        // Since lcolor represents the returned GI color, we
                        // need to Adj it for giRays AND we need to multiply
                        // by 4 * pi since we're sampling a whole sphere, for
                        // some reason
                        LXx_VSCL (sCmpTemp.spec, 4.0 * LXx_PI / giRays);
                        LXx_VSCL (sCmpTemp.subs, 4.0 * LXx_PI / giRays);

                        LXx_VMUL (sCmpTemp.spec, lColor);
                        LXx_VMUL (sCmpTemp.subs, lColor);

                        // Add it to the overall shade components
                        LXx_VADD(sCmp->spec, sCmpTemp.spec);
                        LXx_VADD(sCmp->subs, sCmpTemp.subs);
                }
        }

        /* If there is no global illumination use ambient color */
        if (!globInd->enable) {
                LXtFVector ambientColor;

                LXx_VCPY (w_i, sRay->dir);
                LXx_VSCL3 (ambientColor, globLgt->ambientColor, globLgt->ambientIntensity);

                this->ClearShadeComponents (&sCmpTemp);
                ComputeAmbientShading (w_i, sNrm->tangent, ambientColor, &sCmpTemp);
                LXx_VADD(sCmp->subs, sCmpTemp.subs);
        }
}

        void
HairMaterial::cmt_ShaderEvaluate (
        ILxUnknownID             vector,
        ILxUnknownID		 rayObj,
        LXpShadeComponents     *sCmp,
        LXpShadeOutput         *sOut,
        void                    *data)
{
        LXpSampleRay		*sRay			= (LXpSampleRay*) pkt_service.FastPacket (vector, ray_offset);
        LXpParticleSample	*prtSmp			= (LXpParticleSample*) pkt_service.FastPacket (vector, particle_sample_offset);
        LXpGlobalIndirect	*globInd		= (LXpGlobalIndirect*) pkt_service.FastPacket (vector, global_indirect_offset);	
        LXpGlobalLighting	*globLgt		= (LXpGlobalLighting*) pkt_service.FastPacket (vector, global_lighting_offset);	
        LXpHairShader		*sHair			= (LXpHairShader*) pkt_service.FastPacket (vector, pkt_offset);

        LXpShadeComponents	 sCmpTemp;
        LXpShadeComponents	 sCmp2;
        
        int			 lights;

        double			 gAngle;

        float			 impMul;

        HairMaterialData	 matrData;

        CLxLoc_Raycast1		 rayCast;
        CLxLoc_ShaderService	 shdrSrv;

        LXxUNUSED(gAngle);
        rayCast.set (rayObj);

        lights = rayCast.LightCount (vector);

        this->CopyHairPacket (sHair, matrData);

        // Return if there's no lighting
        if (lights < 1 && 
                (matrData.giType == 0 || matrData.giType == 2 || matrData.giRays <= 0) &&
                 globLgt->ambientIntensity == 0.0) {
                LXx_VCLR (sOut->color);
                return;
        }

        // Return if this isn't a fur strand
        if (rayCast.GetSurfaceType(vector) != LXi_SURF_FUR) {
                return;
        }

        // Perform Russian Roulette for the whole hair shader
        impMul = shdrSrv.RussianRoulette (vector, sRay->importance);
        if (impMul == 0.0) {
                LXx_VCLR (sOut->color);
                return;
        }

        // Set up Gangle - particle ID is a random number between 0.0 and 1.0
        gAngle = ((prtSmp->idParm) * LXx_HALFPI + LXx_PI)/6.0;

        if (!this->isPrecomputed) {
                PrecomputeShaderTables ( &(this->matrData) );
        }

        // Clear the shade components
        this->ClearShadeComponents (&sCmp2);

        /* 
         * If it's an indirect ray, we check to see if we cast indirect rays,
         * and if we don't then we exit
         */
        if ((sRay->flags & LXfRAY_TYPE_INDIRECT) && globInd->enable) {
                if (matrData.giType == 0 || matrData.giType == 1) {
                        LXx_VSET (sOut->color, 0.0);

                        return;
                }
        }

        // Compute the shading from direct lighting
        this->ClearShadeComponents (&sCmpTemp);
        this->ComputeDirectShading (vector, rayObj, &matrData, &sCmpTemp);
        LXx_VADDS (sCmp2.subs, sCmpTemp.subs, impMul);
        LXx_VADDS (sCmp2.spec, sCmpTemp.spec, impMul);

        // Compute the shading from indirect lighting
        this->ClearShadeComponents (&sCmpTemp);
        this->ComputeIndirectShading (vector, rayObj, &matrData, &sCmpTemp);
        LXx_VADDS (sCmp2.subs, sCmpTemp.subs, impMul);
        LXx_VADDS (sCmp2.spec, sCmpTemp.spec, impMul);

        // Copy the shade components
        *sCmp = sCmp2;

        // Compute the final color
        for (int i = 0; i < 3; i++) 
                sOut->color[i] = sCmp->diff[i] + sCmp->spec[i] + sCmp->refl[i] + sCmp->subs[i];
}

/* -------------------------------------------------------------------------
 *
 * Packet Effects definition:
 * This section is used to define all the possible texture effects for the
 * hair material
 *
 * ------------------------------------------------------------------------- */

class HairPFX : public CLxImpl_PacketEffect
{
        public:
                HairPFX () {}

                static LXtTagInfoDesc	descInfo[];

                LxResult		pfx_Packet (const char **packet) LXx_OVERRIDE;
                unsigned int		pfx_Count (void) LXx_OVERRIDE;
                LxResult		pfx_ByIndex (int idx, const char **name, const char **typeName, int *type) LXx_OVERRIDE;
                LxResult		pfx_Get (int idx,void *packet, float *val, void *item) LXx_OVERRIDE;
                LxResult		pfx_Set (int idx,void *packet, const float *val, void *item) LXx_OVERRIDE;
};

#define SRVs_HAIR_PFX		SRVs_HAIR_MATR

enum {
        SRVs_PRIM_COL_IDX = 0,
        SRVs_SEC_COL_IDX,
        SRVs_RIM_COL_IDX,
        SRVs_GLINTS_COL_IDX,
        SRVs_FS_COL_IDX,
        SRVs_BS_COL_IDX
};

#define SRVs_PRIM_COL_TFX	"primHLCol"
#define SRVs_SEC_COL_TFX	"secHLCol"
#define SRVs_RIM_COL_TFX	"rimCol"
#define SRVs_GLINTS_COL_TFX	"glintsCol"
#define SRVs_FS_COL_TFX		"fsColAdj"
#define SRVs_BS_COL_TFX		"bsColAdj"

LXtTagInfoDesc HairPFX::descInfo[] = {
        { LXsSRV_USERNAME,	"Hair Packet FX" },
        { LXsSRV_LOGSUBSYSTEM,	"texture-effect"},
        { LXsTFX_CATEGORY,	LXsSHADE_SURFACE},
        { 0 }
};

        LxResult
HairPFX::pfx_Packet (const char	**packet) 
{
        packet[0] = SRVs_HAIR_VPACKET;
        return LXe_OK;
}

        unsigned int
HairPFX::pfx_Count (void) 
{
        return 6;
}

        LxResult
HairPFX::pfx_ByIndex (int id, const char **name, const char **typeName, int *type) 
{
        switch (id) {
                case SRVs_PRIM_COL_IDX:
                        name[0]     = SRVs_PRIM_COL_TFX;
                        type[0]     = LXi_TFX_COLOR | LXf_TFX_READ | LXf_TFX_WRITE;
                        typeName[0] = LXsTYPE_COLOR1;
                        break;
                case SRVs_SEC_COL_IDX:
                        name[0]     = SRVs_SEC_COL_TFX;
                        type[0]     = LXi_TFX_COLOR | LXf_TFX_READ | LXf_TFX_WRITE;
                        typeName[0] = LXsTYPE_COLOR1;
                        break;
                case SRVs_RIM_COL_IDX:
                        name[0]     = SRVs_RIM_COL_TFX;
                        type[0]     = LXi_TFX_COLOR | LXf_TFX_READ | LXf_TFX_WRITE;
                        typeName[0] = LXsTYPE_COLOR1;
                        break;
                case SRVs_GLINTS_COL_IDX:
                        name[0]     = SRVs_GLINTS_COL_TFX;
                        type[0]     = LXi_TFX_COLOR | LXf_TFX_READ | LXf_TFX_WRITE;
                        typeName[0] = LXsTYPE_COLOR1;
                        break;
                case SRVs_FS_COL_IDX:
                        name[0]     = SRVs_FS_COL_TFX;
                        type[0]     = LXi_TFX_COLOR | LXf_TFX_READ | LXf_TFX_WRITE;
                        typeName[0] = LXsTYPE_COLOR1;
                        break;
                case SRVs_BS_COL_IDX:
                        name[0]     = SRVs_BS_COL_TFX;
                        type[0]     = LXi_TFX_COLOR | LXf_TFX_READ | LXf_TFX_WRITE;
                        typeName[0] = LXsTYPE_COLOR1;
                        break;
        }
                        
        return	LXe_OK;
}

        LxResult
HairPFX::pfx_Get (int id, void *packet, float *val, void *item) 
{
        LXpHairShader	*hairPacket = (LXpHairShader *) packet;

        switch (id) {
                case SRVs_PRIM_COL_IDX:
                        val[0] = hairPacket->matrData.primaryHighlightColor[0];
                        val[1] = hairPacket->matrData.primaryHighlightColor[1];
                        val[2] = hairPacket->matrData.primaryHighlightColor[2];
                        break;
                case SRVs_SEC_COL_IDX:
                        val[0] = hairPacket->matrData.secondaryHighlightColor[0];
                        val[1] = hairPacket->matrData.secondaryHighlightColor[1];
                        val[2] = hairPacket->matrData.secondaryHighlightColor[2];
                        break;
                case SRVs_RIM_COL_IDX:
                        val[0] = hairPacket->matrData.rimLightColor[0];
                        val[1] = hairPacket->matrData.rimLightColor[1];
                        val[2] = hairPacket->matrData.rimLightColor[2];
                        break;
                case SRVs_GLINTS_COL_IDX:
                        val[0] = hairPacket->matrData.glintsColor[0];
                        val[1] = hairPacket->matrData.glintsColor[1];
                        val[2] = hairPacket->matrData.glintsColor[2];
                        break;
                case SRVs_FS_COL_IDX:
                        val[0] = hairPacket->matrData.fwdScatteringColorAdj[0];
                        val[1] = hairPacket->matrData.fwdScatteringColorAdj[1];
                        val[2] = hairPacket->matrData.fwdScatteringColorAdj[2];
                        break;
                case SRVs_BS_COL_IDX:
                        val[0] = hairPacket->matrData.bwdScatteringColorAdj[0];
                        val[1] = hairPacket->matrData.bwdScatteringColorAdj[1];
                        val[2] = hairPacket->matrData.bwdScatteringColorAdj[2];
                        break;
        }
        
        return LXe_OK;
}

        LxResult
HairPFX::pfx_Set (int id, void *packet, const float *val, void *item) 
{
        LXpHairShader	*hairPacket = (LXpHairShader *) packet;

        switch (id) {
                case SRVs_PRIM_COL_IDX:
                        hairPacket->matrData.primaryHighlightColor[0] = val[0];
                        hairPacket->matrData.primaryHighlightColor[1] = val[1];
                        hairPacket->matrData.primaryHighlightColor[2] = val[2];
                        break;
                case SRVs_SEC_COL_IDX:
                        hairPacket->matrData.secondaryHighlightColor[0] = val[0];
                        hairPacket->matrData.secondaryHighlightColor[1] = val[1];
                        hairPacket->matrData.secondaryHighlightColor[2] = val[2];
                        break;
                case SRVs_RIM_COL_IDX:
                        hairPacket->matrData.rimLightColor[0] = val[0];
                        hairPacket->matrData.rimLightColor[1] = val[1];
                        hairPacket->matrData.rimLightColor[2] = val[2];
                        break;
                case SRVs_GLINTS_COL_IDX:
                        hairPacket->matrData.glintsColor[0] = val[0];
                        hairPacket->matrData.glintsColor[1] = val[1];
                        hairPacket->matrData.glintsColor[2] = val[2];
                        break;
                case SRVs_FS_COL_IDX:
                        hairPacket->matrData.fwdScatteringColorAdj[0] = val[0];
                        hairPacket->matrData.fwdScatteringColorAdj[1] = val[1];
                        hairPacket->matrData.fwdScatteringColorAdj[2] = val[2];
                        break;
                case SRVs_BS_COL_IDX:
                        hairPacket->matrData.bwdScatteringColorAdj[0] = val[0];
                        hairPacket->matrData.bwdScatteringColorAdj[1] = val[1];
                        hairPacket->matrData.bwdScatteringColorAdj[2] = val[2];
                        break;
        }
        
        return LXe_OK;
}

        void
initialize ()
{
        CLxGenericPolymorph*    materialServer = new CLxPolymorph<HairMaterial>;
        CLxGenericPolymorph*    packetServer = new CLxPolymorph<HairPacket>;
        CLxGenericPolymorph*    FXServer = new CLxPolymorph<HairPFX>;

        materialServer->AddInterface (new CLxIfc_CustomMaterial<HairMaterial>);
        materialServer->AddInterface (new CLxIfc_ChannelUI<HairMaterial>);
        materialServer->AddInterface (new CLxIfc_StaticDesc<HairMaterial>);
        lx::AddServer (SRVs_HAIR_MATR, materialServer);

        packetServer->AddInterface (new CLxIfc_VectorPacket<HairPacket>);
        packetServer->AddInterface (new CLxIfc_StaticDesc<HairPacket>);
        lx::AddServer (SRVs_HAIR_VPACKET, packetServer);

        FXServer->AddInterface (new CLxIfc_PacketEffect<HairPFX>);
        FXServer->AddInterface (new CLxIfc_StaticDesc<HairPFX>);
        lx::AddServer (SRVs_HAIR_PFX, FXServer);
}

