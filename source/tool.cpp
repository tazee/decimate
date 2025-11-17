//
// Polygon reduction tool and tool operator
//

#include "tool.hpp"
#include "command.hpp"

/*
 * On create we add our one tool attribute. We also allocate a vector type
 * and select mode mask.
 */
CTool::CTool()
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

    CLxUser_PacketService sPkt;
    CLxUser_MeshService   sMesh;

    dyna_Add(ATTRs_MODE, LXsTYPE_INTEGER);
    dyna_SetHint(ATTRa_MODE, decimate_mode);

    dyna_Add(ATTRs_RATIO, LXsTYPE_PERCENT);

    dyna_Add(ATTRs_COUNT, LXsTYPE_INTEGER);

    dyna_Add(ATTRs_COST, LXsTYPE_INTEGER);
    dyna_SetHint(ATTRa_COST, decimate_cost);

    dyna_Add(ATTRs_PREBND, LXsTYPE_BOOLEAN);

    dyna_Add(ATTRs_PREMAT, LXsTYPE_BOOLEAN);

    tool_Reset();

    sPkt.NewVectorType(LXsCATEGORY_TOOL, v_type);
    sPkt.AddPacket(v_type, LXsP_TOOL_VIEW_EVENT, LXfVT_GET);
    sPkt.AddPacket(v_type, LXsP_TOOL_SCREEN_EVENT, LXfVT_GET);
    sPkt.AddPacket(v_type, LXsP_TOOL_FALLOFF, LXfVT_GET);
    sPkt.AddPacket(v_type, LXsP_TOOL_SUBJECT2, LXfVT_GET);
    sPkt.AddPacket(v_type, LXsP_TOOL_INPUT_EVENT, LXfVT_GET);
	sPkt.AddPacket (v_type, LXsP_TOOL_EVENTTRANS,  LXfVT_GET);
	sPkt.AddPacket (v_type, LXsP_TOOL_ACTCENTER,   LXfVT_GET);

    offset_view = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_VIEW_EVENT);
    offset_screen = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_SCREEN_EVENT);
    offset_falloff = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_FALLOFF);
    offset_subject = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_SUBJECT2);
    offset_input = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_INPUT_EVENT);
	offset_event  = sPkt.GetOffset (LXsCATEGORY_TOOL, LXsP_TOOL_EVENTTRANS);
	offset_center = sPkt.GetOffset (LXsCATEGORY_TOOL, LXsP_TOOL_ACTCENTER);
    mode_select = sMesh.SetMode("select");

    m_count0 = 0;
    m_ratio0 = 0.0;
}

/*
 * Reset sets the attributes back to defaults.
 */
void CTool::tool_Reset()
{
    CDecimate dec;
    dyna_Value(ATTRa_MODE).SetInt(CDecimate::Ratio);
    dyna_Value(ATTRa_RATIO).SetFlt(1.0);
    dyna_Value(ATTRa_COUNT).SetInt(0);
    dyna_Value(ATTRa_COST).SetInt(CDecimate::Lindstrom_Turk);
    dyna_Value(ATTRa_PREBND).SetInt(0);
    dyna_Value(ATTRa_PREMAT).SetInt(0);
}

/*
 * Boilerplate methods that identify this as an action (state altering) tool.
 */
LXtObjectID CTool::tool_VectorType()
{
    return v_type.m_loc;  // peek method; does not add-ref
}

const char* CTool::tool_Order()
{
    return LXs_ORD_ACTR;
}

LXtID4 CTool::tool_Task()
{
    return LXi_TASK_ACTR;
}

LxResult CTool::tool_GetOp(void** ppvObj, unsigned flags)
{
    CLxSpawner<CToolOp> spawner(SRVNAME_TOOLOP);
    CToolOp*            toolop = spawner.Alloc(ppvObj);

	if (!toolop)
	{
		return LXe_FAILED;
	}

    dyna_Value(ATTRa_MODE).GetInt(&toolop->m_mode);
    dyna_Value(ATTRa_RATIO).GetFlt(&toolop->m_ratio);
    dyna_Value(ATTRa_COUNT).GetInt(&toolop->m_count);
    dyna_Value(ATTRa_COST).GetInt(&toolop->m_cost);
    dyna_Value(ATTRa_PREBND).GetInt(&toolop->m_preserveBoundary);
    dyna_Value(ATTRa_PREMAT).GetInt(&toolop->m_preserveMaterial);

    toolop->offset_view = offset_view;
    toolop->offset_screen = offset_screen;
    toolop->offset_falloff = offset_falloff;
    toolop->offset_subject = offset_subject;
    toolop->offset_input = offset_input;

	return LXe_OK;
}

LXtTagInfoDesc CTool::descInfo[] =
{
	{LXsTOOL_PMODEL, "."},
	{LXsTOOL_USETOOLOP, "."},
	{LXsPMODEL_SELECTIONTYPES, LXsSELOP_TYPE_POLYGON},
	{0}

};

/*
 * We employ the simplest possible tool model -- default hauling. We indicate
 * that we want to haul one attribute, we name the attribute, and we implement
 * Initialize() which is what to do when the tool activates or re-activates.
 * In this case set the axis to the current value.
 */
unsigned CTool::tmod_Flags()
{
    return LXfTMOD_I0_INPUT | LXfTMOD_DRAW_3D;
}

LxResult CTool::tmod_Enable(ILxUnknownID obj)
{
    CLxUser_Message msg(obj);
    unsigned int primary_index = 0;

    if (TestPolygon(primary_index) == false)
    {
        msg.SetCode(LXe_CMD_DISABLED);
        msg.SetMessage(SRVNAME_TOOL, "NoPolygon", 0);
        return LXe_DISABLED;
    }

	CLxUser_LayerService _lyr_svc;
	CLxSceneSelection	 scene_sel;
	CLxUser_Scene		 scene;

	scene_sel.Get (scene);
	if (scene.test ())
		_lyr_svc.SetScene (scene);

	if (_lyr_svc.IsProcedural (primary_index) == LXe_TRUE)
	{
        msg.SetCode(LXe_CMD_DISABLED);
        msg.SetMessage(SRVNAME_TOOL, "proceduralMesh", 0);
        return LXe_DISABLED;
	}
    return LXe_OK;
}

LxResult CTool::tmod_Down(ILxUnknownID vts, ILxUnknownID adjust)
{
	CLxUser_AdjustTool	 at (adjust);
	CLxUser_VectorStack	 vec (vts);
	LXpToolActionCenter* acen = (LXpToolActionCenter *) vec.Read (offset_center);
	LXpToolInputEvent*   ipkt = (LXpToolInputEvent *) vec.Read (offset_input);

    dyna_Value(ATTRa_RATIO).GetFlt(&m_ratio0);
    dyna_Value(ATTRa_COUNT).GetInt(&m_count0);

    return LXe_TRUE;
}

void CTool::tmod_Move(ILxUnknownID vts, ILxUnknownID adjust)
{
    CLxUser_AdjustTool at(adjust);
    CLxUser_VectorStack vec(vts);
    LXpToolScreenEvent*  spak = static_cast<LXpToolScreenEvent*>(vec.Read(offset_screen));

    int mode;
    dyna_Value(ATTRa_MODE).GetInt(&mode);

    if (mode == CDecimate::Count)
    {
        double delta = spak->cx - spak->px;
        int count = m_count0 + static_cast<int>(delta * 0.1);
        if (count < 0)
            count = 0;
        at.SetInt(ATTRa_COUNT, count);
    }
    else if (mode == CDecimate::Ratio)
    {
        double delta = spak->cx - spak->px;
        double ratio = m_ratio0 - delta * 0.001;
        if (ratio < 0.0)
            ratio = 0.0;
        if (ratio > 1.0)
            ratio = 1.0;
        at.SetFlt(ATTRa_RATIO, ratio);
    }
}

void CTool::tmod_Up(ILxUnknownID vts, ILxUnknownID adjust)
{
    m_count0 = 0;
    m_ratio0 = 0.0;
}

void CTool::atrui_UIHints2(unsigned int index, CLxUser_UIHints& hints)
{
    switch (index)
    {
        case ATTRa_RATIO:
            hints.MinFloat(0.0);
            hints.MaxFloat(1.0);
            break;
        
        case ATTRa_COUNT:
            hints.MinInt(0);
            break;
    }
}

LxResult CTool::atrui_DisableMsg (unsigned int index, ILxUnknownID msg)
{
    CLxUser_Message		 message (msg);

    int mode;
    dyna_Value(ATTRa_MODE).GetInt(&mode);

    switch (index) {
        case ATTRa_RATIO:
            if (mode != CDecimate::Ratio)
            {
                message.SetCode (LXe_DISABLED);
                message.SetMessage ("tool.decimate", "OnlyRatio", 0);
                return LXe_DISABLED;
            }
            break;
        case ATTRa_COUNT:
            if (mode != CDecimate::Count)
            {
                message.SetCode (LXe_DISABLED);
                message.SetMessage ("tool.decimate", "OnlyCount", 0);
                return LXe_DISABLED;
            }
            break;
    }
    return LXe_OK;
}

bool CTool::TestPolygon(unsigned int& primary_index)
{
    /*
     * Start the scan in read-only mode.
     */
    CLxUser_LayerScan scan;
    CLxUser_Mesh      mesh;
    unsigned          i, n, count;
    bool              ok = false;

    primary_index = 0;

    s_layer.BeginScan(LXf_LAYERSCAN_ACTIVE | LXf_LAYERSCAN_MARKPOLYS, scan);

    /*
     * Count the polygons in all mesh layers.
     */
    if (scan)
    {
        n = scan.NumLayers();
        for (i = 0; i < n; i++)
        {
            scan.BaseMeshByIndex(i, mesh);
            mesh.PolygonCount(&count);
            if (count > 0)
            {
                ok = true;
                primary_index = i;
                break;
            }
        }
        scan.Apply();
    }

    /*
     * Return false if there is no polygons in any active layers.
     */
    return ok;
}

LxResult CTool::cui_Enabled (const char *channelName, ILxUnknownID msg_obj, ILxUnknownID item_obj, ILxUnknownID read_obj)
{
	CLxUser_Item	 	 item (item_obj);
	CLxUser_ChannelRead	 chan_read (read_obj);

    std::string name(channelName);

	if (name == ATTRs_RATIO)
    {
        if (chan_read.IValue (item, ATTRs_MODE) != CDecimate::Ratio)
		    return LXe_CMD_DISABLED;
    }
	else if (name == ATTRs_COUNT)
    {
        if (chan_read.IValue (item, ATTRs_MODE) != CDecimate::Count)
		    return LXe_CMD_DISABLED;
    }
	
	return LXe_OK;
}

LxResult CTool::cui_DependencyCount (const char *channelName, unsigned *count)
{
	count[0] = 0;

	if (std::string(channelName) == ATTRs_RATIO)
		count[0] = 1;
	else if (std::string(channelName) == ATTRs_COUNT)
		count[0] = 1;
	
	return LXe_OK;
}

LxResult CTool::cui_DependencyByIndex (const char *channelName, unsigned index, LXtItemType *depItemType, const char **depChannel)
{
	depItemType[0] = m_itemType;
	
	if (std::string(channelName) == ATTRs_RATIO)
	{
		depChannel[0] = ATTRs_MODE;
		return LXe_OK;
	}	
	else if (std::string(channelName) == ATTRs_COUNT)
	{
		depChannel[0] = ATTRs_MODE;
		return LXe_OK;
	}
		
	return LXe_OUTOFBOUNDS;
}

/*
 * Tool evaluation uses layer scan interface to walk through all the active
 * meshes and visit all the selected polygons.
 */
LxResult CToolOp::top_Evaluate(ILxUnknownID vts)
{
    CLxUser_VectorStack vec(vts);

    /*
     * Start the scan in edit mode.
     */
    CLxUser_LayerScan  scan;
    CLxUser_Mesh       base_mesh, edit_mesh;
    CDecimate          dec;

    if (m_ratio >= 1.0 && m_mode == CDecimate::Ratio)
        return LXe_OK;

    if (m_count == 0 && m_mode == CDecimate::Count)
        return LXe_OK;

    if (vec.ReadObject(offset_subject, subject) == false)
        return LXe_FAILED;
    if (vec.ReadObject(offset_falloff, falloff) == false)
        return LXe_FAILED;

    CLxUser_MeshService   s_mesh;

    LXtMarkMode pick = s_mesh.SetMode(LXsMARK_SELECT);

    subject.BeginScan(LXf_LAYERSCAN_EDIT_POLYS, scan);

    dec.m_mode = m_mode;
    dec.m_ratio = m_ratio;
    dec.m_count = m_count;
    dec.m_cost = m_cost;
    dec.m_preserveBoundary = m_preserveBoundary;
    dec.m_preserveMaterial = m_preserveMaterial;

    auto n = scan.NumLayers();
    for (auto i = 0u; i < n; i++)
    {
        scan.BaseMeshByIndex(i, base_mesh);
        scan.EditMeshByIndex(i, edit_mesh);

        dec.DecimateMesh(base_mesh);
        dec.m_cmesh.ApplyMesh(edit_mesh, dec.m_triple);

        scan.SetMeshChange(i, LXf_MESHEDIT_GEOMETRY);
    }

    scan.Apply();
    return LXe_OK;
}


/*
 * Export tool server.
 */
void initialize()
{
    CLxGenericPolymorph* srv;

    srv = new CLxPolymorph<CTool>;
    srv->AddInterface(new CLxIfc_Tool<CTool>);
    srv->AddInterface(new CLxIfc_ToolModel<CTool>);
    srv->AddInterface(new CLxIfc_Attributes<CTool>);
    srv->AddInterface(new CLxIfc_AttributesUI<CTool>);
    srv->AddInterface(new CLxIfc_ChannelUI<CTool>);
    srv->AddInterface(new CLxIfc_StaticDesc<CTool>);
    thisModule.AddServer(SRVNAME_TOOL, srv);

    srv = new CLxPolymorph<CToolOp>;
    srv->AddInterface(new CLxIfc_ToolOperation<CToolOp>);
    lx::AddSpawner(SRVNAME_TOOLOP, srv);

    CCommand::initialize();
}
