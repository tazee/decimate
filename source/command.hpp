//
// Test command to export CMesh to new mesh item
//

#pragma once

#include "decimate.hpp"
#include "util.hpp"

#include <lxsdk/lx_layer.hpp>
#include <lxsdk/lxu_math.hpp>
#include <lxsdk/lxu_vector.hpp>
#include <lxsdk/lxu_command.hpp>

#define ATTRs_MODE   "mode"
#define ATTRs_RATIO  "ratio"
#define ATTRs_COUNT  "count"
#define ATTRs_COST   "costStrategy"
#define ATTRs_PREBND "preserveBoundary"
#define ATTRs_PREMAT "preserveMaterial"

#define ATTRa_MODE     0
#define ATTRa_RATIO    1
#define ATTRa_COUNT    2
#define ATTRa_COST     3
#define ATTRa_PREBND   4
#define ATTRa_PREMAT   5

class CCommand : public CLxBasicCommand
{
public:
    CLxUser_LayerService lyr_S;
    CLxUser_MeshService  msh_S;
    unsigned             select_mode;

    CCommand()
    {
        static const LXtTextValueHint decimate_mode[] = {
            { CDecimate::Ratio, "ratio" }, 
            { CDecimate::Count, "count" }, 
            { 0, "=decimate_mode" }, 0
        };
        static const LXtTextValueHint decimate_cost[] = {
            { CDecimate::Edge_Length, "Edge_Length" }, 
            { CDecimate::Lindstrom_Turk, "Lindstrom_Turk" }, 
            { CDecimate::Garland_Heckbert, "Garland_Heckbert" }, 
            { 0, "=decimate_cost" }, 0
        };

        dyna_Add(ATTRs_MODE, LXsTYPE_INTEGER);
        dyna_SetHint(ATTRa_MODE, decimate_mode);

        dyna_Add(ATTRs_RATIO, LXsTYPE_PERCENT);

        dyna_Add(ATTRs_COUNT, LXsTYPE_INTEGER);

        dyna_Add(ATTRs_COST, LXsTYPE_INTEGER);
        dyna_SetHint(ATTRa_COST, decimate_cost);

        dyna_Add(ATTRs_PREBND, LXsTYPE_BOOLEAN);

        dyna_Add(ATTRs_PREMAT, LXsTYPE_BOOLEAN);

        select_mode = msh_S.SetMode(LXsMARK_SELECT);
    }

    static void initialize()
    {
        CLxGenericPolymorph* srv;

        srv = new CLxPolymorph<CCommand>;
        srv->AddInterface(new CLxIfc_Command<CCommand>);
        srv->AddInterface(new CLxIfc_Attributes<CCommand>);
        srv->AddInterface(new CLxIfc_AttributesUI<CCommand>);
        lx::AddServer("decimate.test", srv);
    }

    int basic_CmdFlags()
    {
        return LXfCMD_MODEL | LXfCMD_UNDO;
    }

    void basic_Execute(unsigned int flags)
    {
	    CLxUser_ChannelWrite    chanWrite;
		CLxSceneSelection       sel_scene;
		CLxUser_Scene           scene;
        CLxUser_Item            meshItem;
        CLxUser_Mesh		    base_mesh, new_mesh;
        unsigned		        index;
        CLxUser_MeshService     mS;
        CLxUser_LayerService    lyr_S;
        CLxUser_LayerScan       scan;
        unsigned                n;
        CDecimate               dec;

        dyna_Value(ATTRa_MODE).GetInt(&dec.m_mode);
        dyna_Value(ATTRa_RATIO).GetFlt(&dec.m_ratio);
        dyna_Value(ATTRa_COUNT).GetInt(&dec.m_count);
        dyna_Value(ATTRa_COST).GetInt(&dec.m_cost);
        dyna_Value(ATTRa_PREBND).GetInt(&dec.m_preserveBoundary);
        dyna_Value(ATTRa_PREMAT).GetInt(&dec.m_preserveMaterial);
    
		sel_scene.Get(scene);
    
        lyr_S.BeginScan(LXf_LAYERSCAN_ACTIVE, scan);
        scan.Count(&n);

        for (auto i = 0u; i < n; i++)
        {
            scan.BaseMeshByIndex(i, base_mesh);

            dec.DecimateMesh(base_mesh);

            scene.NewItem(LXsTYPE_MESH, meshItem);

            if (LXx_OK (meshItem.ChannelLookup (LXsICHAN_MESH_MESH, &index))) {
                meshItem.GetContext (scene);
                scene.SetChannels (chanWrite, LXs_ACTIONLAYER_EDIT, 0.0);
                if (chanWrite.Object (meshItem, index, new_mesh)) {
                    dec.m_cmesh.WriteMesh(new_mesh);
                }
            }
        }
    }

    LxResult cmd_DialogInit(void)
    {
        if (LXxCMDARG_ISSET(dyna_GetFlags(ATTRa_MODE)) == false)
        {
            attr_SetInt(ATTRa_MODE, CDecimate::Ratio);
        }
        if (LXxCMDARG_ISSET(dyna_GetFlags(ATTRa_RATIO)) == false)
        {
            attr_SetFlt(ATTRa_RATIO, 1.0);
        }
        if (LXxCMDARG_ISSET(dyna_GetFlags(ATTRa_COUNT)) == false)
        {
            attr_SetInt(ATTRa_COUNT, 0);
        }
        if (LXxCMDARG_ISSET(dyna_GetFlags(ATTRa_COST)) == false)
        {
            attr_SetInt(ATTRa_COST, CDecimate::Lindstrom_Turk);
        }
        if (LXxCMDARG_ISSET(dyna_GetFlags(ATTRa_PREBND)) == false)
        {
            attr_SetInt(ATTRa_PREBND, 0);
        }
        if (LXxCMDARG_ISSET(dyna_GetFlags(ATTRa_PREMAT)) == false)
        {
            attr_SetInt(ATTRa_PREMAT, 0);
        }

        return LXe_OK;
    }
};
