# NexiLB

<p align="right"><a href="README.md">English</a> | <strong>简体中文</strong></p>

<div align="center">
  <p><strong>基于 CUDA 的 N 相 LBM–IMB–DEM 复杂流体–颗粒系统仿真程序</strong></p>
  <p>
    <a href="https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/tag/v1.0.0"><img alt="NexiLB v1.0.0" src="https://img.shields.io/badge/release-v1.0.0-1565c0"></a>
    <a href="https://github.com/HaoqiSong-Pro/nexilb-sdk/actions/workflows/check.yml"><img alt="Public SDK checks" src="https://github.com/HaoqiSong-Pro/nexilb-sdk/actions/workflows/check.yml/badge.svg"></a>
    <a href="https://haoqisong-pro.github.io/nexilb-docs/zh-cn/"><img alt="NexiLB 中文文档" src="https://img.shields.io/badge/docs-GitHub%20Pages-0b7285"></a>
    <a href="LICENSE.md"><img alt="Restricted research-use license" src="https://img.shields.io/badge/license-restricted%20research%20use-b45309"></a>
  </p>
</div>

> **NexiLB** 源自 **N-phase Extensible Interface Lattice Boltzmann**。程序面向不可混溶 N 相流动、复杂表面润湿、浸入边界以及颗粒–流体–接触耦合，通过受控 CUDA 运行库、稳定 C/C++ SDK 和可扩展案例接口连接数值模型与研究应用。

> [!IMPORTANT]
> NexiLB 相关论文目前仍处于同行评审阶段，尚未通过评审并正式发表，因此 v1.0.0 暂时采用自定义的受限研究使用许可。这是服务于现阶段研究延续与负责任共享的临时安排，并不代表 NexiLB 长期开放程度的上限。
>
> 这项工作凝聚了我们近三年的探索与努力。我们对这项研究充满期待，也真诚希望相关模型与程序能够支持更多前沿研究，在新的问题中发挥它的最大价值。一个可信、可复现并能够长期发展的研究程序，需要作者与同行使用者共同维护，也需要大家在使用、验证与改进中付出持续努力。
>
> 我们承诺：一旦论文通过同行评审，仓库权限将进一步开放；随着研究持续深入，代码内容与使用权限也将逐步扩大。NexiLB 后续的发布与许可策略只会向更加开放的方向发展。

<table>
  <tr>
    <td width="50%" align="center">
      <a href="https://haoqisong-pro.github.io/nexilb-docs/zh-cn/#featured-simulations">
        <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__CylinderWaterEntry__D75__phase__2d-xy__t000000-t011461.gif" alt="NexiLB 圆柱入水相场与浸入边界演化" width="100%">
      </a><br>
      <strong>圆柱入水 · 相场与浸入边界</strong>
    </td>
    <td width="50%" align="center">
      <a href="https://haoqisong-pro.github.io/nexilb-docs/zh-cn/#featured-simulations">
        <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__CylinderWaterEntry__D75__experiment-vs-simulation.png" alt="NexiLB 圆柱入水实验与仿真形态对比" width="100%">
      </a><br>
      <strong>圆柱入水 · 实验与仿真形态</strong>
    </td>
  </tr>
</table>
<p align="center"><em>圆柱入水：界面形变与浸入边界 · 查看更多 <a href="https://haoqisong-pro.github.io/nexilb-docs/zh-cn/#featured-simulations">精选案例</a></em></p>

## Why NexiLB?

- **N 相界面与复杂润湿** — 支持不可混溶 N 相流动、逐对界面张力、平面及嵌入曲面接触角，并可处理多条接触线协同演化。
- **流体–颗粒–接触耦合** — 将 LBM、浸入边界方法和 DEM 结合，用于固定边界、规定运动刚体、自由颗粒以及颗粒接触问题。
- **CUDA 加速** — 提供 2D/3D、f32/f64 四种运行变体，首版支持 WSL2、Ubuntu 24.04、CUDA 13.3 和 `sm_120` 参考环境。
- **面向个人 GPU 的高性能计算** — 支持在个人游戏本和高性能笔记本上开展计算，可在数十小时内完成千万级 LBM 网格与数千颗粒的全耦合研究算例。
- **稳定的二次开发入口** — 公共 C11 ABI、C++17 封装、CMake package 和四个可编译案例模板，使新问题无需依赖私有实现即可接入统一生命周期。
- **可复现案例合同** — 模型、维度、精度、输入资产、能力要求和验收定义均可机器读取，便于比较配置、结果与后续版本。

如果 NexiLB 对你的研究有帮助，欢迎为仓库点亮 ⭐，并在使用前阅读[许可边界](#license)与[引用说明](#citation--paper)。

## Get NexiLB

| Resource | Link |
|---|---|
| Documentation | [NexiLB 中文文档](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/) |
| Current release | [NexiLB v1.0.0](https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/tag/v1.0.0) |
| Getting Started | [选择模型并开始使用](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/getting-started/) |
| Research-use application | [许可与申请要求](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/licensing/) |
| Developer Guide | [创建新的案例](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/secondary-development/) |

> [!IMPORTANT]
> NexiLB v1.0.0 因相关论文仍处于同行评审阶段，暂时使用自定义的受限研究使用许可。仓库公开、程序包可下载或持有文件都不会自动授予运行、修改或再分发权利；实际使用前须取得 Haoqi Song 针对具体研究项目签发的书面授权。论文通过评审后，仓库权限将进一步开放。

选择与仿真维度及浮点精度一致的程序包：

| Variant | Runtime package |
|---|---|
| 2D · f32 | [`d2-f32`](https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/download/v1.0.0/NexiLB-v1.0.0-wsl2-ubuntu24.04-x86_64-cuda13.3-sm120-d2-f32.tar.gz) |
| 2D · f64 | [`d2-f64`](https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/download/v1.0.0/NexiLB-v1.0.0-wsl2-ubuntu24.04-x86_64-cuda13.3-sm120-d2-f64.tar.gz) |
| 3D · f32 | [`d3-f32`](https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/download/v1.0.0/NexiLB-v1.0.0-wsl2-ubuntu24.04-x86_64-cuda13.3-sm120-d3-f32.tar.gz) |
| 3D · f64 | [`d3-f64`](https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/download/v1.0.0/NexiLB-v1.0.0-wsl2-ubuntu24.04-x86_64-cuda13.3-sm120-d3-f64.tar.gz) |

首版运行库面向 64 位 Linux ABI。Windows 用户通过 WSL2 与 Ubuntu 24.04 使用；首版参考环境为 CUDA Toolkit 13.3、NVIDIA 610.88 驱动和计算能力 12.0（`sm_120`）的 GeForce RTX 5070 Ti Laptop GPU。使用其他驱动、CUDA 版本或 GPU 架构时，应先以最小案例确认兼容性，并根据研究目标完成必要的数值比较。

## Try NexiLB

解压所选程序包并保持目录结构不变。以下命令以 `d2-f64` 的 N 相润湿示例为例；`<package>` 和 `<build>` 分别替换为解压后的程序包根目录和构建目录。

```console
cmake \
  -S <package>/share/nexilb-sdk/examples/secondary-development/nphase-wetting-minimal \
  -B <build> \
  -DCMAKE_PREFIX_PATH=<package>
cmake --build <build> --config Release

cd <package>/share/nexilb-sdk/examples/secondary-development/nphase-wetting-minimal
<build>/nexilb_nphase_wetting_consumer \
  <package>/lib/libnexilb_runtime.so.1 \
  assets/2d/config-f64.txt
```

消费者必须从案例包根目录启动。配置路径及其引用的颗粒表、轨迹和几何资产均使用 `/`，保持为案例目录内的相对路径。计算结果可以在 ParaView 中查看 VTS 内的速度、压力、密度、相场以及颗粒状态。完整过程见 [Installation](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/installation/) 与 [Running NexiLB](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/running/)。

## Four Models

NexiLB 提供四个明确的模型入口。选择模型时应以物理问题为依据；规定运动边界不等同于自由颗粒动力学，纯流体 N 相问题也不需要构造空颗粒系统。

| Model | Stable ID | Intended use |
|---|---|---|
| NSAllen | `model.nexilb.NSAllen` | 两相流动与界面演化 |
| NSAllenImbPrescribedMotion | `model.nexilb.NSAllenImbPrescribedMotion` | 两相流动与由轨迹给定的浸入移动边界 |
| NPhaseContactAngle | `model.NPhaseContactAngle` | 不可混溶 N 相流动、静态表面和嵌入曲面润湿 |
| NPhaseImbDemContactAngle | `model.NPhaseImbDemContactAngle` | 自由颗粒、移动表面润湿、流固耦合与 DEM 接触 |

模型方程、适用范围和配置方法见 [Core Models & API](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/models/)。

## Examples

<table>
  <tr>
    <td width="50%" align="center">
      <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__CylinderWaterEntry__D75__phase__2d-xy__t000000-t011461.gif" alt="NexiLB 圆柱入水计算与实验对比" width="100%"><br>
      <strong>圆柱入水 · 计算与实验对比</strong>
    </td>
    <td width="50%" align="center">
      <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__ThreePhaseContactAngle__theta-60-90__long-run.gif" alt="NexiLB 三相接触角演化" width="100%"><br>
      <strong>三相接触角 · 独立润湿条件</strong>
    </td>
  </tr>
  <tr>
    <td width="50%" align="center">
      <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__LiquidLens__condition-iii__long-run-t000000-t1000000.gif" alt="NexiLB 三相液透镜演化" width="100%"><br>
      <strong>三相液透镜 · 非等界面张力</strong>
    </td>
    <td width="50%" align="center">
      <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__GranularCollapse__submerged-side-projection.gif" alt="NexiLB 三维浸没态颗粒柱坍塌" width="100%"><br>
      <strong>3D 颗粒柱坍塌 · 浸没工况</strong>
    </td>
  </tr>
</table>

文档首页展示了六个主题案例板和十六组演化序列，包括 Rayleigh–Taylor 不稳定性、液透镜、曲面润湿、三相接触角、圆柱入水与颗粒柱坍塌。进入 [精选案例](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/#featured-simulations) 查看完整展示。

## SDK & Developer Interface

NexiLB 的公共接口以稳定 C11 ABI 为核心，并提供 C++17 封装。运行库只要求使用者解析 `nexilb_get_abi_version` 与 `nexilb_get_api` 两个引导符号；随后通过 catalog 选择模型，创建 context 和 model，载入配置，初始化并推进，在同步点读取 snapshot 或 checkpoint，最后按创建顺序的反方向释放对象。

```cmake
find_package(NexiLBHeaders 1.0.0 EXACT CONFIG REQUIRED)
add_executable(my_nexilb_case main.c)
target_compile_features(my_nexilb_case PRIVATE c_std_11)
target_link_libraries(my_nexilb_case PRIVATE NexiLB::Headers ${CMAKE_DL_LIBS})
```

仓库提供四个可编译二次开发模板：

| Template | Starting point |
|---|---|
| `nphase-wetting-minimal` | N 相纯流体、壁面或嵌入曲面润湿 |
| `imb-coupling-minimal` | 固定颗粒、规定轨迹和流固耦合 |
| `dem-contact-restart` | 自由颗粒、接触历史与 checkpoint/restart |
| `pure-coupled-degeneration` | 纯流体与耦合模型的退化一致性比较 |

建立新案例时，从物理过程最接近的模板复制案例包，并明确更新模型 ID、输入资产、2D/3D 与 f32/f64 变体、所需能力和验收条件。具体目录、消费者接入和测试方法见 [Developer Guide](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/secondary-development/) 与 [`examples/secondary-development`](examples/secondary-development/README.md)。

## Documentation

| Guide | Purpose |
|---|---|
| [Getting Started](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/getting-started/) | 认识四个模型并选择计算入口 |
| [Installation](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/installation/) | 准备运行环境、程序包与 SDK |
| [API Reference](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/api/) | C ABI、对象生命周期与可运行示例 |
| [Model Equations](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/theory/) | 连续模型、格子离散与耦合方法 |
| [Validation](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/validation/) | 为新问题建立数值比较与验收条件 |
| [References](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/references/) | 模型与数值方法的主要参考文献 |

## Citation & Paper

NexiLB 相关论文正在同行评审阶段。论文正式发表后，推荐题目、作者、期刊、DOI 和引用格式将在 [Paper](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/papers/) 页面更新。在此之前，研究成果中应说明使用了 NexiLB，并联系维护者确认适当的致谢方式。

仓库已经提供 [`CITATION.cff`](CITATION.cff)。当前版本可记为：

> Haoqi Song. *NexiLB public SDK*, version 1.0.0, 2026.

## License

Copyright © 2026 Haoqi Song. All rights reserved.

NexiLB v1.0.0 采用自定义的 `LicenseRef-NexiLB-Research-Use-1.0` 受限研究使用许可。只有单独签发、明确列出被许可人、研究项目、版本、用途、平台和期限的书面授权才会产生使用权。完整条款见 [`LICENSE.md`](LICENSE.md)，申请要求见 [License](https://haoqisong-pro.github.io/nexilb-docs/zh-cn/licensing/)。

这一许可安排源于相关论文尚处于同行评审阶段，是服务于当前研究与负责任共享的临时边界。近三年的探索使我们更加期待 NexiLB 能够支持更多前沿研究，并在同行的使用、验证与共同改进中持续成长。论文通过评审后，仓库权限将进一步开放；随着研究持续深入，代码内容与使用权限也将逐步扩大，后续发布与许可策略只会向更加开放的方向发展。

## People & Acknowledgements

- **Supervisor — 王路君**：浙江大学建筑工程学院研究员、博士生导师，指导本项目的研究方向、数值方法与学术工作。[Official profile](https://person.zju.edu.cn/0616512)
- **Author & Maintainer — Haoqi Song**：浙江大学建筑工程学院土木水利专业硕士研究生；NexiLB 的设计者、主要开发者与维护者。[ResearchGate](https://www.researchgate.net/profile/Haoqi-Song-3) · [haoqisong@126.com](mailto:haoqisong@126.com)
- **Contributor · v1.0.0 — Guochong Liu**：对理论与验证完备性提出重要意见，并参与部分图件整理，为首版发布提供重要辅助支持。

NexiLB 依托 Haoqi Song 硕士研究课题开展，面向复杂多相流与颗粒耦合数值方法研发。
