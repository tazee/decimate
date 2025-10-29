#pragma once

// Obj Loader Class
//
//   Copyright 0000

#include "baseGeomTypes.h"
#include "objLogMessage.h"

#include <lxsdk/lxu_scene.hpp>

#include <map>

/*
 * ----------------------------------------------------------------
 * OBJ Loader Class
 *
 * The loader is a standard scene loader, and also derives from the parser
 * for convenience.
 */
class COBJLoader : public CLxSceneLoader, public COBJParser
{
	OBJLogMessage		 log;
	bool			 foundGroup;
	unsigned		 lineIndex;

	// Variables to prevent calling GetUserInt multiple times in an import, which
	// causes a massive slowdown.
	int			importStatic;
	int			importSeparateMeshes;
	bool			staticPreference;
	bool			separatePreference;

    public:
	COBJLoader () :
		importStatic(0), importSeparateMeshes(0),
		staticPreference(false), separatePreference(false)
	{}
	
	~COBJLoader () {}

	void			 Cleanup ();

	virtual CLxFileParser *	 sl_Parser    ()	{ return this; }

	virtual bool		 sl_Recognize  ();
	virtual bool		 sl_HasOptions ();
	virtual void		 sl_SpawnOpt   (void **);
	virtual bool		 sl_ParseInit  ();
	virtual bool		 sl_ParseDone  ();
	virtual bool		 sl_Parse      (LxResult *);

	static LXtTagInfoDesc	 descInfo[];
	string			 matr_name, part_name, obj_name;

	bool			 ImportStaticMesh ();
	bool			 isImportingTriSurf;
	bool			 ImportGroups();
	UnitsType		 ImportUnits ();

	/*
	 * Mesh item loading.
	 */
	void			 MakeVrts ();
	void			 NeedMesh ();
	void			 ParseFace ();
	void			 ParseLine ();

	unsigned		 MeshPointIndexForVertex(unsigned index);

	double			 unitScale;
	unsigned		 uv_map, norm_map, vertCol_map;
	bool			 no_mesh, no_uv, no_norm, no_vertCol;

	vector<CVertex>		 vertex_list;
	map<unsigned, unsigned>	 vertex_index_map;
	vector<C4Vector>	 vertex_color_list;
	vector<CTexture>	 texcoord_list;
	vector<CNormal>		 normal_list;
	int			 vrt_count;
	int			 vrt_base;
	bool			 vrt_hasCol;

	vector<unsigned>	 pol_vrt, pol_uv, pol_norm;
	set<string>		 material_lib_paths;

	/*
	 * TriSurf item loading.
	 */
	void			 WriteSurf ();
	unsigned		 PntIndex  (const CPolyElt &);

	CVectorList<C3Vector>	 vrt_list, norm_list;
	CVectorList<C2Vector>	 uv_list;
	CVectorList<C4Vector>	 vrt_color_list;
	bool			 has_uv, has_norm, has_vrtColor;

	vector<CPolyElt>	 pnt_list;
	map<CPolyElt,unsigned>	 pnt_map;
	vector<unsigned>	 tri_list;

	vector<CPolyElt>	 cur_pol;

	CLxUser_TriangleGroup	 grp_item;
	unsigned		 grp_ntri;
};
