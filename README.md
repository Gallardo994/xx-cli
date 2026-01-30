# xx-cli [WIP]

xx-cli is a CLI tool for executing pre-made command aliases on per-project basis, meaning the commands can be shared across team members working on the same project.

## Work in Progress

This project is a work in progress. The features and syntax are subject to change.

## Example TOML Configuration

```toml
# xx run helloworld (Can be run on any OS, as there are no constraints)
[alias.helloworld]
cmd = "echo 'Hello, World!'"

# xx run build (Unix family version, meaning it will be skipped on Windows)
[alias.build]
cmd = "cmake . -G 'Ninja' --fresh && ninja"
constraints = [ [ "osfamily", "unix" ] ]

# xx run build (Windows family version, sets env variables for clang installed via msys2)
[alias.build]
cmd = "cmake . -G 'Ninja' --fresh && ninja"
constraints = [ [ "osfamily", "windows" ] ]
env = { 'CC' = 'C:/tools/msys64/clang64/bin/clang.exe', 'CXX' = 'C:/tools/msys64/clang64/bin/clang++.exe' }

# xx run install (Unix family version)
[alias.install]
cmd = "sudo rm -f /usr/local/bin/xx && sudo cp ./xx/xx-cli /usr/local/bin/xx"
constraints = [ [ "osfamily", "unix" ] ]
```

## Commands

Generally you're going to use `xx run <alias>` (use `--dry` to simulate command execution without actually running it) and `xx list` to see all the available aliases.

Use `xx --help` to see the list of available commands.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.