#if defined ILLUM && defined SOLVERS // && !defined USE_MPI
#include "illum_part.h"

#include "dbgdef.h"
#include "global_types.h"
#include "global_value.h"
#include "illum_utils.h"

#include <omp.h>

static Type CalculateIllumeOnInnerFace(const int num_in_face, const std::vector<face_t> &faces, elem_t *cell, Vector3 &inter_coef) {
  const int id_face_ = cell->geo.id_faces[num_in_face]; //номер грани
  const int neigh_id = faces[id_face_].geo.id_r;        // признак ГУ + связь с глобальной нумерацией

  if (neigh_id < 0) {
    Type I_x0 = illum::BoundaryConditions(neigh_id);
    inter_coef = Vector3(I_x0, I_x0, I_x0);
    return I_x0;
  }
  // Vector3 coef = grid[num_cell].nodes_value[num_in_face];
  Vector3 coef = inter_coef; // cell->illum_val.coef_inter[num_in_face];

  /// \note сейчас храним значения а не коэффициента интерполяции

  // Vector2	x0_local = X0[ShiftX0 + posX0++]; // grid[num_cell].x0_loc[num_in_face_dir];
  // I_x0 = x0_local[0] * coef[0] + x0_local[1] * coef[1] + coef[2];

  Type I_x0 = (coef[0] + coef[1] + coef[2]) / 3.;

  if (I_x0 < 0) {
    D_L;
    return 0;
  }
  return I_x0;
}

static Type GetS(const int N_cell, const int num_cell, const Vector3 &direction, const Type *illum_old,
                 const grid_directions_t &grid_direction) {
  // num_cell equals x
  auto Gamma{[](const Vector3 &direction, const Vector3 &direction2) {
    Type dot = direction.dot(direction2);
    return (3. * (1 + dot * dot)) / 4;
  }};

  Vector3 cur_direction;
  Type S = 0;
  const int N_dir = grid_direction.size;
  // const int N_cell = illum_old.size() / N_dir;

  for (int num_direction = 0; num_direction < N_dir; num_direction++) {
    Type I = 0;
    for (int i = 0; i < CELL_SIZE; i++) {
      I += illum_old[num_direction * N_cell + num_cell + i];
    }
    I /= CELL_SIZE;

    S += Gamma(grid_direction.directions[num_direction].dir, direction) * I * grid_direction.directions[num_direction].area;
  }
  return S / grid_direction.full_area; // было *4PI, но из-за нормировки Gamma разделили на 4PI
}

static int CalculateIntCPU(const int num_cells, const grid_directions_t &grid_direction, grid_t grid) {

#pragma omp parallel default(none) shared(num_cells, illum, grid_direction, int_scattering)
  {
    Vector3 direction;
    const int num_directions = grid_direction.size;
#pragma omp for
    for (int num_direction = 0; num_direction < num_directions; ++num_direction) {
      direction = grid_direction.directions[num_direction].dir;
      for (int cell = 0; cell < num_cells; cell++) {
        grid.scattering[num_direction * num_cells + cell] = GetS(grid.size, CELL_SIZE * cell, direction, grid.Illum, grid_direction);
      }
    }
  }
  return 0;
}

static Type GetCurIllum(const Vector3 x, const Type s, const Type I_0, const Type int_scattering, const elem_t &cell) {
  switch (_solve_mode.class_vtk) {
  case 0: // без интеграла рассеивания  (излучающий шар)
  {
    Type Q = 0;
    Type alpha = 2;
    Type betta = 1;
    Type S = int_scattering; // 0;

    if ((x - Vector3(1, 0, 0)).norm() > 0.09) {
      Q = 0;
      alpha = 0.5;
      betta = 0.5;
    }

    Type k = alpha + betta;

    Type I;
    if (k > 1e-10)
      I = (exp(-k * s) * (I_0 * k + (exp(k * s) - 1) * (Q + S * betta))) / k;
    else
      I = (1 - s * k) * (I_0 + s * (Q + S * betta));

    if (I < 0) {
      return 0;
    }

    return I;

    //-------------------------------------------
    /*Type Ie = 10;
    Type k = 10;
    if ((x - Vector3(1, 0, 0)).norm() > 0.09) { Ie = 0; k = 1; }

    Type I;

    if (k > 1e-10)
            I = Ie * (1 - exp(-s * k)) + I_node_prev * exp(-s * k);
    else
            I = I_node_prev * (1 - s * k) + Ie * s * k;

    if (I < 0)
            I = 0;
    return I;*/
  }

  case 1: // test task
  {

    // U_Full[cur_id][0] -> density;
    Type S = int_scattering;
    Type Q = cell.illum_val.rad_en_loose_rate; // Q=alpha*Ie
    Type alpha = cell.illum_val.absorp_coef;

    Type betta = alpha / 2; // просто из головы
    Type k = alpha + betta;

    Type I;
    if (k > 1e-10)
      I = (exp(-k * s) * (I_0 * k + (exp(k * s) - 1) * (Q + S * betta))) / k;
    else
      I = (1 - s * k) * (I_0 + s * (Q + S * betta));

    if (I < 0) {
      return 0;
    }

    return I;
  }

  case 2: {
    const Type ss = s * 388189 * 1e5; // числа --- переход к размерным параметрам

    Type Q = cell.illum_val.rad_en_loose_rate; // Q=alpha*Ie
    Type alpha = cell.phys_val.d * cell.illum_val.absorp_coef;

    Type I;
    if (alpha > 1e-15)
      I = I_0 * exp(-alpha * ss) + Q * (1 - exp(-alpha * ss)) / alpha;
    else
      I = I_0 * (1 - alpha * ss) + Q * ss;

    if (I < 0) {
      return 0;
    }

    return I;
  }

  case 3: {

    const Type d = cell.phys_val.d;
    Type S = int_scattering;

    // alpha = d*XXX...
    // betta = d*sigma*XXX...
    Type alpha = 0;         // absorp_coef[cur_id];
    Type betta = alpha / 2; // просто из головы
    Type Q = 0;             //  rad_en_loose_rate[cur_id];  //Q=alpha*Ie

    Type k = alpha + betta;

    Type I;
    if (k > 1e-10)
      I = (exp(-k * s) * (I_0 * k + (exp(k * s) - 1) * (Q + S * betta))) / k;
    else
      I = (1 - s * k) * (I_0 + s * (Q + S * betta));

    if (I < 0) {
      return 0;
    }

    return I;
  }

  case 10: // для конуса (считаем, что излучающая часть не изменяется в зависимости от газораспределения)
  {
    Type Q = 0;
    Type alpha = 0.5;
    Type betta = 0.5;
    Type S = int_scattering;

    // if (x[0] < 0.06) // излучающий слой
    //{
    //	Q = 10; alpha = 1;  betta = 2;
    // }

    Type k = alpha + betta;

    Type I;
    if (k > 1e-10)
      I = (exp(-k * s) * (I_0 * k + (exp(k * s) - 1) * (Q + S * betta))) / k;
    else
      I = (1 - s * k) * (I_0 + s * (Q + S * betta));

    if (I < 0) {
      return 0;
    }

    return I;
  }

  case 11: // HLLC + Illum для конуса
  {

    Type S = int_scattering * kRadiation;

    Type d = cell.phys_val.d * kDensity;
    Type v = cell.phys_val.v.norm() * kVelocity;
    Type p = cell.phys_val.p * kPressure;

    Type T = p / (d * R_gas);

    if (x[0] < 0.05) {
      // d = 0.1;
      // p = 0.01;
      // T = p / (d * R);
    }

    Type Ie = 2 * pow(k_boltzmann * PI * T, 4) / (15 * h_plank * h_plank * h_plank * kC_Light * kC_Light);
    Type betta = sigma_thomson * d / m_hydrogen;
    Type alpha = betta; // alpha = ???

    Type Q = alpha * Ie;

    Type k = alpha + betta;

    Type ss = s * kDist;
    Type I0 = I_0 * kRadiation;

    Type I;
    if (k > 1e-10)
      I = (exp(-k * ss) * (I0 * k + (exp(k * ss) - 1) * (Q + S * betta))) / k;
    else
      I = (1 - ss * k) * (I0 + ss * (Q + S * betta));

    if (I < 0) {
      return 0;
    }

    return I / kRadiation;
  }

  default:
    D_LD;
  }
}

// static Type ReCalcIllum(const int num_dir, std::vector<elem_t>& cells, std::vector<Type>& Illum)
static Type ReCalcIllum(const int num_dir, std::vector<elem_t> &cells, const std::vector<Vector3> &inter_coef, Type *Illum) {
  Type norm = -1;
  //#pragma omp parallel default(none) shared(num_dir, cells, Illum, norm)
  {
    Type norm_loc = -1;
    const int size = cells.size();
    const int shift_dir = num_dir * size;

    //#pragma omp for
    for (int num_cell = 0; num_cell < size; num_cell++) {
      for (int i = 0; i < CELL_SIZE; i++) {
        Vector3 Il = inter_coef[num_cell * CELL_SIZE + i]; // cells[num_cell].illum_val.coef_inter[i];
        const Type curI = (Il[0] + Il[1] + Il[2]) / 3;
        const int id = CELL_SIZE * (shift_dir + num_cell) + i;
        {
          Type buf_norm = fabs(Illum[id] - curI) / curI;
          if (buf_norm > norm_loc)
            norm_loc = buf_norm;
        }
        Illum[id] = curI;

        cells[num_cell].illum_val.illum[num_dir * CELL_SIZE + i] = curI; //на каждой грани по направлениям
      }
      // cells[num_cell].illum_val.illum[num_dir] /= CELL_SIZE; //пересчёт по направлению для расчёта энергий и т.д.
    }

    //#pragma omp critical
    {
      if (norm_loc > norm) {
        norm = norm_loc;
      }
    }
  } // parallel

  return norm;
}

static int GetIntScattering(const int count_cells, const grid_directions_t &grid_direction, grid_t &grid) {
#ifdef USE_CUDA
  if (solve_mode.use_cuda) {
    if (CalculateIntScattering(32, count_cells, grid_direction.size, Illum, int_scattering)) // если была ошибка пересчитать на CPU
    {
      CalculateIntCPU(count_cells, Illum, grid_direction, int_scattering);
      ClearDevice(solve_mode.cuda_mod);
      solve_mode.use_cuda = false;
    }
  } else
#endif
  {
    CalculateIntCPU(count_cells, grid_direction, grid);
  }

  return 0;
}

int illum::CalculateIllum(const grid_directions_t &grid_direction, const std::vector<std::vector<State>> &face_states, const std::vector<int> &pairs,
                          const std::vector<std::vector<cell_local>> &vec_x0, std::vector<BasePointTetra> &vec_x,
                          const std::vector<std::vector<int>> &sorted_id_cell, grid_t &grid) {
  const int count_directions = grid_direction.size;
  const int count_cells = grid.size;

  // вроде не обязательно. ПРОВЕРИТЬ
  // Illum.assign(4 * count_cells * count_directions, 0);
  // int_scattering.assign(count_cells * count_directions, 0);

  int count = 0;
  Type norm = 0;

  const int number_of_trhed = 1;
  omp_set_num_threads(number_of_trhed);

  static std::vector<std::vector<Vector3>> inter_coef_all(number_of_trhed);
  for (size_t i = 0; i < number_of_trhed; i++) {
    inter_coef_all[i].resize(count_cells * CELL_SIZE);
  }

  do {
    Type _clock = -omp_get_wtime();

    norm = -1;
    /*---------------------------------- далее FOR по направлениям----------------------------------*/

#pragma omp parallel default(none) shared(sorted_id_cell, pairs, face_states, vec_x0, vec_x, grid, int_scattering, Illum, norm, inter_coef_all)
    {
      Type loc_norm = -1;
      // std::vector<Vector3> inter_coef(count_cells * CELL_SIZE);
      const int num = omp_get_thread_num();
      std::vector<Vector3> *inter_coef = &inter_coef_all[num];
#pragma omp for
      for (register int num_direction = 0; num_direction < count_directions; ++num_direction) {
        /*---------------------------------- далее FOR по ячейкам----------------------------------*/
        int posX0 = 0;
        Vector3 I;

        for (int h = 0; h < count_cells; ++h) {
          const int num_cell = sorted_id_cell[num_direction][h];

          elem_t *cell = &grid.cells[num_cell];

          // sumI = 0;
          for (ShortId num_out_face = 0; num_out_face < CELL_SIZE; ++num_out_face) {
            if (CHECK_BIT(face_states[num_direction][num_cell], num_out_face)) {
              if (pairs[num_cell * CELL_SIZE + num_out_face] < 0) {
                Type I0 = illum::BoundaryConditions(pairs[num_cell * CELL_SIZE + num_out_face]);
                (*inter_coef)[num_cell * CELL_SIZE + num_out_face] = Vector3(I0, I0, I0);
              }
              continue;
            }

            // GetNodes
            for (int num_node = 0; num_node < 3; ++num_node) {
              Vector3 x = vec_x[num_cell].x[num_out_face][num_node];

              cell_local x0 = vec_x0[num_direction][posX0++];

              ShortId num_in_face = x0.in_face_id;
              Type s = x0.s;
              Vector2 X0 = x0.x0;
              Type I_x0 = CalculateIllumeOnInnerFace(num_in_face, grid.faces, cell, (*inter_coef)[num_cell * CELL_SIZE + num_in_face]);

              I[num_node] = GetCurIllum(x, s, I_x0, grid.scattering[num_direction * count_cells + num_cell], *cell);

            } // num_node

            // если хранить не знгачения у коэфф. интерполяции
            {
              // Vector3 coef;
              // if (num_out_face == 3)
              //	coef = inclined_face_inverse * I;// GetInterpolationCoefInverse(inclined_face_inverse, I);
              // else
              //	coef = straight_face_inverse * I;// GetInterpolationCoefInverse(straight_face_inverse, I);
            }
#if 0 // в id_r лежит ячейка. а не грань
						const int id_face_ = cell->geo.id_faces[num_out_face]; //номер грани
						const int id_face = grid.faces[id_face_].geo.id_r;  // признак ГУ + связь с глобальной нумерацией
						cell->illum_val.coef_inter[num_out_face] = I;  //coef					
						if (id_face >= 0)
						{
							grid.cells[id_face / CELL_SIZE].illum_val.coef_inter[id_face % CELL_SIZE] = I;
						}
#else

            // cell->illum_val.coef_inter[num_out_face] = I;  //coef
            (*inter_coef)[num_cell * CELL_SIZE + num_out_face] = I;
            const int id_face = pairs[num_cell * CELL_SIZE + num_out_face];

            if (id_face >= 0) {
              // grid.cells[id_face / CELL_SIZE].illum_val.coef_inter[id_face % CELL_SIZE] = I;
              (*inter_coef)[id_face] = I;
            }
#endif

          } // num_out_face
        }

        /*---------------------------------- конец FOR по ячейкам----------------------------------*/
        loc_norm = ReCalcIllum(num_direction, grid.cells, *inter_coef, grid.Illum);
      }
      /*---------------------------------- конец FOR по направлениям----------------------------------*/

      if (loc_norm > norm) {
#pragma omp critical
        {
          if (loc_norm > norm) {
            norm = loc_norm;
          }
        }
      }
    }

    if (_solve_mode.max_number_of_iter > 1) // пропуск первой итерации
    {
      GetIntScattering(count_cells, grid_direction, grid);
    }

    _clock += omp_get_wtime();

    WRITE_LOG("End iter number#: %d, time= %lf, norm=%lf\n", count, _clock, norm);
    count++;
  } while (norm > _solve_mode.accuracy && count < _solve_mode.max_number_of_iter);

  return 0;
}

//-----------------------------------------------------//

#define GET_FACE_TO_CELL(val, data, init)       \
  {                                             \
    val = init;                                 \
    for (int pos = 0; pos < CELL_SIZE; pos++) { \
      val += data[pos];                         \
    }                                           \
    val /= CELL_SIZE;                           \
  }

static Type IntegarteDirection(const std::vector<Type> &Illum, const grid_directions_t &grid_direction) {
#ifdef SORT_ILLUM
  std::vector<Type> I(grid_direction.size);
  int i = 0;
  Type illum_cell;
  for (auto &dir : grid_direction.directions) {
    GET_FACE_TO_CELL(illum_cell, (&Illum[i * CELL_SIZE]), 0);
    I[i / CELL_SIZE] += illum_cell * dir.area;
    i += CELL_SIZE;
  }

  auto cmp{[](const Type left, const Type right) { return left < right; }};
  std::sort(I.begin(), I.end(), cmp);

  Type res = 0;
  for (size_t i = 0; i < grid_direction.size; i++) {
    res += I[i];
  }

#else

  Type res = 0;
  Type illum_cell;
  int i = 0;
  for (auto &dir : grid_direction.directions) {
    GET_FACE_TO_CELL(illum_cell, (&Illum[i * CELL_SIZE]), 0);
    res += illum_cell * dir.area;
    i++; // = CELL_SIZE;
  }
#endif

  return res / grid_direction.full_area;
}
static int MakeEnergy(const grid_directions_t &grid_direction, std::vector<elem_t> &cells) {

  // for (auto &el : cells)
#pragma omp parallel default(none) shared(grid_direction, cells)
  {
#pragma omp for
    for (int i = 0; i < cells.size(); i++) {
      elem_t &el = cells[i];
      el.illum_val.energy = IntegarteDirection(el.illum_val.illum, grid_direction);
    }
  }
  return 0;
}

static int IntegarteDirection3(const std::vector<Type> &Illum, const grid_directions_t &grid_direction, Vector3 *stream_face) {
  int i = 0;
  for (int f = 0; f < CELL_SIZE; f++) {
    stream_face[f] = Vector3::Zero();
  }

  for (auto &dir : grid_direction.directions) {
    for (int f = 0; f < CELL_SIZE; f++) {
      stream_face[f] += Illum[i++] * dir.area * dir.dir;
    }
  }

  for (int f = 0; f < CELL_SIZE; f++) {
    stream_face[f] /= grid_direction.full_area;
  }

  return 0;
}
static int MakeStream(const grid_directions_t &grid_direction, grid_t &grid) {
#pragma omp parallel default(none) shared(grid_direction, grid)
  {
#pragma omp for
    for (int i = 0; i < grid.cells.size(); i++) {
      elem_t &el = grid.cells[i];

      Vector3 Stream[CELL_SIZE];
      IntegarteDirection3(el.illum_val.illum, grid_direction, Stream);

      GET_FACE_TO_CELL(el.illum_val.stream, Stream, Vector3::Zero());

      el.illum_val.div_stream = 0;
      for (int j = 0; j < CELL_SIZE; j++) {
        geo_face_t *geo_f = &grid.faces[el.geo.id_faces[j]].geo;
        if (el.geo.sign_n[j]) {
          el.illum_val.div_stream += Stream[j].dot(geo_f->n) * geo_f->S;
        } else {
          el.illum_val.div_stream -= Stream[j].dot(geo_f->n) * geo_f->S;
        }
      }
      el.illum_val.div_stream /= el.geo.V;
    }
  }
  return 0;
}

static int IntegarteDirection9(const std::vector<Type> &Illum, const grid_directions_t &grid_direction, Matrix3 *impuls_face) {

  int i = 0;
  for (int f = 0; f < CELL_SIZE; f++) {
    impuls_face[f] = Matrix3::Zero();
  }

  for (auto &dir : grid_direction.directions) {
    for (int f = 0; f < CELL_SIZE; f++) {
      for (size_t h = 0; h < 3; h++)
        for (size_t k = 0; k < 3; k++) {
          impuls_face[f](h, k) += dir.dir[h] * dir.dir[k] * (Illum[i] * dir.area);
        }
      i++;
    }
  }

  for (int f = 0; f < CELL_SIZE; f++) {
    impuls_face[f] /= grid_direction.full_area;
  }

  return 0;
}
static int MakeDivImpuls(const grid_directions_t &grid_direction, grid_t &grid) {
#pragma omp parallel default(none) shared(grid_direction, grid)
  {
#pragma omp for
    for (int i = 0; i < grid.cells.size(); i++) {
      elem_t &el = grid.cells[i];

      Matrix3 Impuls[CELL_SIZE];
      IntegarteDirection9(el.illum_val.illum, grid_direction, Impuls);

      GET_FACE_TO_CELL(el.illum_val.impuls, Impuls, Matrix3::Zero());

      el.illum_val.div_impuls = Vector3::Zero();
      for (int j = 0; j < CELL_SIZE; j++) {
        geo_face_t *geo_f = &grid.faces[el.geo.id_faces[j]].geo;
        if (el.geo.sign_n[j]) {
          el.illum_val.div_impuls += Impuls[j] * (geo_f->n) * geo_f->S;
        } else {
          el.illum_val.div_impuls += Impuls[j] * (-geo_f->n) * geo_f->S;
        }
      }
      el.illum_val.div_impuls /= el.geo.V;
    }
  }
  return 0;
}

int illum::CalculateIllumParam(const grid_directions_t &grid_direction, grid_t &grid) {
  MakeEnergy(grid_direction, grid.cells);
  MakeStream(grid_direction, grid);
  MakeDivImpuls(grid_direction, grid);

  return 0;
}

int TestDivStream(const std::vector<Vector3> &centers_face, grid_t &grid) {
#pragma omp parallel default(none) shared(centers_face, grid)
  {
#pragma omp for
    for (int i = 0; i < grid.cells.size(); i++) {
      elem_t &el = grid.cells[i];

      Vector3 Stream[CELL_SIZE];
      for (int j = 0; j < CELL_SIZE; j++) {
        Vector3 x = centers_face[i * CELL_SIZE + j];
        Stream[j] = Vector3(3 * x[0], 0, 0);
      }

      GET_FACE_TO_CELL(el.illum_val.stream, Stream, Vector3::Zero());

      el.illum_val.div_stream = 0;
      for (int j = 0; j < CELL_SIZE; j++) {
        geo_face_t *geo_f = &grid.faces[el.geo.id_faces[j]].geo;
        if (el.geo.sign_n[j]) {
          el.illum_val.div_stream += Stream[j].dot(geo_f->n) * geo_f->S;
        } else {
          el.illum_val.div_stream -= Stream[j].dot(geo_f->n) * geo_f->S;
        }
      }

      el.illum_val.div_stream /= el.geo.V;
    }
  }
  return 0;
}

#endif //! defined ILLUM && defined SOLVERS