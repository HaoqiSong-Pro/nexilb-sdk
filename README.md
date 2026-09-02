# NexiLB SDK

NexiLB v1.0.0 provides a stable C11 ABI, a C++17 wrapper, machine-readable
model and case contracts, and runnable secondary-development examples for the
NexiLB CUDA runtime. The numerical implementation remains proprietary and is
distributed separately as four platform-specific runtime packages.

The v1.0.0 catalog exposes four models:

- `model.nexilb.NSAllen`
- `model.nexilb.NSAllenImbPrescribedMotion`
- `model.NPhaseContactAngle`
- `model.NPhaseImbDemContactAngle`

The binary release targets WSL2 with Ubuntu 24.04 on x86-64, CUDA 13.3 and
`sm_120`. Choose exactly one package for the simulation dimension and
floating-point precision you need: `d2-f32`, `d2-f64`, `d3-f32`, or
`d3-f64`.

## Use a release package

Extract the selected v1.0.0 archive and keep its directory structure intact.
The public headers are under `include/nexilb`, the runtime is
`lib/libnexilb_runtime.so.1`, and the SDK contracts and examples are under
`share/nexilb-sdk`.

An external CMake project can consume the headers with:

```cmake
find_package(NexiLBHeaders 1.0.0 EXACT CONFIG REQUIRED)
target_link_libraries(my_consumer PRIVATE NexiLB::Headers)
```

Pass the extracted package root through `CMAKE_PREFIX_PATH`. Runtime consumers
load `lib/libnexilb_runtime.so.1` explicitly and negotiate API v1 through
`nexilb_get_abi_version` and `nexilb_get_api`; no private include directory
or implementation symbol is required.

To build the N-phase wetting example from an extracted package:

```text
cmake -S <package>/share/nexilb-sdk/examples/secondary-development/nphase-wetting-minimal -B <build> -DCMAKE_PREFIX_PATH=<package>
cmake --build <build>
cd <package>/share/nexilb-sdk/examples/secondary-development/nphase-wetting-minimal
<build>/nexilb_nphase_wetting_consumer <package>/lib/libnexilb_runtime.so.1 assets/2d/config-f32.txt
```

The other examples use the same loading and lifecycle rules. See
`examples/secondary-development/README.md` for their commands and the
operation-specific capability gates.

## Build and verify the SDK

```text
cmake --preset contract
cmake --build --preset contract
ctest --preset contract
```

These checks compile the C and C++ interfaces, validate the public catalog and
case contracts, build all four consumers, install `NexiLBHeaders`, and verify
that consumers fail safely when no runtime path is supplied.

## License

NexiLB is distributed under the custom
`LicenseRef-NexiLB-Research-Use-1.0` boundary. Repository visibility,
download, or possession does not grant a right to use the program. A separate
written research-use authorization from Haoqi Song is required. Read
`LICENSE.md` and `licensing/README.md` before requesting access.

Maintainer and research-use contact: Haoqi Song,
`haoqisong@126.com`.
