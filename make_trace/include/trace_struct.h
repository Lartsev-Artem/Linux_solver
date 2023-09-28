/**
 * @file trace_struct.h
 * @brief
 *
 */

#if !defined TRACE_STRUCT_H && defined MAKE_TRACE
#define TRACE_STRUCT_H

#include "global_types.h"

namespace trace {

struct BaseTetra_t {
  Vector3 start_point_plane_coord;  ///< начало координат плоскости
  Matrix3 transform_matrix;         ///< матрица перехода из базового тетраэдра в плоскость
  Matrix3 inverse_transform_matrix; ///< матрица перехода из плоскости в базовый тетраэдр

  Matrix3 straight_face; ///< 3 узла интерполяции
  Matrix3 inclined_face; ///< 3 узла интерполяции на наклонной плоскости

  Matrix3 straight_face_inverse; ///< 3 узла интерполяции
  Matrix3 inclined_face_inverse; ///< 3 узла интерполяции на наклонной плоскости

  //	Vector3 center_local_sphere;  // центр описанной сферы около стандартного тетраэдра

  BaseTetra_t() {
    // 3 узла интерполяции
    {
      straight_face << 1. / 6, 1. / 6, 1,
          2. / 3, 1. / 6, 1,
          1. / 6, 2. / 3, 1;
    }

    // 3 узла интерполяции на наклонной плоскости
    {
      inclined_face << 0, sqrt(2. / 3), 1,
          sqrt(2) / 4, 1. / (2 * sqrt(6)), 1,
          -sqrt(2) / 4, 1. / (2 * sqrt(6)), 1;
    }

    //Матрицы перехода из стандартного тетраэдра в координаты наклонной плоскости
    {
      transform_matrix << -1. / sqrt(2), 1. / sqrt(2), 0,
          -1. / sqrt(6), -1. / sqrt(6), sqrt(2. / 3),
          1. / sqrt(3), 1. / sqrt(3), 1. / sqrt(3);
    }

    //Матрицы перехода из наклонной плоскости в  координаты стандартного тетраэдра
    {
      inverse_transform_matrix << -1. / sqrt(2), -1. / sqrt(6), 1. / sqrt(3),
          1. / sqrt(2), -1. / sqrt(6), 1. / sqrt(3),
          0, sqrt(2. / 3), 1. / sqrt(3);
    }

    // Начало координата плоскости
    start_point_plane_coord << 0.5, 0.5, 0;

    inclined_face_inverse = inclined_face.inverse(); // в решении
    straight_face_inverse = straight_face.inverse(); // в решении
  }
};
const BaseTetra_t base_tetra_geo;

// параметры диска и внутренней сферы:
const Type Rsphere = 0.001;
const Type R1disk = 0.001;
const Type R2disk = 0.09;

extern std::vector<Vector3> x_try_surface;
extern std::vector<int> id_try_surface;

} // namespace trace
#endif //! TRACE_STRUCT_H