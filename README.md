A template project to help you get started with building a JSON publisher using Roq's C++ API.

The project includes

* Bare-minimum implementation of a JSON publisher

* Test target

  * Using Catch2

* Benchmark target

  * Using Google benchmark

* Conda build recipe


## Prerequisites

> Use `stable` for (the approx. monthly) release build.
> Use `unstable` for the more regularly updated development builds.

### Initialize sub-modules

```bash
git submodule update --init --recursive
```

### Create development environment

```bash
scripts/create_conda_env unstable debug
```

### Activate environment

```bash
source opt/conda/bin/activate dev
```

## Build the project

> Sometimes you may have to delete CMakeCache.txt if CMake has already cached an incorrect configuration.

```bash
cmake . && make -j4
```

## License

The project is released under the terms of the MIT license.
