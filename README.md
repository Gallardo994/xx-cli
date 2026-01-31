# xx-cli [WIP]

[![CI](https://github.com/Gallardo994/xx-cli/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/Gallardo994/xx-cli/actions/workflows/cmake-multi-platform.yml)

xx-cli is a CLI tool for executing pre-made command aliases on per-project basis, meaning the commands can be shared across team members working on the same project.

It is both suitable for running locally, as well as in CI/CD pipelines by wget'ing or curl'ing a pre-compiled binary from the releases page.

## Work in Progress

This project is a work in progress. The features and syntax are subject to change.

## Example TOML Configuration

`.xx.toml` file in current working directory:
```toml
# `xx run helloworld` will print "Hello World!" to the console.
# `xx run helloworld greeting=Hi target=Everyone` will print "Hi Everyone!" to the console.
# `xx run helloworld greeting=It\'s target=$(date)` will print "It's, <current date>!" to the console, if your shell supports command substitution.
# User will be prompted for confirmation before executing the command unless --yolo flag is provided.
[[alias.helloworld]]
cmd = "echo \"{{ greeting }} {{ target }}!\""
render_engine = "inja"
template_vars = { greeting = "Hello", target = "World" }
requires_confirmation = true

# `xx run build` will configure and build the project using CMake and Ninja. Linux+MacOS and Windows versions are separate.
[[alias.build]]
cmd = "cmake . --preset default --fresh && ninja -C build/"
constraints = [ [ "osfamily", "unix" ] ]

[[alias.build]]
cmd = "cmake . --preset default --fresh; if ($?) { ninja -C build/ }"
constraints = [ [ "osfamily", "windows" ] ]
env = { 'CC' = 'C:/tools/msys64/clang64/bin/clang.exe', 'CXX' = 'C:/tools/msys64/clang64/bin/clang++.exe' }

# `xx run path` will print the PATH environment variable on both Unix-like systems and Windows.
[[alias.path]]
cmd = "echo $PATH"
constraints = [ [ "osfamily", "unix" ] ]

[[alias.path]]
cmd = "echo $Env:Path"
constraints = [ [ "osfamily", "windows" ] ]
```

User-defined configuration is stored in these locations:
- `~/.config/xx/xx.toml` on Linux and MacOS
- `%APPDATA%\xx\xx.toml` on Windows

Be sure to name your aliases uniquely as they may conflict between project-level and user-level configurations.

## Commands

Generally you're going to use `xx run <alias>` (use `--dry` to simulate command execution without actually running it) and `xx list` to see all the available aliases (with `--grep abc` to quickly find what you're looking for).

Use `xx --help` to see the list of available commands.

## Third-Party Libraries

Special thanks to the following open-source projects:

- [CLI11](https://github.com/CLIUtils/CLI11)
- [tomlplusplus](https://github.com/marzer/tomlplusplus)
- [spdlog](https://github.com/gabime/spdlog)
- [inja](https://github.com/pantor/inja)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
