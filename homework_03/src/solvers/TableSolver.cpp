#include "solvers/TableSolver.h"

#include <format>
#include <fstream>
#include <string_view>
#include <vector>

namespace {

// Результат в кожному вузлі сітки
struct Result {
  double t;      // час польоту
  double hDist;  // горизонтальна дистанція
};

// Лінійна інтерполяція для Result (обидва поля паралельно)
Result lerp(const Result& a, const Result& b, double t)
{
  return {a.t + (b.t - a.t) * t, a.hDist + (b.hDist - a.hDist) * t};
}

// Індекс і коефіцієнт для одного виміру
struct Interp {
  int lo;       // нижній індекс в осі
  double frac;  // коефіцієнт [0..1]
};

Interp findInterp(double val, const std::vector<double>& axis)
{
  if (val <= axis.front())
    return {0, 0.0f};
  if (val >= axis.back())
    return {(int)axis.size() - 2, 1.0f};

  auto it = std::lower_bound(axis.begin(), axis.end(), val);
  int i = (int)(it - axis.begin()) - 1;
  if (i < 0)
    i = 0;

  double frac = (val - axis[i]) / (axis[i + 1] - axis[i]);
  return {i, frac};
}

}  // namespace

struct TableSolver::BallisticTable {
private:
  BallisticTable(const BallisticTable&) = delete;
  BallisticTable& operator=(const BallisticTable&) = delete;

public:
  BallisticTable() = default;
  ~BallisticTable() = default;
  BallisticTable(BallisticTable&&) = default;
  BallisticTable& operator=(BallisticTable&&) = default;

public:
  // 5 осей — кожна зі своїм набором вузлів (нерівномірний крок)
  std::vector<double> axisZ0;  // висота
  std::vector<double> axisV0;  // швидкість
  std::vector<double> axisM;   // маса
  std::vector<double> axisD;   // опір
  std::vector<double> axisL;   // підйомна сила

  // Плоский масив розміром |Z0| * |V0| * |M| * |D| * |L|
  std::vector<Result> data;

public:
  // Індекс у плоскому масиві: [iZ0][iV0][iM][iD][iL]
  size_t index(int iz, int iv, int im, int id, int il) const
  {
    return ((((size_t)iz * axisV0.size() + iv) * axisM.size() + im) * axisD.size() + id) * axisL.size() + il;
  }

  const Result& at(int iz, int iv, int im, int id, int il) const { return data[index(iz, iv, im, id, il)]; }

  bool Load(std::string_view dataFolderPath)
  {
    const auto ballisticTablePath = dataFolderPath.data() + std::string("/ballistic_table.txt");
    std::ifstream f(ballisticTablePath);
    if (!f.is_open())
      return false;

    int nZ, nV, nM, nD, nL;
    f >> nZ >> nV >> nM >> nD >> nL;

    axisZ0.resize(nZ);
    for (auto& v : axisZ0)
      f >> v;
    axisV0.resize(nV);
    for (auto& v : axisV0)
      f >> v;
    axisM.resize(nM);
    for (auto& v : axisM)
      f >> v;
    axisD.resize(nD);
    for (auto& v : axisD)
      f >> v;
    axisL.resize(nL);
    for (auto& v : axisL)
      f >> v;

    size_t total = (size_t)nZ * nV * nM * nD * nL;
    data.resize(total);

    // Порядок: Z0 → V0 → m → d → l (зовнішній → внутрішній)
    for (size_t i = 0; i < total; i++)
      f >> data[i].t >> data[i].hDist;

    return f.good();
  }

  Result Lookup(double Z0, double V0, double m, double d, double l) const
  {
    Interp iz = findInterp(Z0, axisZ0);
    Interp iv = findInterp(V0, axisV0);
    Interp im = findInterp(m, axisM);
    Interp id = findInterp(d, axisD);
    Interp il = findInterp(l, axisL);

    // 2^5 = 32 вершини гіперкуба
    // Згортаємо: 32 → 16 → 8 → 4 → 2 → 1

    // l: 32 → 16
    Result v[16];
    for (int a = 0; a < 2; a++)
      for (int b = 0; b < 2; b++)
        for (int c = 0; c < 2; c++)
          for (int e = 0; e < 2; e++) {
            auto& lo = at(iz.lo + a, iv.lo + b, im.lo + c, id.lo + e, il.lo);
            auto& hi = at(iz.lo + a, iv.lo + b, im.lo + c, id.lo + e, il.lo + 1);
            v[a * 8 + b * 4 + c * 2 + e] = lerp(lo, hi, il.frac);
          }

    // d: 16 → 8
    Result w[8];
    for (int a = 0; a < 2; a++)
      for (int b = 0; b < 2; b++)
        for (int c = 0; c < 2; c++)
          w[a * 4 + b * 2 + c] = lerp(v[a * 8 + b * 4 + c * 2], v[a * 8 + b * 4 + c * 2 + 1], id.frac);

    // m: 8 → 4
    Result u[4];
    for (int a = 0; a < 2; a++)
      for (int b = 0; b < 2; b++)
        u[a * 2 + b] = lerp(w[a * 4 + b * 2], w[a * 4 + b * 2 + 1], im.frac);

    // V0: 4 → 2
    Result s[2];
    for (int a = 0; a < 2; a++)
      s[a] = lerp(u[a * 2], u[a * 2 + 1], iv.frac);

    // Z0: 2 → 1
    return lerp(s[0], s[1], iz.frac);
  }
};

TableSolver::TableSolver() = default;

TableSolver::~TableSolver() = default;

TargetFireParams TableSolver::Solve(const BallisticsSolverContext& context)
{
  if (!m_ballisticTable) {  // load only once
    m_ballisticTable = std::make_unique<BallisticTable>();
    if (!m_ballisticTable->Load(context.dataPath)) {
      throw std::logic_error(std::format("Cannot load ballistic table from {}", context.dataPath));
    }
  }

  const auto& conf = context.conf.GetConfig();

  if (!solveCommonBallistics(conf)) {
    throw std::logic_error(std::format("Cannot solve ballistics for given ammo {}", conf.ammoName));
  }

  return solveTargetFireParams(context);
}

bool TableSolver::solveCommonBallistics(const DroneConfig& conf)
{
  const auto result =
    m_ballisticTable->Lookup(conf.altitude, conf.attackSpeed, conf.ammoParams.mass, conf.ammoParams.drag, conf.ammoParams.lift);

  m_timeOfFlight = result.t;
  m_horizontalFlightDistance = result.hDist;
  return true;
}