/*
 * Command: morphDeform.mapName - Set and display a popup of morph names for morphDeform items
 *
 *  Copyright 0000
 *
 * This module implements the command:
 *
 *	morphDeform.mapName <name>
 */
#include <lxsdk/lxu_command.hpp>
#include <lxsdk/lxu_select.hpp>
#include <lxsdk/lx_listener.hpp>
#include <lxsdk/lx_action.hpp>
#include <lxsdk/lx_item.hpp>
#include <lxsdk/lx_mesh.hpp>
#include <lxsdk/lx_deform.hpp>
#include <lxsdk/lx_layer.hpp>
#include <lxsdk/lxidef.h>
#include <string>
#include <vector>
#include <set>
#include <algorithm>

// not used, but included to make it part of the SDK
#include <lxsdk/lx_undo.hpp>


	namespace Command_Morph {	// disambiguate everything with a namespace

#define MORPHDEF_ITEMTYPE		LXsITYPE_MORPHDEFORM
#define Cs_MORPH_MAPNAME		LXsICHAN_MORPHDEFORM_MAPNAME

static CLxItemType	morphDeformType (MORPHDEF_ITEMTYPE);
static CLxItemType	morphContType (LXsITYPE_MORPHCONTAINER);
static CLxItemType	meshInstType (LXsITYPE_MESHINST);
static CLxItemType	meshType (LXsITYPE_MESH);


/*
 * ----------------------------------------------------------------
 * The selection tracker class keeps track of when item selection changes and
 * takes care of building a list of unique item types for the current state.
 * It's also in charge of enumeration, which is why we have a vistor class.
 */
class CItemVisitor
{
    public:
        virtual bool	Item (CLxUser_Item &item) = 0;
};

class CItemSelectionTracker :
                public CItemVisitor,
                public CLxImpl_SelectionListener,
                public CLxSingletonPolymorph
{
    public:
        CLxUser_SceneService		 srv_scene;
        CLxUser_SelectionService	 srv_sel;
        CLxUser_ItemPacketTranslation	 pkt_item;
        LXtID4				 selID_item;
        bool				 is_valid;
        unsigned			 use_count;
        std::set<LXtItemType>		 cur_types;

        LXxSINGLETON_METHOD;

        CItemSelectionTracker ()
        {
                is_valid  = false;
                use_count = 1;

                AddInterface (new CLxIfc_SelectionListener<CItemSelectionTracker>);

                selID_item = srv_sel.LookupType ("item");
                pkt_item.autoInit ();
        }

                void
        selevent_Add (
                LXtID4			 type,
                unsigned int		 subtype)
                                         LXx_OVERRIDE
        {
                if (type == selID_item)
                        is_valid = false;
        }

                void
        selevent_Remove (
                LXtID4			 type,
                unsigned int		 subtype)
                                         LXx_OVERRIDE
        {
                if (type == selID_item)
                        is_valid = false;
        }

                void
        Enumerate (
                CItemVisitor		&vis)
        {
                CLxUser_Item		 item;
                LXtScanInfoID		 scan;
                void			*pkt;

                scan = 0;
                while ((scan = srv_sel.ScanLoopCurrent (scan, selID_item, &pkt))) {
                        pkt_item.GetItem (pkt, item);
                        if (vis.Item (item))
                                return;
                }
        }

                void
        ValidateTypeSet ()
        {
                cur_types.clear ();
                Enumerate (*this);
        }

                bool
        Item (
                CLxUser_Item		&item) 
                                        LXx_OVERRIDE
        {
                cur_types.insert (item.Type ());
                return false;
        }

                bool
        AllowType (
                LXtItemType		 type)
        {
                std::set<LXtItemType>::iterator	 sit;

                if (!is_valid)
                        ValidateTypeSet ();

                for (sit = cur_types.begin(); sit != cur_types.end(); sit++)
                        if (srv_scene.ItemTypeTest (*sit, type) == LXe_TRUE)
                                return true;

                return false;
        }
};


/*
 * We only need one selection tracker for however many instances of commands, so
 * we keep a count.
 */
static CItemSelectionTracker	*sT = 0;

        void
SelTrack_Acquire (void)
{
        if (sT) {
                sT->use_count++;
                return;
        }

        CLxUser_ListenerService	 ls;

        sT = new CItemSelectionTracker;
        ls.AddListener (*sT);
}

        void
SelTrack_Release (void)
{
        sT->use_count--;
        if (sT->use_count)
                return;

        CLxUser_ListenerService	 ls;

        ls.RemoveListener (*sT);
        delete sT;
        sT = 0;
}



/*
 * ----------------------------------------------------------------
 * The command has the usual collection of basic methods,
 * plus a method for customizing argument UI.
 */

#define ARGi_MAP		 0

class CMorphDefCommand : public CLxBasicCommand
{
    public:
         CMorphDefCommand ();
        ~CMorphDefCommand ();

        int		basic_CmdFlags	() LXx_OVERRIDE;
        bool		basic_Notifier	(int index, std::string &name, std::string &args) LXx_OVERRIDE;
        bool		basic_Enable	(CLxUser_Message &msg) LXx_OVERRIDE;

        CLxDynamicUIValue *
                        atrui_UIValue	(unsigned int index) LXx_OVERRIDE;

        void		cmd_Execute	(unsigned int flags) LXx_OVERRIDE;
        LxResult	cmd_Query	(unsigned int index, ILxUnknownID vaQuery) LXx_OVERRIDE;

        CLxUser_SceneService		 srv_scene;
};


CMorphDefCommand::CMorphDefCommand ()
{
	dyna_Add (Cs_MORPH_MAPNAME, LXsTYPE_STRING);
	basic_SetFlags (0, LXfCMDARG_QUERY);

        SelTrack_Acquire ();
}

CMorphDefCommand::~CMorphDefCommand ()
{
        SelTrack_Release ();
}


        int
CMorphDefCommand::basic_CmdFlags ()
{
        return LXfCMD_MODEL | LXfCMD_UNDO;
}


// This is probably important for gettng the popup to update.  Searching for docs or an example as yet, fruitless.
	bool
CMorphDefCommand::basic_Notifier (
        int			 index,
        std::string		&name,
        std::string		&args)
{
        if (index == 0) {
                name = LXsNOTIFIER_SELECT;
                args = "item +v";

        }
	else if (index == 1) {
                name = LXsNOTIFIER_CHANNEL;
                args = "+d " Cs_MORPH_MAPNAME " " MORPHDEF_ITEMTYPE;
        }
	else if (index == 2) {
                name = LXsNOTIFIER_MESHES;
                args = "+l";
	} else if (index == 3) {
		name = LXsNOTIFIER_GRAPHS;
		args = LXsGRAPH_DEFORMERS " +d";
	} else
		return false;

	return true;
}


/*
 * Enable -- test if there's anything selected.
 */
class CEnableItemVisitor : public CItemVisitor
{
    public:
	bool		 any;

	bool Item (CLxUser_Item &item)
	{
		// Check if item is morph influence.
		if (!item.IsA (morphDeformType))
			return false;

		// Check morph influence links
		CLxUser_ItemGraph	 graph;
		CLxUser_Item		 othr;
		int			 i, count;

		graph.from (item, LXsGRAPH_DEFORMERS);
		count = graph.Forward (item);
		for (i = 0; i < count; i++) {
			// if linked item is a morph container
			graph.Forward (item, i, othr);
			if (othr.IsA (morphContType))
				return false;
		}

		any = true;
		return true;
	}
};

        bool
CMorphDefCommand::basic_Enable (
        CLxUser_Message		&msg)
{
        CEnableItemVisitor	 vis;

        vis.any = false;
        sT->Enumerate (vis);
        return vis.any;
}


/*
 * Query -- return a list of all selected item name channel values 
 * perhaps this should also filtered by the value of the type
 */
class CQueryItemVisitor : public CItemVisitor
{
    public:
        CLxUser_ValueArray	 va;

        bool Item (CLxUser_Item &item)
        {
		CLxUser_Scene		 scene;
		CLxUser_ChannelRead	 rchan;
		unsigned		 index;
		const char		*name = NULL;

		item.GetContext (scene);
		scene.GetChannels (rchan, 0.0);
		if (LXx_OK (item.ChannelLookup (Cs_MORPH_MAPNAME, &index)))
			if (LXx_OK (rchan.String (item, index, &name))) // HA, this looks like a User class method with a bool return, but it falls through to the LXe return local method.
				va.Add (name);

                return false;
        }
};

        LxResult
CMorphDefCommand::cmd_Query (
        unsigned int		 index,
        ILxUnknownID		 vaQuery)
{
        CQueryItemVisitor	 vis;

        vis.va.set (vaQuery);
        sT->Enumerate (vis);
        return LXe_OK;
}



class CMorphPopup : public CLxDynamicUIValue
{
    public:
	int				 wgtType;
	std::vector<std::string>	 morfNames;

        unsigned	Flags ()
                        LXx_OVERRIDE
        {
                return LXfVALHINT_POPUPS;
        }

        unsigned	PopCount ()
                        LXx_OVERRIDE
        {
		return morfNames.size ();
        }

	const char*	PopUserName (unsigned index)
                        LXx_OVERRIDE
        {
		if(index>=morfNames.size ())
			return "OOB";
		return morfNames[index].c_str ();
        }

	const char*	PopInternalName (unsigned index)
                        LXx_OVERRIDE
        {
		if(index>=morfNames.size ())
			return "OOB";
		return morfNames[index].c_str ();
        }

	int		BuildNameLists ();
};

/*------------------------------------------------------------------------ 04/11
 * Mesh map visitor. Builds an array of mesh map names, which can be
 * used to select the maps to enable iterating over their elements.
 *----------------------------------------------------------------------------*/
class MeshMapVisitor : public CLxImpl_AbstractVisitor
{
	CLxUser_MeshMap*	m_meshMap;
	std::set<std::string>	m_mapNames;

	virtual LxResult	Evaluate ()
	{
		const char*	mapName;
		if (LXx_OK (m_meshMap->Name (&mapName))) {
			m_mapNames.insert (std::string (mapName));
		}
		return LXe_OK;
	}

    public:
	MeshMapVisitor (CLxUser_MeshMap *MeshMap)
	{
		m_meshMap = MeshMap;
	}

	std::set<std::string>	GetMapNames ()	{ return m_mapNames; }
	void			ClearNames () {m_mapNames.clear ();}
};

	int
CMorphPopup::BuildNameLists ()
{
	CLxUser_DeformerService	 dfm_svc;
	CLxUser_SelectionService sel_svc;
	CLxUser_ChannelRead	 chan_read;
	CLxUser_Scene		 scene;
	CLxUser_Item		 morph_item;
	CLxItemSelectionType	 morph_sel (MORPHDEF_ITEMTYPE);
	std::set <std::string>	 maps_set;
	
	morfNames.clear ();
	morfNames.push_back ("(none)");

	/*
	 *	Loop over the selected morph deformers, and then loop over
	 *	their meshes. For each mesh, evaluate the stack up to this morph
	 *	influence and collect the morph maps.
	 */

	morph_sel.LoopInit ();
	while (morph_sel.LoopNext (morph_item))
	{
		const char		*ident = NULL;
		unsigned		 i = 0, count = 0;
		
		morph_item.Ident (&ident);
		if (!ident)
			ident = LXs_MESHFILTER_TOP;
		
		if (!chan_read.test ())
		{
			morph_item.GetContext (scene);
			scene.GetChannels (chan_read, sel_svc.GetTime ());
		}
		
		dfm_svc.MeshCount (morph_item, &count);
		for (i = 0; i < count; i++)
		{
			CLxUser_Item		 mesh_item, inst_item;
			CLxUser_MeshFilter	 mesh_filter;
			CLxUser_MeshFilterIdent	 meshIdent_filter;
			CLxUser_MeshMap		 meshMap;
			CLxUser_Mesh		 mesh;

			if (!dfm_svc.GetMesh (morph_item, i, mesh_item))
				continue;
			
			if (mesh_item.IsA (meshInstType))
			{
				if (!mesh_item.Source (inst_item))
					continue;
				
				mesh_item.copy (inst_item);
			}
			
			if (chan_read.Object (mesh_item, LXsICHAN_MESH_MESH, mesh_filter))
			{
				if (meshIdent_filter.copy (mesh_filter))
					meshIdent_filter.GetMesh (ident, mesh);
			}
			
			if (mesh.test ())
			{
				mesh.GetMaps (meshMap);

				MeshMapVisitor		 name_visitor (&meshMap);

				meshMap.FilterByType (LXi_VMAP_MORPH);
				meshMap.Enum (&name_visitor);
				meshMap.FilterByType (LXi_VMAP_SPOT);
				meshMap.Enum (&name_visitor);

				std::set <std::string>	 name_set = name_visitor.GetMapNames ();
				
				maps_set.insert (name_set.begin (), name_set.end ());
			}
		}
	}
	
	/*
	 *	Dump the contents of the set into the morfNames vector. The
	 *	list is also sorted, so it's alpha numeric.
	 */
	
	std::set<std::string>::iterator		map_iterator;
	
	for (map_iterator = maps_set.begin (); map_iterator != maps_set.end (); map_iterator++)
	{
		morfNames.push_back (*map_iterator);
	}
	
	std::sort (morfNames.begin ()+1, morfNames.end ());

	return 0;
}


       CLxDynamicUIValue *
CMorphDefCommand::atrui_UIValue (
        unsigned int		 index)
{	
	CMorphPopup *wp = new CMorphPopup;
	wp->BuildNameLists ();
        return wp;
}

/*
 * Execute -- this sets the map name for all selected morph items.
 */
class CExecItemVisitor : public CItemVisitor
{
    public:
	std::string		 mapName;
	unsigned		 nameIdx;

        bool Item (CLxUser_Item &item)
        {
			CLxUser_ChannelWrite	 chan;
			CLxUser_Scene		 scene;

			item.GetContext (scene);
			scene.SetChannels (chan, LXs_ACTIONLAYER_EDIT, 0.0);
			if (LXx_OK (item.ChannelLookup (Cs_MORPH_MAPNAME, &nameIdx)))
			{
				if(mapName=="(none)")
				{
					chan.Set (item, nameIdx, "");
				}
				else
				{
					chan.Set (item, nameIdx, mapName.c_str());
				}
			}
			return false;			
		}
};

        void
CMorphDefCommand::cmd_Execute (
        unsigned int		 flags)
{
        CExecItemVisitor	 vis;

        if (!LXx_OK (attr_GetString (0, vis.mapName))) {
		CLxUser_Message		&msg = basic_Message ();
		msg.SetMsg ("common", 99);
		msg.SetArg (1, "Please choose an morph deformer!");
		return;
	}
        sT->Enumerate (vis);
}



/*
 * Create command. Set the morph to the selected map, if any.
 */
class CCreateCommand
		: public CLxBasicCommand,
		  public CLxVertexMapSelection
{
    public:
	CLxSceneSelection		 scene_sel;

		int
	basic_CmdFlags ()
				LXx_OVERRIDE
	{
	        return LXfCMD_MODEL | LXfCMD_UNDO;
	}

		void
	cmd_Execute (
		unsigned int		 flags)
				LXx_OVERRIDE
	{
		CLxUser_Scene		 scene;
		CLxUser_Item		 item;
		std::string		 name;

		if (!scene_sel.Get (scene))
			throw (LXe_NOTFOUND);

		if (!scene.NewItem (morphDeformType, item))
			throw (LXe_FAILED);

		if (!First (name))
			return;

		CLxUser_ChannelWrite	 write;

		write.setupFrom (scene);
		write.Set (item, Cs_MORPH_MAPNAME, name.c_str ());
		
		// Set the name of the item to the morph map's name.
		item.SetName (name.c_str ());
	}

		bool
	Include (
		LXtID4			 type)
				LXx_OVERRIDE
	{
		return (type == LXi_VMAP_MORPH) || (type == LXi_VMAP_SPOT);
	}
};



/*
 * Setup our command as a server. It has a command interface, an attributes
 * interface for arguments, and an attributesUI interface.
 */
void initialize ()
{
        CLxGenericPolymorph	*srv;

        srv = new CLxPolymorph<CMorphDefCommand>;
        srv->AddInterface (new CLxIfc_Command     <CMorphDefCommand>);
        srv->AddInterface (new CLxIfc_Attributes  <CMorphDefCommand>);
        srv->AddInterface (new CLxIfc_AttributesUI<CMorphDefCommand>);
        lx::AddServer ("morphDeform.mapName", srv);

        srv = new CLxPolymorph<CCreateCommand>;
        srv->AddInterface (new CLxIfc_Command     <CCreateCommand>);
        srv->AddInterface (new CLxIfc_Attributes  <CCreateCommand>);
        srv->AddInterface (new CLxIfc_AttributesUI<CCreateCommand>);
        lx::AddServer ("morphDeform.create", srv);
}


	};	// END namespace
