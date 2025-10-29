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


class CCommand : public CLxBasicCommand
{
public:
    CLxUser_LayerService lyr_S;
    CLxUser_MeshService  msh_S;
    unsigned             select_mode;

    CCommand()
    {
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
    
		sel_scene.Get(scene);
    
        lyr_S.BeginScan(LXf_LAYERSCAN_ACTIVE, scan);
        scan.Count(&n);

        for (auto i = 0u; i < n; i++)
        {
            scan.BaseMeshByIndex(i, base_mesh);
            CDecimate dec;
            dec.m_mode = CDecimate::Ratio;
            dec.m_ratio = 0.5;
            dec.m_count = 0;
            dec.m_cost = CDecimate::Garland_Heckbert;
            dec.m_preserveBoundary = 1;
            dec.m_preserveMaterial = 1;
            dec.BuildMesh(base_mesh);
            dec.DecimateMesh(base_mesh);

            scene.NewItem(LXsTYPE_MESH, meshItem);

            if (LXx_OK (meshItem.ChannelLookup (LXsICHAN_MESH_MESH, &index))) {
                meshItem.GetContext (scene);
                scene.SetChannels (chanWrite, LXs_ACTIONLAYER_EDIT, 0.0);
                if (chanWrite.Object (meshItem, index, new_mesh)) {
                    dec.WriteMesh(new_mesh);
                }
            }
        }
    }
};
