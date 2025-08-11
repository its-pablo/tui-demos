# Overview

Not much here yet. This is just a collection of tech demos as I figure out what I want to do for the garden control TUI. Using `cxxopts` for arg parsing, `tomlplusplus` for config parsing, and `ftxui` for the TUI interface.

Top of my head, some prereqs are `cmake`, `git`, and `gcc`/`g++`. So far I've had success running this just fine on Ubuntu and WSL.

There's some convenience bash scripts you can use to build and run. Namely: `build.sh`, `run.sh`, `build_and_run.sh`, and `clean.sh`.

If you're using VS Code and want to make changes or run the debugger, you'll likely want something along the following in your `.vscode` project config:

* In `tasks.json`:

```
{
    "tasks": [
        {
            "label": "CMake Debug Build",
            "type": "shell",
            "command": "mkdir -p debug && cd debug && cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build ."
        }
    ],
    "version": "2.0.0"
}
```

* In `launch.json` (note the args, which you can change depending on what you want to debug):

```
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "CMake Devices Debug",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceRoot}/debug/demos",
            "args": [
                "-u",
                "test_files/garden_config.toml"
            ],
            "cwd": "${workspaceRoot}",
            "preLaunchTask": "CMake Debug Build"
        }
    ]
}
```

* To avoid having a billion red squiggles, in `c_cpp_properties.json`:

```
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/src/devices"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c17",
            "cppStandard": "gnu++17",
            "compileCommands": "${workspaceFolder}/build/compile_commands.json",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
```
