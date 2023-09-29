#ifdef MAKE_TRACE
#include "trace_main.h"

#include "trace_nodes.h"

#include "geo_types.h"
#include "global_types.h"
#include "global_value.h"
#include "intersections.h"
#include "reader_bin.h"
#include "reader_txt.h"
#include "writer_bin.h"

#include <chrono>
namespace tick = std::chrono;

int trace::RunTracesModule() {

  int np = get_mpi_np();
  int myid = get_mpi_id();

#ifdef ONLY_GEO_DATA
  return PreBuild(glb_files);
#endif

  //-----------файлы с данными сетки. Построены здесь на метки USE_VTK-----------------------
  const std::string name_file_normals = glb_files.base_address + F_NORMALS;
  const std::string name_file_cells = glb_files.base_address + F_TRACE_GRID;
  const std::string name_file_vertex = glb_files.base_address + F_TRACE_VERTEX;
  const std::string name_file_neigh = glb_files.base_address + F_NEIGHBOR;
  //-------------------читающиеся файлы, построенные в build_graph---------------------------
  const std::string name_file_id_defining_faces = glb_files.base_address + "pairs.bin";
  const std::string name_file_x_defining_faces = glb_files.base_address + "x_defining_faces.bin";
  const std::string name_file_size = glb_files.base_address + "Size.txt"; // на ДОЗАПИСЬ
  //--------------------------------создающиеся файлы----------------------------------------
  const std::string name_file_state_face = glb_files.illum_geo_address + F_STATE_FACE;
  const std::string name_file_x = glb_files.illum_geo_address + F_X;
  const std::string name_file_x0_loc = glb_files.illum_geo_address + F_X0_LOC;
  const std::string name_file_res_bound = glb_files.illum_geo_address + F_RES;

  // make
  std::vector<Face> grid;
  std::vector<Matrix4> vertexs;
  std::vector<Normals> normals;
  std::vector<IntId> neighbours;

  grid_directions_t grid_direction;
  std::vector<std::vector<IntId>> sorted_graph;

  auto start_clock = tick::steady_clock::now();

  uint32_t err = 0;
  err |= files_sys::bin::ReadSimple(glb_files.name_file_neigh, neighbours);
  err |= files_sys::bin::ReadSimple(name_file_cells, grid);
  err |= files_sys::bin::ReadSimple(name_file_vertex, vertexs);
  err |= files_sys::bin::ReadNormals(name_file_normals, normals);
  err |= files_sys::txt::ReadSphereDirectionСartesian(glb_files.name_file_sphere_direction, grid_direction);

  sorted_graph.resize(grid_direction.size);
  for (int i = myid; i < grid_direction.size; i += np) {
    err |= files_sys::bin::ReadSimple(glb_files.graph_address + std::to_string(i) + ".bin", sorted_graph[i]);
  }
  if (err != 0) {
    RETURN_ERR("error during reading\n");
  }

  WRITE_LOG("Reading time trace prebuild %lf\n", (double)tick::duration_cast<tick::milliseconds>(tick::steady_clock::now() - start_clock).count() / 1000.);

  {
    /*	int n;
            std::ifstream ifile(name_file_size);
            if (!ifile.is_open()) RETURN("Error read files size\n")
            ifile >> n;
            ifile.close();

            FILE* f;
            f = fopen(name_file_id_defining_faces.c_str(), "rb");
            if (!f) RETURN("name_file_id_defining_faces not open\n")
                    id_try_surface.resize(n);
            fread_unlocked(id_try_surface.data(), sizeof(int), n, f);
            fclose(f);

            f = fopen(name_file_x_defining_faces.c_str(), "rb");
            if (!f) RETURN("name_file_x_defining_faces not open\n")
                    x_try_surface.resize(n);
            fread_unlocked(x_try_surface.data(), sizeof(Vector3), n, f);
            fclose(f);*/
  }

  const int count_cells = vertexs.size();
  std::vector<BasePointTetra> vec_x(count_cells);
  if (GetInterpolationNodes(vertexs, vec_x)) {
    D_LD;
  }

  if (files_sys::bin::WriteSimple(name_file_x, vec_x)) {
    RETURN_ERR("Error writing %s\n", name_file_x.c_str());
  }

  register int num_cell;
  Vector3 direction;

  // массивы записи в файл:
  std::vector<bits_flag_t> face_states(count_cells, 0); //битовое поле: 0=> выходящая грань,  1=> входящая
  std::vector<cell_local> vec_x0;
  std::vector<Type> vec_res_bound;

  /*---------------------------------- далее FOR по направлениям----------------------------------*/
  int loc_num_dir = 0;
  for (int num_direction = myid; num_direction < grid_direction.size; num_direction += np, loc_num_dir++) {
    direction = grid_direction.directions[num_direction].dir;

    vec_x0.clear();
    vec_x0.reserve(CELL_SIZE * count_cells);

    /*---------------------------------- далее FOR по ячейкам----------------------------------*/
    for (int h = 0; h < count_cells; ++h) {
      num_cell = sorted_graph[loc_num_dir][h];

      bits_flag_t face_state = 0;
      intersection::FindInAndOutFaces(direction, normals[num_cell], face_state);
      face_states[num_cell] = face_state;

      for (ShortId num_out_face = 0; num_out_face < CELL_SIZE; ++num_out_face) {
        if (!CHECK_BIT(face_state, num_out_face)) // выходящие грани
        {
          GetLocNodes(num_cell, num_out_face, grid, vertexs[num_cell],
                      face_state, direction, normals[num_cell], neighbours,
                      vec_x[num_cell], vec_res_bound, vec_x0);
        }
      }
    }
    /*---------------------------------- конец FOR по ячейкам----------------------------------*/

    if (files_sys::bin::WriteSimple(name_file_state_face + std::to_string(num_direction) + ".bin", face_states))
      RETURN_ERR("Error face_states");
    if (files_sys::bin::WriteSimple(name_file_x0_loc + std::to_string(num_direction) + ".bin", vec_x0))
      RETURN_ERR("Error vec_x0");

    WRITE_LOG("End trace direction number # $d\n", num_direction);
  }
  /*---------------------------------- конец FOR по направлениям----------------------------------*/

  if (files_sys::bin::WriteSimple(name_file_res_bound, vec_res_bound))
    RETURN_ERR("Error vec_res_bound");

  WRITE_LOG("Full trace time: %lf\n", (double)tick::duration_cast<tick::milliseconds>(tick::steady_clock::now() - start_clock).count() / 1000.);
  return e_completion_success;
}

#endif ///! MAKE_TRACE