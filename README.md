# xx-cli [WIP]

[![CI](https://github.com/Gallardo994/xx-cli/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/Gallardo994/xx-cli/actions/workflows/cmake-multi-platform.yml)

xx-cli is a CLI tool for executing pre-made command aliases on per-project basis, meaning the commands can be shared across team members working on the same project.

## Work in Progress

This project is a work in progress. The features and syntax are subject to change.

## Example TOML Configuration

```toml
# `xx run helloworld` will print "Hello World!" to the console.
[[alias.helloworld]]
cmd = "echo \"Hello World!\""

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

## Commands

Generally you're going to use `xx run <alias>` (use `--dry` to simulate command execution without actually running it) and `xx list` to see all the available aliases.

Use `xx --help` to see the list of available commands.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
