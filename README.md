# NexiLB

<p align="right"><strong>English</strong> | <a href="README.zh-CN.md">简体中文</a></p>

<div align="center">
  <p><strong>A CUDA-accelerated N-phase LBM–IMB–DEM program for complex fluid–particle systems</strong></p>
  <p>
    <a href="https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/tag/v1.0.0"><img alt="NexiLB v1.0.0" src="https://img.shields.io/badge/release-v1.0.0-1565c0"></a>
    <a href="https://github.com/HaoqiSong-Pro/nexilb-sdk/actions/workflows/check.yml"><img alt="Public SDK checks" src="https://github.com/HaoqiSong-Pro/nexilb-sdk/actions/workflows/check.yml/badge.svg"></a>
    <a href="https://haoqisong-pro.github.io/nexilb-docs/"><img alt="NexiLB Documentation" src="https://img.shields.io/badge/docs-GitHub%20Pages-0b7285"></a>
    <a href="LICENSE.md"><img alt="Restricted research-use license" src="https://img.shields.io/badge/license-restricted%20research%20use-b45309"></a>
  </p>
</div>

> **NexiLB** stands for **N-phase Extensible Interface Lattice Boltzmann**. It connects immiscible N-phase flow, complex surface wetting, immersed boundaries, and particle–fluid–contact coupling through controlled CUDA runtimes, a stable C/C++ SDK, and extensible case interfaces.

> [!IMPORTANT]
> The paper associated with NexiLB is currently under peer review and has not yet been accepted or formally published. NexiLB v1.0.0 therefore uses a custom restricted research-use license as a temporary framework for responsible sharing and continuity of the research. This temporary boundary does not represent the long-term limit of NexiLB's openness.
>
> This work reflects nearly three years of exploration and sustained effort. We have high expectations for the research and sincerely hope that its models and program can support a wider range of frontier studies and realize their full value in new problems. A credible, reproducible, and durable research program depends on continued care from both its authors and the researchers who use, verify, and improve it.
>
> We commit that, once the paper passes peer review, repository access will be broadened. As the research develops, more code and permissions will be made available, and future releases and licensing will move only toward greater openness.

<table>
  <tr>
    <td width="50%" align="center">
      <a href="https://haoqisong-pro.github.io/nexilb-docs/#featured-simulations">
        <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__CylinderWaterEntry__D75__phase__2d-xy__t000000-t011461.gif" alt="NexiLB phase-field and immersed-boundary evolution during cylinder water entry" width="100%">
      </a><br>
      <strong>Cylinder water entry · Phase field and immersed boundary</strong>
    </td>
    <td width="50%" align="center">
      <a href="https://haoqisong-pro.github.io/nexilb-docs/#featured-simulations">
        <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__CylinderWaterEntry__D75__experiment-vs-simulation.png" alt="NexiLB experiment and simulation comparison for cylinder water entry" width="100%">
      </a><br>
      <strong>Cylinder water entry · Experiment and simulation</strong>
    </td>
  </tr>
</table>
<p align="center"><em>Cylinder water entry: interface deformation and immersed boundary · Explore more <a href="https://haoqisong-pro.github.io/nexilb-docs/#featured-simulations">Featured Simulations</a></em></p>

## Why NexiLB?

- **N-phase interfaces and complex wetting** — Simulate immiscible N-phase flow, pairwise surface tensions, contact angles on planar or embedded curved surfaces, and the coordinated evolution of multiple contact lines.
- **Fluid–particle–contact coupling** — Combine LBM, the immersed-boundary method, and DEM for fixed boundaries, prescribed rigid-body motion, freely moving particles, and particle contact.
- **CUDA acceleration** — Use four 2D/3D and f32/f64 runtime variants. The v1.0.0 reference environment is WSL2, Ubuntu 24.04, CUDA 13.3, and `sm_120`.
- **High-performance computing on personal GPUs** — Run research calculations on gaming and high-performance laptops, including fully coupled cases with tens of millions of LBM cells and thousands of particles within tens of hours.
- **Stable extension interface** — Build new problems against a public C11 ABI, C++17 wrapper, CMake package, and four compilable case templates without depending on the private implementation.
- **Reproducible case contracts** — Describe models, dimensionality, precision, input assets, required capabilities, and acceptance criteria in machine-readable form for comparison across configurations, results, and releases.

If NexiLB supports your research, consider starring the repository. Before use, read the [license boundary](#license) and [citation guidance](#citation--paper).

## Get NexiLB

| Resource | Link |
|---|---|
| Documentation | [NexiLB Documentation](https://haoqisong-pro.github.io/nexilb-docs/) |
| Current release | [NexiLB v1.0.0](https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/tag/v1.0.0) |
| Getting Started | [Choose a model and begin](https://haoqisong-pro.github.io/nexilb-docs/getting-started/) |
| Research-use application | [License and application requirements](https://haoqisong-pro.github.io/nexilb-docs/licensing/) |
| Developer Guide | [Create a new case](https://haoqisong-pro.github.io/nexilb-docs/secondary-development/) |

> [!IMPORTANT]
> Because the associated paper is still under peer review, NexiLB v1.0.0 temporarily uses a custom restricted research-use license. Repository visibility, package availability, or possession of files does not itself grant permission to run, modify, or redistribute NexiLB. Before use, obtain written authorization from Haoqi Song for the specific research project. Repository access will be broadened after the paper passes peer review.

Choose the package matching the simulation dimensionality and floating-point precision:

| Variant | Runtime package |
|---|---|
| 2D · f32 | [`d2-f32`](https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/download/v1.0.0/NexiLB-v1.0.0-wsl2-ubuntu24.04-x86_64-cuda13.3-sm120-d2-f32.tar.gz) |
| 2D · f64 | [`d2-f64`](https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/download/v1.0.0/NexiLB-v1.0.0-wsl2-ubuntu24.04-x86_64-cuda13.3-sm120-d2-f64.tar.gz) |
| 3D · f32 | [`d3-f32`](https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/download/v1.0.0/NexiLB-v1.0.0-wsl2-ubuntu24.04-x86_64-cuda13.3-sm120-d3-f32.tar.gz) |
| 3D · f64 | [`d3-f64`](https://github.com/HaoqiSong-Pro/nexilb-sdk/releases/download/v1.0.0/NexiLB-v1.0.0-wsl2-ubuntu24.04-x86_64-cuda13.3-sm120-d3-f64.tar.gz) |

The v1.0.0 runtimes target the 64-bit Linux ABI. Windows users run them through WSL2 and Ubuntu 24.04. The reference environment uses CUDA Toolkit 13.3, NVIDIA driver 610.88, and a GeForce RTX 5070 Ti Laptop GPU with compute capability 12.0 (`sm_120`). For other drivers, CUDA versions, or GPU architectures, first confirm compatibility with a minimal case and then perform the numerical comparisons required by the research objective.

## Try NexiLB

Extract the selected package without changing its directory structure. The following example uses the N-phase wetting case from the `d2-f64` package; replace `<package>` and `<build>` with the extracted package root and build directory.

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

Start the consumer from the case-package root. Configuration paths and referenced particle tables, trajectories, and geometry assets use `/` and remain relative to the case directory. View velocity, pressure, density, phase fields, and particle states from the VTS output in ParaView. See [Installation](https://haoqisong-pro.github.io/nexilb-docs/installation/) and [Running NexiLB](https://haoqisong-pro.github.io/nexilb-docs/running/) for the complete workflow.

## Four Models

NexiLB exposes four explicit model entries. Select the model from the physical problem: a prescribed moving boundary is not free-particle dynamics, and a pure N-phase fluid problem does not require an empty particle system.

| Model | Stable ID | Intended use |
|---|---|---|
| NSAllen | `model.nexilb.NSAllen` | Two-phase flow and interface evolution |
| NSAllenImbPrescribedMotion | `model.nexilb.NSAllenImbPrescribedMotion` | Two-phase flow with an immersed moving boundary defined by a trajectory |
| NPhaseContactAngle | `model.NPhaseContactAngle` | Immiscible N-phase flow and wetting on stationary or embedded curved surfaces |
| NPhaseImbDemContactAngle | `model.NPhaseImbDemContactAngle` | Free particles, moving-surface wetting, fluid–solid coupling, and DEM contact |

Equations, applicability, and configuration are described in [Core Models & API](https://haoqisong-pro.github.io/nexilb-docs/models/).

## Examples

<table>
  <tr>
    <td width="50%" align="center">
      <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__CylinderWaterEntry__D75__phase__2d-xy__t000000-t011461.gif" alt="NexiLB cylinder water-entry simulation" width="100%"><br>
      <strong>Cylinder water entry · Simulation and experiment</strong>
    </td>
    <td width="50%" align="center">
      <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__ThreePhaseContactAngle__theta-60-90__long-run.gif" alt="NexiLB three-phase contact-angle evolution" width="100%"><br>
      <strong>Three-phase contact angle · Independent wetting conditions</strong>
    </td>
  </tr>
  <tr>
    <td width="50%" align="center">
      <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__LiquidLens__condition-iii__long-run-t000000-t1000000.gif" alt="NexiLB three-phase liquid-lens evolution" width="100%"><br>
      <strong>Three-phase liquid lens · Unequal surface tensions</strong>
    </td>
    <td width="50%" align="center">
      <img src="https://haoqisong-pro.github.io/nexilb-docs/_static/media/showcase/NexiLB__GranularCollapse__submerged-side-projection.gif" alt="NexiLB 3D submerged granular-column collapse" width="100%"><br>
      <strong>3D granular-column collapse · Submerged condition</strong>
    </td>
  </tr>
</table>

The documentation homepage presents six themed boards and sixteen evolution sequences covering Rayleigh–Taylor instability, liquid lenses, curved-surface wetting, three-phase contact angles, cylinder water entry, and granular-column collapse. Explore the complete [Featured Simulations](https://haoqisong-pro.github.io/nexilb-docs/#featured-simulations).

## SDK & Developer Interface

The public interface is built around a stable C11 ABI with a C++17 wrapper. A consumer resolves only the two bootstrap symbols `nexilb_get_abi_version` and `nexilb_get_api`; it then selects a model from the catalog, creates a context and model, loads a configuration, initializes and advances the model, reads snapshots or checkpoints at synchronization points, and finally releases objects in reverse creation order.

```cmake
find_package(NexiLBHeaders 1.0.0 EXACT CONFIG REQUIRED)
add_executable(my_nexilb_case main.c)
target_compile_features(my_nexilb_case PRIVATE c_std_11)
target_link_libraries(my_nexilb_case PRIVATE NexiLB::Headers ${CMAKE_DL_LIBS})
```

Four compilable secondary-development templates are included:

| Template | Starting point |
|---|---|
| `nphase-wetting-minimal` | Pure N-phase fluids and wetting on walls or embedded curved surfaces |
| `imb-coupling-minimal` | Fixed particles, prescribed trajectories, and fluid–solid coupling |
| `dem-contact-restart` | Free particles, contact history, and checkpoint/restart |
| `pure-coupled-degeneration` | Degeneration-consistency comparisons between pure-fluid and coupled models |

To create a case, copy the template closest to the intended physics and explicitly update the model ID, input assets, 2D/3D and f32/f64 variant, required capabilities, and acceptance criteria. See the [Developer Guide](https://haoqisong-pro.github.io/nexilb-docs/secondary-development/) and [`examples/secondary-development`](examples/secondary-development/README.md) for the directory structure, consumer integration, and testing workflow.

## Documentation

| Guide | Purpose |
|---|---|
| [Getting Started](https://haoqisong-pro.github.io/nexilb-docs/getting-started/) | Understand the four models and choose a computational entry point |
| [Installation](https://haoqisong-pro.github.io/nexilb-docs/installation/) | Prepare the runtime environment, packages, and SDK |
| [API Reference](https://haoqisong-pro.github.io/nexilb-docs/api/) | Use the C ABI, object lifecycle, and runnable examples |
| [Model Equations](https://haoqisong-pro.github.io/nexilb-docs/theory/) | Study the continuum models, lattice discretization, and coupling methods |
| [Validation](https://haoqisong-pro.github.io/nexilb-docs/validation/) | Define numerical comparisons and acceptance criteria for a new problem |
| [References](https://haoqisong-pro.github.io/nexilb-docs/references/) | Find the principal references for models and numerical methods |

## Citation & Paper

The paper associated with NexiLB is under peer review. Its recommended title, authors, journal, DOI, and citation format will be added to the [Paper](https://haoqisong-pro.github.io/nexilb-docs/papers/) page after formal publication. Until then, research outputs should identify NexiLB and contact the maintainer to confirm appropriate acknowledgement.

The repository includes [`CITATION.cff`](CITATION.cff). The current version may be cited as:

> Haoqi Song. *NexiLB public SDK*, version 1.0.0, 2026.

## License

Copyright © 2026 Haoqi Song. All rights reserved.

NexiLB v1.0.0 uses the custom restricted research-use license `LicenseRef-NexiLB-Research-Use-1.0`. Permission exists only through a separately issued written authorization that identifies the licensee, research project, version, purpose, platform, and term. See [`LICENSE.md`](LICENSE.md) for the complete terms and the documentation [License](https://haoqisong-pro.github.io/nexilb-docs/licensing/) page for application requirements.

This licensing arrangement is a temporary boundary while the associated paper remains under peer review. Nearly three years of exploration have strengthened our hope that NexiLB can support more frontier research and continue to grow through use, verification, and shared improvement by the research community. After peer review, repository access will be broadened; as the research develops, more code and permissions will be released, and future policy will move only toward greater openness.

## People & Acknowledgements

- **Supervisor — Lujun Wang**: Research professor and doctoral supervisor at the College of Civil Engineering and Architecture, Zhejiang University. He guides the project's research direction, numerical methods, and academic work. [Official profile](https://person.zju.edu.cn/0616512)
- **Author & Maintainer — Haoqi Song**: Master's student in Civil and Hydraulic Engineering at the College of Civil Engineering and Architecture, Zhejiang University; designer, principal developer, and maintainer of NexiLB. [ResearchGate](https://www.researchgate.net/profile/Haoqi-Song-3) · [haoqisong@126.com](mailto:haoqisong@126.com)
- **Contributor · v1.0.0 — Guochong Liu**: Provided important advice on theoretical and validation completeness and assisted with the preparation of selected figures, making a significant supporting contribution to the initial release.

NexiLB is developed as part of Haoqi Song's master's research on numerical methods for complex multiphase flows and particle coupling.
