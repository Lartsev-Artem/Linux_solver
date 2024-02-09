/**
 * @file global_consts.h
 * @brief Файл содержит глобальные определения всех констант
 *
 */
#ifndef GLOBAL_CONSTS_H
#define GLOBAL_CONSTS_H

constexpr double PI = 3.1415926535897932384626433832795; ///<число пи

constexpr double kMinPressure = 1e-12;
constexpr double kMinDensity = 1e-12;

constexpr double kGamma1 = 4.0 / 3; ///< показатель адиабаты
constexpr double kGamma_g = kGamma1 / (kGamma1 - 1.0);

#if 1 //СГС

constexpr double kC_Light = 3 * 1e10;                     ///<скорость света в м/c
constexpr double kC_LightInv = (1.0 / (kC_Light));        ///< обратная величина к скорости света
constexpr double kR_gas = 83144626.1815324;               ///< газовая постоянная [ Дж/(моль*К)]
constexpr double kH_plank = 6.62 * 1e-27;                 ///< постоянная Планка[кг * м^2 /с]
constexpr double k_boltzmann = 1.3807 * 1e-16;            ///< постоянная Больцмана[Дж/K] = [ кг*м^2/(с^2*T)]
constexpr double kSigma_thomson = 6.65210 * 1e-25;        ///< сечение томсоновского рассеяния [m^2]
constexpr double kM_hydrogen = 1.6735575 * 1e-24;         ///< масса водорода[кг]
constexpr double kDistAccretor = 3.88190065213158 * 1e10; ///< характерное расстояние
constexpr double kStefanBoltzmann = 5.670374419 * 1e-5;   ///< постоянная Стефана-Больцмана[ эрг·с^−1·см^−2·К^−4]
constexpr double kM_electron = 9.109383701528 * 1e-28;    ///< масса электрона [г]
constexpr double kR_electron = 2.8179403 * 1e-13;         ///< радиус электрона [см]

#else //СИ

constexpr double kC_Light = 299792458.0;                ///<скорость света в м/c
constexpr double kC_LightInv = (1.0 / (kC_Light));      ///< обратная величина к скорости света
constexpr double kR_gas = 8.314;                        ///< газовая постоянная [ Дж/(моль*К)]
constexpr double kH_plank = 6.62 * 1e-34;               ///< постоянная Планка[кг * м^2 /с]
constexpr double k_boltzmann = 1.38 * 1e-23;            ///< постоянная Больцмана[Дж/K] = [ кг*м^2/(с^2*T)]
constexpr double kSigma_thomson = 6.65210 * 1e-29;      ///< сечение томсоновского рассеяния [m^2]
constexpr double kM_hydrogen = 1.6735575 * 1e-27;       ///< масса водорода[кг]
constexpr double kStefanBoltzmann = 5.670374419 * 1e-8; ///< постоянная Стефана-Больцмана[Вт*м^-2*К^−4]
constexpr double kM_electron = 9.109383701528 * 1e-31;  ///< масса электрона [кг]
constexpr double kR_electron = 2.8179403 * 1e-15;       ///< радиус электрона [м]
#endif

constexpr double kEarthMass = (5.9722 * 1e25); ///< масса Земли в кг
constexpr double kSunMass = (1.9891 * 1e31);   ///< масса Солнца в кг

constexpr double kDistSun = (149.6 * 10e9); ///< расстояние до Солнца в м
constexpr double kDistMoon = 400000000.;    ///<расстояние до Луны в м

constexpr double kDist = 1e6;          ///< характерное расстояние
constexpr double kMass = (1 * 1e21);   ///< характерная масса
constexpr double kVelocity = kC_Light; ///< характерная скорость
// constexpr double kTemperature = 4000;         ///< характерная температура
constexpr double kTime = (kDist / kVelocity); ///< характерное время

// constexpr double kDensity = (kMass / (kDist * kDist * kDist));   ///< характерная плотность
// constexpr double kPressure = (kMass / (kDist * kTime * kTime));  ///< характерное давление
// constexpr double kRadiation = (kMass / (kTime * kTime * kTime)); ///< характерное излучение

constexpr double kDensity = (3.34 * 10e-14);                                  ///< характерная плотность
constexpr double kPressure = (kDensity * kVelocity * kVelocity);              ///< характерное давление
constexpr double kRadiation = (kDensity * kVelocity * kVelocity * kVelocity); ///< характерное излучение

#endif //! GLOBAL_CONSTS_H
