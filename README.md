# xx-cli

[![release](https://github.com/Gallardo994/xx-cli/actions/workflows/release.yml/badge.svg)](https://github.com/Gallardo994/xx-cli/actions/workflows/release.yml)
[![testing](https://github.com/Gallardo994/xx-cli/actions/workflows/testing.yml/badge.svg)](https://github.com/Gallardo994/xx-cli/actions/workflows/testing.yml)

xx-cli is a CLI tool for executing pre-made command aliases on per-project basis, meaning the commands can be shared across team members working on the same project. Alongside aliases, xx-cli includes self-contained Lua VM that can make abstracting OS differences easier.

It is both suitable for running locally, as well as in CI/CD pipelines by wget'ing or curl'ing a pre-compiled binary from the releases page. No extra dependencies are required to run the binaries, however, some execution engines might require external dependencies (e.g. Dotnet Run engine requires .NET SDK to be installed).

## Install

### Homebrew (MacOS/Linux) (recommended)

On MacOS/Linux you can install xx-cli via Homebrew:

```bash
brew install gallardo994/tap/xx
```

To update, be sure to use `brew update` before `brew upgrade xx`.

### Script installation (MacOS/Linux)

MacOS/Linux:

```bash
curl -sSL https://raw.githubusercontent.com/Gallardo994/xx-cli/refs/heads/main/install.sh | bash
```

You can re-run the script to update xx-cli to the latest version, if you have it already installed.

### Manual installation (Windows)

Windows:
1. Visit the [releases page](https://github.com/Gallardo994/xx-cli/releases)
2. Download the latest `xx-windows-x86_64-release.exe` if you're on Windows x86_64, or `xx-windows-arm64-release.exe` if you're on Windows ARM64.
3. Rename the downloaded file to `xx.exe` and place it somewhere in your PATH.

If you're unsure which architecture you have, you most likely have x86_64.

Updating is done by repeating the same steps, however, you can check for updates using `xx check-updates` command.

## Example YAML Configuration

`.xx.yaml` file in current working directory:

```yaml
alias:
  # `xx run helloworld` will print "Hello World!" to the console.
  # `xx run helloworld greeting=Hi target=Everyone` will print "Hi Everyone!" to the console.
  # `xx run helloworld greeting=It\'s target=$(date)` will print "It's, <current date>!" to the console, if your shell supports command substitution.
  # User will be prompted for confirmation before executing the command unless --yolo flag is provided.
  helloworld:
    cmd: 'echo "{{ greeting }} {{ target }}!"'
    render_engine: inja
    template_vars:
      greeting: "Hello"
      target: "World"
    requires_confirmation: true

  # `xx run luacode values="1,2,3,4,5"` will print sum of those values to the console using embedded Lua VM. No dependency on external Lua installation.
  luacode:
    cmd: |
      local t = {}
      -- Use TEMPLATE_VARS, ENVS and CTX tables to read context information.
      for str in string.gmatch(TEMPLATE_VARS.values, '([^,]+)') do
          table.insert(t, tonumber(str))
      end

      local sum = 0
      for k,v in pairs(t) do
          sum = sum + v
      end

      print(tostring(sum))

      -- Returning 0 indicates success, just as not returning anything at all.
      -- Returning a string will treat it as a command to execute in system shell.
      return 0
    execution_engine: lua
    template_vars:
      values: "0"
      
  # `xx run dotnetcode` will execute a simple C# program using Dotnet Run execution engine, with a Nuget dependency. Requires .NET 10 to be installed.
  dotnetcode:
    - cmd: |
        #:package Humanizer@2.14.1

        using System;
        using System.Linq;
        using Humanizer;

        var currentDirectory = Environment.CurrentDirectory;
        var timeToMidnight = new DateTime(DateTime.Now.Year, DateTime.Now.Month, DateTime.Now.Day, 0, 0, 0).AddDays(1) - DateTime.Now;

        Console.WriteLine($"Current Directory: {currentDirectory}");
        Console.WriteLine($"Time to Midnight: {timeToMidnight.Humanize()}");

      execution_engine: dotnet_run

  # `xx run build` will configure and build the project using CMake and Ninja. Linux+MacOS and Windows versions are separate.
  build:
    - cmd: "cmake . --preset default --fresh && ninja -C build/"
      constraints:
        - osfamily: unix

    - cmd: "cmake . --preset default --fresh; if ($?) { ninja -C build/ }"
      constraints:
        - osfamily: windows
      env:
        CC: "C:/tools/msys64/clang64/bin/clang.exe"
        CXX: "C:/tools/msys64/clang64/bin/clang++.exe"

  # `xx run path` will print the PATH environment variable according to the OS.
  path:
    - cmd: "echo $PATH"
      constraints:
        - osfamily: unix

    - cmd: "echo $Env:Path"
      constraints:
        - osfamily: windows
```

User-defined configuration is stored in these locations:

- `~/.config/xx/xx.yaml` on Linux and MacOS
- `%APPDATA%\xx\xx.yaml` on Windows

Be sure to name your aliases uniquely as they may conflict between project-level and user-level configurations.

## Commands

Generally you're going to use `xx run <alias>` (use `xx run --dry <alias>` to simulate command execution without actually running it) and `xx list` to see all the available aliases (with `--grep abc` to quickly find what you're looking for).

Use `xx --help` to see the list of available commands.

## System shell execution

On Linux and MacOS, the default system shell (e.g. `/bin/sh`) is used, while on Windows specifically `powershell.exe` is used.

## Lua execution engine

When using the Lua execution engine, the following global tables are available within the Lua script:
- `TEMPLATE_VARS`: A table containing the template variables passed to the alias.
- `ENVS`: A table containing the environment variables defined for the alias.
- `CTX`: A table containing context information such as OS type, architecture, etc.

The Lua script can return:
- A number: Treated as the exit code of the command.
- A string: Treated as a command to be executed in the system shell.
- A nil value (or no return): Treated as a successful execution (exit code 0).

Available plugins/modules:
- `json`: For JSON serialization and deserialization using nlohmann::json bindings.
  - `json.dump(lua_table)` - Serializes a Lua table to a JSON string.
  - `json.load(json_string)` - Deserializes a JSON string into a Lua table.
- `pcr`: For making HTTP requests using cpr bindings. Each function returns a table `status_code`, `body`, `headers`, `url` and `error` fields.
  - `pcr.get(url, [ queryParametersTable, headersTable ])` - Makes a GET request to the specified URL with optional parameters.
  - `pcr.post(url, [ postBodyString, queryParametersTable, headersTable ])` - Makes a POST request to the specified URL with optional parameters.
- `fs` for filesystem operations using `std::filesystem`-compatible API.

## Dotnet Run execution engine

When using the Dotnet Run execution engine, the command specified should be a single-file C# program, which will be fed to system's `dotnet run` command.

This requires .NET SDK to be installed on the system and `dotnet` available in PATH.

## Third-Party Libraries

Special thanks to the following open-source projects:

- [CLI11](https://github.com/CLIUtils/CLI11)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [spdlog](https://github.com/gabime/spdlog)
- [inja](https://github.com/pantor/inja)
- [Lua](https://www.lua.org/)
- [nlohmann_json_lua](https://github.com/graymadness/nlohmann_json_lua)
- [neargye-semver](https://github.com/Neargye/semver)
- [cpr](https://github.com/libcpr/cpr)
- [filesystem](https://github.com/PG1003/filesystem)

## Building and Testing

See .xx.yaml for currently used build presets. Generally you can use CMake to configure and build the project:

```bash
cmake --preset testing # or default / release
cmake --build --preset testing # or default / release
```

Or just use `xx` binary itself, if you have it installed locally:

```bash
xx run test # runs the test suite
xx run build preset=testing # or default / release
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
