/**
 * ╔══════════════════════════════════════════════════════════╗
 * ║         市场均衡价格模拟器 (Market Equilibrium Simulator)  ║
 * ║               二分法求解 · 随机冲击模拟                    ║
 * ╚══════════════════════════════════════════════════════════╝
 *
 * 项目简介
 * ────────
 * 基于微观经济学供需模型 (Qd = a - b·P,  Qs = c + d·P)，使用二分法
 * 高精度求解市场均衡价格。支持自定义随机冲击模拟，可分别对需求端和
 * 供给端施加外生扰动，逐轮追踪价格与供需量的动态变化，直观展示外生
 * 冲击在市场中的传导路径。
 *
 * 核心特性
 * ────────
 * · 二分法搜索   — 替代传统线性扫描，精度达 1e-8，最多 200 次迭代
 * · 随机冲击引擎 — 乘法型冲击 (1.0 ± δ)，支持仅需求/仅供给/双重三种模式
 * · 逐轮追踪     — 实时输出每轮冲击后的新均衡价格、变动幅度与供需量
 * · 理论对照     — 同时输出二分法数值解与代数精确解，验证算法正确性
 *
 * 编译 & 运行
 * ────────
 *   g++ -o equilibrium_simulator main.cpp -std=c++17
 *   ./equilibrium_simulator
 *
 * 输入示例
 * ────────
 *   需求函数 Qd = a - b·P
 *     请输入截距 a 和斜率 b：100 2
 *   供给函数 Qs = c + d·P
 *     请输入截距 c 和斜率 d：10 1.5
 *   随机冲击模拟设置：
 *     模拟轮数（如 10）：5
 *     冲击幅度（如 0.1 表示 ±10%）：0.1
 *     冲击类型 (1/2/3)：3
 *
 * 输出示例
 * ────────
 *   初始均衡价格 P* = 25.714286
 *   轮次   需求冲击%    供给冲击%    新均衡P*      价格变动%
 *   ─────────────────────────────────────────────────────
 *   1      +7.34%      -2.11%      28.7143       +11.67%
 *   2      -3.21%      +5.88%      26.3281        -8.31%
 *   ...
 *
 * 作者
 * ────
 *   张津铭 (Zhang Jinming)
 *   上海财经大学 经济学院
 *   GitHub: https://github.com/[your-username]/market-equilibrium-simulator
 *
 * 许可
 * ────
 *   MIT License — 自由使用、修改与分发
 */

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
using namespace std;

// ═══════════════════════════════════════════════════════════
//  二分法求解均衡价格
//  需求: Qd = a - b·P     供给: Qs = c + d·P
//  定义 excess(P) = Qd - Qs = (a-c) - (b+d)·P
//  excess(P) 随 P 严格单调递减，在 [low, high] 内二分查找零点
//
//  参数:
//    a, b  — 需求函数截距与斜率 (b > 0)
//    c, d  — 供给函数截距与斜率 (d > 0)
//    low   — 搜索区间下界
//    high  — 搜索区间上界
//    tol   — 收敛容差 (默认 1e-6)
//
//  返回:
//    均衡价格 P*，若区间内无解则返回 -1.0
// ═══════════════════════════════════════════════════════════
double findEquilibrium(double a, double b, double c, double d,
                       double low, double high, double tol = 1e-6) {
    // 零点定理检查：区间两端 excess 必须异号
    double excess_low  = (a - b * low)  - (c + d * low);
    double excess_high = (a - b * high) - (c + d * high);

    if (excess_low * excess_high > 0) {
        return -1.0;    // 区间内无交点
    }

    double mid;
    int iter = 0, maxIter = 200;
    while ((high - low) > tol && iter < maxIter) {
        mid = (low + high) / 2.0;
        double excess_mid = (a - b * mid) - (c + d * mid);

        if (excess_mid > 0) {
            low = mid;   // Qd > Qs → 价格偏低 → 向右搜索
        } else if (excess_mid < 0) {
            high = mid;  // Qd < Qs → 价格偏高 → 向左搜索
        } else {
            return mid;  // 精确命中零点
        }
        iter++;
    }
    return (low + high) / 2.0;  // 返回区间中点作为近似解
}

// ═══════════════════════════════════════════════════════════
//  生成乘法型随机冲击系数
//  返回 1.0 + δ，其中 δ ∈ [-range, +range] 均匀分布
//
//  例如 range = 0.1 → 返回 [0.9, 1.1] 之间的随机值
// ═══════════════════════════════════════════════════════════
double randomShock(double range) {
    double r = (double)rand() / RAND_MAX;       // r ∈ [0, 1]
    double delta = (r * 2.0 - 1.0) * range;     // δ ∈ [-range, +range]
    return 1.0 + delta;
}

// ═══════════════════════════════════════════════════════════
int main() {
    // UTF-8 控制台输出 (Windows)
    system("chcp 65001 > nul");

    srand((unsigned)time(0));

    cout << R"(
╔══════════════════════════════════════════════╗
║   市场均衡价格模拟器                          ║
║   Market Equilibrium Simulator               ║
║   二分法求解 · 随机冲击模拟                    ║
╚══════════════════════════════════════════════╝
)" << endl;

    // ── 模型参数输入 ──
    double a, b, c, d;
    cout << "需求函数 Qd = a - b·P" << endl;
    cout << "  请输入截距 a 和斜率 b：";
    cin >> a >> b;
    cout << "供给函数 Qs = c + d·P" << endl;
    cout << "  请输入截距 c 和斜率 d：";
    cin >> c >> d;
    cout << endl;

    // 参数校验：b + d 必须 > 0 以保证 excess(P) 单调递减
    if (b + d <= 0) {
        cout << "[错误] 参数 b+d 必须 > 0，否则无稳定均衡解。" << endl;
        cout << "       请确保需求曲线向下倾斜、供给曲线向上倾斜。" << endl;
        return 1;
    }

    // ── 二分法求解初始均衡 ──
    double low = 0.0, high = 1e6;
    double tol = 1e-8;     // 高精度：小数点后 8 位

    double P_star = findEquilibrium(a, b, c, d, low, high, tol);

    if (P_star < 0) {
        cout << "[错误] 在 [0, 1e6] 区间内未找到均衡解。" << endl;
        cout << "       请检查参数：均衡价格可能为负或超出搜索范围。" << endl;
        return 1;
    }

    double Qd_star = a - b * P_star;
    double Qs_star = c + d * P_star;
    double P_theory = (a - c) / (b + d);    // 代数精确解：令 Qd = Qs

    cout << fixed << setprecision(6);
    cout << "═══════════════════════════════════════════" << endl;
    cout << "  初 始 均 衡" << endl;
    cout << "───────────────────────────────────────────" << endl;
    cout << "  二分法数值解 P*  = " << P_star << endl;
    cout << "  代数精确解      = " << P_theory << endl;
    cout << "  绝对误差        = " << fabs(P_star - P_theory) << endl;
    cout << "───────────────────────────────────────────" << endl;
    cout << "  均衡需求量 Qd    = " << Qd_star << endl;
    cout << "  均衡供给量 Qs    = " << Qs_star << endl;
    cout << "═══════════════════════════════════════════" << endl;
    cout << endl;

    // ── 随机冲击模拟 ──
    int num_rounds;
    double shock_range;
    cout << "随机冲击模拟设置：" << endl;
    cout << "  模拟轮数（如 10）：";
    cin >> num_rounds;
    cout << "  冲击幅度（如 0.1 表示 ±10%）：";
    cin >> shock_range;
    cout << endl;

    if (num_rounds <= 0 || shock_range <= 0) {
        cout << "[提示] 冲击模拟已跳过。" << endl;
        cout << "程序结束。" << endl;
        return 0;
    }

    cout << "冲击类型选择：" << endl;
    cout << "  1 — 仅需求冲击（随机扰动需求截距 a）" << endl;
    cout << "  2 — 仅供给冲击（随机扰动供给截距 c）" << endl;
    cout << "  3 — 供需双重冲击（同时扰动 a 和 c）" << endl;
    cout << "  请输入 (1/2/3)：";
    int shock_mode;
    cin >> shock_mode;

    cout << endl;
    cout << "═══════════════════════════════════════════" << endl;
    cout << "  随 机 冲 击 模 拟 结 果" << endl;
    cout << "═══════════════════════════════════════════" << endl;

    // 表头
    cout << left;
    cout << setw(6)  << "轮次"
         << setw(12) << "需求冲击%"
         << setw(12) << "供给冲击%"
         << setw(14) << "新均衡P*"
         << setw(14) << "价格变动%"
         << setw(12) << "新Qd"
         << setw(12) << "新Qs"
         << endl;
    cout << string(82, '─') << endl;

    double cur_a = a, cur_c = c;
    double prev_P = P_star;

    for (int round = 1; round <= num_rounds; round++) {
        double demand_shock_pct = 0.0, supply_shock_pct = 0.0;

        // 按模式施加冲击（乘法型：参数 × (1 + δ)）
        if (shock_mode == 1 || shock_mode == 3) {
            double old_a = cur_a;
            cur_a *= randomShock(shock_range);
            demand_shock_pct = (cur_a / old_a - 1.0) * 100.0;
        }
        if (shock_mode == 2 || shock_mode == 3) {
            double old_c = cur_c;
            cur_c *= randomShock(shock_range);
            supply_shock_pct = (cur_c / old_c - 1.0) * 100.0;
        }

        // 二分法求解新均衡
        double new_P = findEquilibrium(cur_a, b, cur_c, d, low, high, tol);

        if (new_P < 0) {
            cout << setw(6)  << round
                 << setw(12) << demand_shock_pct
                 << setw(12) << supply_shock_pct
                 << "  [无解 — 冲击后供需曲线不再相交]" << endl;
            continue;
        }

        double new_Qd = cur_a - b * new_P;
        double new_Qs = cur_c + d * new_P;
        double price_change = (new_P / prev_P - 1.0) * 100.0;

        cout << setw(6)  << round
             << setw(12) << setprecision(4) << demand_shock_pct
             << setw(12) << setprecision(4) << supply_shock_pct
             << setw(14) << setprecision(6) << new_P
             << setw(14) << setprecision(4) << price_change
             << setw(12) << setprecision(4) << new_Qd
             << setw(12) << setprecision(4) << new_Qs
             << endl;

        prev_P = new_P;
    }

    // ── 汇总：冲击前后对比 ──
    cout << string(82, '─') << endl;
    cout << endl;
    cout << "═══════════════════════════════════════════" << endl;
    cout << "  冲 击 前 后 对 比" << endl;
    cout << "═══════════════════════════════════════════" << endl;
    cout << "  初始均衡价格 P₀ = " << setprecision(6) << P_star << endl;
    double final_P = findEquilibrium(cur_a, b, cur_c, d, low, high, tol);
    cout << "  最终均衡价格 Pₙ = " << final_P << endl;
    double total_change = (final_P / P_star - 1.0) * 100.0;
    cout << "  累计变动幅度    = " << setprecision(2) << total_change << "%" << endl;
    cout << endl;
    cout << "═══════════════════════════════════════════" << endl;
    cout << "  模拟完成。" << endl;
    cout << "  GitHub: https://github.com/[your-username]/market-equilibrium-simulator" << endl;
    cout << "═══════════════════════════════════════════" << endl;

    return 0;
}
