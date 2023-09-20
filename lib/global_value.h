#ifndef GLOBAL_VALUE
#define GLOBAL_VALUE

#include <string>
#include "prj_config.h"
#include "global_def.h"

#define PI 3.14159265358979323846

const double gamma1 = 5. / 3; // 5. / 3;  // показатель адиабаты
const double gamma_g = gamma1 / (gamma1 - 1);

const double eps = 1e-10;

#ifdef Sphere
const Vector3 center_point(0, 0, 0);
const Type inner_radius = 0.51; // радиус внутренней сферы (с запасом)
#else 
const Vector3 center_point(10, 0, 0);
const Type inner_radius = 0.12; // радиус внутренней сферы (с запасом)
#endif

//кг
#define EarthMass (5.9722*1e25)
#define SunMass	  (1.9891*1e31)

// м
#define DistSun (149.6 * 10e9)
#define DistMoon 400000000.

//м/c
#define C_LIGHT 299792458.0
#define C_LIGHT_INV (1.0/(C_LIGHT))

#define DIST  1e6;//(1*1e10) //DistMoon
#define MASS (1 * 1e21)//EarthMass
#define VELOCITY (3 * 1e8)//C_LIGHT

#define TIME (DIST/VELOCITY)
//#define DENSITY (MASS/(DIST*DIST*DIST))
//#define PRESSURE (MASS/(DIST*TIME*TIME))
//#define RADIATION (MASS/(TIME*TIME*TIME))
#define DENSITY (3.34*10e-14)
#define PRESSURE (DENSITY*VELOCITY*VELOCITY)
#define RADIATION (DENSITY*VELOCITY*VELOCITY*VELOCITY)

const Type R_gas = 8.314;  //газовая постоянная [ Дж/(моль*К)]
const Type c_light = C_LIGHT; // 3 * 1e8;  //[м/c]
const Type h_plank = 6.62 * 1e-34;  // [кг * м^2 /с]
const Type k_boltzmann = 1.38 * 1e-23; // [Дж/K] = [ кг*м^2/(с^2*T)]
const Type sigma_thomson = 6.652 * 1e-29; //сечение томсоновского рассеяния [m^2]
const Type m_hydrogen = 1.6735575 * 1e-27; //масса водорода[кг]

//--------------------------Файлы управления-----------------------------------//
#define F_SET "settings_file.txt"
#define F_HLLC_SET "hllc_settings.txt"
#define F_INIT_HLLC "no.bin"
#define F_MPI_CONF "mpi_conf"

#define F_LOG "File_Logs.txt"

//--------------------------Файлы расчётных данных(отдельные файлы по направлениям)-----------------------------------//
#define F_GRAPH "graph"
#define F_CENTERS "centers.bin"
#define F_NORMALS "normals.bin"
#define F_SQUARES "squares.bin"
#define F_VOLUME "volume.bin"

#define F_CENTER_FACE "center_face.bin"

#define F_X0_LOC  "LocX0"
#define F_X  "X.bin"
#define F_STATE_FACE "state_face" 

//--------------------------Файлы трассировки сквозь внутреннюю границу----------------------------------------//
#define F_DIST_TRY  "dist_defining_faces"
#define F_ID_TRY "id_defining_faces"
#define F_RES  "ResBound"
#define F_NEIB "pairs.bin"

//--------------------------Файлы геометрии--------------------------------------------------------------------//
#define F_GEO_FACES "geo_faces.bin"
#define F_GEO_CELLS "geo_cells.bin"

//--------------------------Файлы решение--------------------------------------------------------------------//
#define F_SOLVE "Solve"  // задается в настройках

#define F_DENSITY "density.bin"
#define F_PRESSURE "pressure.bin"
#define F_VELOCITY "velocity.bin"

#define F_ILLUM "Illum.bin"
#define F_ENERGY "energy.bin"
#define F_STREAM "stream.bin"
#define F_IMPULS "impuls.bin"
#define F_DIVSTREAM "divstream.bin"
#define F_DIVIMPULS "divimpuls.bin"

#define F_ABSORPCOEF "AbsorpCoef.bin"
#define F_RADLOOSERATE "radEnLooseRate.bin"

#define STRUCT_FILES_T \
/*--------------------------Файлы сеток----------------------------------------*/ \
    X(std::string, name_file_vtk) \
    X(std::string, name_file_sphere_direction) \
/*--------------------------Глобальные адреса----------------------------------------*/ \
    X(std::string, base_address)\
	X(std::string, illum_geo_address)\
	X(std::string, graph_address)\
	X(std::string, solve_address)\
/*--------------------------Настроичные файлы----------------------------------------*/ \
	X(std::string, name_file_hllc_set)\
	X(std::string, hllc_init_value) \
	X(std::string, solve_configuration)

struct global_files_t
{
#define X(type, name) type name;
	STRUCT_FILES_T;
#undef X

	std::string name_file_settings;

	//--------------------------Файлы расчётных данных(отдельные файлы по направлениям)-----------------------------------//
	std::string name_file_state_face;
	std::string name_file_x0_loc;
	std::string name_file_x;

	//--------------------------Файлы трассировки сквозь внутреннюю границу----------------------------------------//
	std::string name_file_dist_try;
	std::string name_file_id_try;
	std::string name_file_res;
	std::string name_file_neib;

#if 0
	//--------------------------Файлы сдвигов по направлениям в расчётных файлах-----------------------------------//
	const std::string name_file_shift_out = glb_files.base_address + "ShiftOut";
	const std::string name_file_shift_res = glb_files.base_address + "ShiftRes";
	const std::string name_file_shift_x0 = glb_files.base_address + "ShiftX0";
	const std::string name_file_shift_try = glb_files.base_address + "ShiftTry";
#endif

	//--------------------------Файлы геометрии--------------------------------------------------------------------//
	std::string name_file_geometry_faces;
	std::string name_file_geometry_cells;


	global_files_t(file_name_t BASE_ADDRESS = "", file_name_t address_illum_geo_file = "")
	{
		base_address = BASE_ADDRESS;
		illum_geo_address = address_illum_geo_file;
		graph_address = F_GRAPH;
		solve_address = F_SOLVE;
		hllc_init_value = F_INIT_HLLC;
		Build();
	}

	void Build()
	{		
		//name_file_hllc_set = base_address + F_HLLC_SET;

		//--------------------------Файлы расчётных данных(отдельные файлы по направлениям)-----------------------------------//
		name_file_state_face = illum_geo_address + F_STATE_FACE;
		name_file_x0_loc = illum_geo_address + F_X0_LOC;
		name_file_x = illum_geo_address + F_X;

		//--------------------------Файлы трассировки сквозь внутреннюю границу----------------------------------------//
		name_file_dist_try = base_address + F_DIST_TRY;
		name_file_id_try = base_address + F_ID_TRY;
		name_file_res = illum_geo_address + F_RES;
		name_file_neib = base_address + F_NEIB;

#if 0
		//--------------------------Файлы сдвигов по направлениям в расчётных файлах-----------------------------------//
		const std::string name_file_shift_out = glb_files.base_address + "ShiftOut";
		const std::string name_file_shift_res = glb_files.base_address + "ShiftRes";
		const std::string name_file_shift_x0 = glb_files.base_address + "ShiftX0";
		const std::string name_file_shift_try = glb_files.base_address + "ShiftTry";
#endif

		//--------------------------Файлы геометрии--------------------------------------------------------------------//
		name_file_geometry_faces = base_address + F_GEO_FACES;
		name_file_geometry_cells = base_address + F_GEO_CELLS;
	}

#ifdef DEBUG
	void print()
	{
		std::string* str = (std::string*)this;
		while (str < (std::string*)this + sizeof(global_files_t) / sizeof(std::string))
		{
			std::cout << *str++ << '\n';
		}
	}
#endif
};

#include "json/json.hpp"
static void to_json(nlohmann::json& j, const global_files_t& p)
{
#define X(type, name) {#name, p.name},
	j = nlohmann::json{ STRUCT_FILES_T };
#undef X	
}

static void from_json(const nlohmann::json& j, global_files_t& p) {
#define X(type, name) j.at(#name).get_to(p.name);
	STRUCT_FILES_T
#undef X			
}
#undef STRUCT_FILES_T

extern global_files_t glb_files;

#endif //GLOBAL_VALUE
