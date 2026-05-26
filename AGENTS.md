# AGENTS.md

## 项目概要

Fusou 是一个 Windows x64 控制台小游戏，使用 C++20 编写。

本项目明确不追求跨平台。不要为了 Linux、macOS、MinGW、WSL 或其他平台引入兼容层，除非任务明确要求。

## 开发环境

- 操作系统：Windows x64
- IDE：Visual Studio 2026
- 编译器：MSVC
- 构建系统：CMake
- 生成器：Ninja
- C++ 标准：C++20

需要命令行构建时，使用 Visual Studio 自带的 `cmake.exe`，不要假设 `cmake` 已经在 `PATH` 中。

## 构建命令

命令行构建不得假设当前终端已经是正确的 MSVC x64 环境。即使在 Visual Studio 终端中，也必须先检查并确保使用 x64 工具链。

构建前先在 PowerShell 中定位 Visual Studio，并显式加载 x64 DevShell：

```powershell
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsRoot = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath

Import-Module (Join-Path $vsRoot "Common7\Tools\Microsoft.VisualStudio.DevShell.dll")
Enter-VsDevShell -VsInstallPath $vsRoot -DevCmdArguments "-arch=x64 -host_arch=x64"

where.exe cl
where.exe link
```

`where.exe cl` 输出中优先使用的 `cl.exe` 必须来自 `Hostx64\x64`。如果仍然是 `Hostx86\x86`，应停止并报告环境问题，不要继续配置 CMake。

本机环境变量中没有全局 `cmake`。需要命令行构建时，使用 Visual Studio 自带的 `cmake.exe`，不要假设 `cmake` 已经在 `PATH` 中。加载 x64 DevShell 后，使用以下方式获得 Visual Studio 自带的 CMake 和 Ninja：

```powershell
$cmake = Join-Path $vsRoot "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$ninjaDir = Join-Path $vsRoot "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
$env:PATH = "$ninjaDir;$env:PATH"
```

配置 Dev：

```powershell
& $cmake -S . -B out\build\codex-x64-Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFUSOU_STAGE=Dev
```

构建 Dev：

```powershell
& $cmake --build out\build\codex-x64-Debug
```

除非任务明确要求 Shipping 验证，否则默认只执行 Dev 构建。

配置 Shipping：

```powershell
& $cmake -S . -B out\build\codex-x64-Shipping -G Ninja -DCMAKE_BUILD_TYPE=Release -DFUSOU_STAGE=Shipping
```

构建 Shipping：

```powershell
& $cmake --build out\build\codex-x64-Shipping
```

不要使用或新建根目录 `build\` 作为 CMake 构建目录。

AI Agent 命令行构建必须使用独立构建目录：

- Dev: `out\build\codex-x64-Debug`
- Shipping: `out\build\codex-x64-Shipping`

除非用户明确要求，AI Agent 不得使用、清理或删除 `out\build\x64-Debug`。

不要新增项目内的 `CMakePresets.json`、`CMakeSettings.json` 或额外构建脚本，除非任务明确要求。
如果构建失败是因为本地 Visual Studio、CMake、Ninja 或 MSVC 环境不可用，只报告失败原因，不要改项目代码来绕过环境问题。

如果 `out\build\codex-x64-Debug` 已存在，且本次修改未影响 CMake、工具链、资源拷贝、代码生成规则或构建类型，可以直接执行 Dev 构建，不必重新配置。

如果本次修改影响 `CMakeLists.txt`、`tools/GenerateTilePalette.ps1`、资源拷贝规则、编译选项、目标列表或构建类型，必须先重新配置对应构建目录，再执行构建。

## 仓库结构

- `src/Main.cpp` 程序入口
- `src/Fusou.h` 和 `src/Fusou.cpp` 对入口暴露的项目启动层
- `src/App` 应用组装、运行时启动、资源加载协调
- `src/Core` 平台无关的游戏规则、循环、世界模型、表现模型和端口接口
- `src/Platform` Windows 控制台、输入、渲染、时钟等平台适配器等平台相关依赖
- `Assets` JSON 游戏资源
- `Resources` Windows 资源文件和图标
- `third_party` 第三方库
- `FusouMapForge` 是 Fusou 静态资源的编辑器

## FusouMapForge

涉及到 FusouMapForge 相关修改，即 `FusouMapForge/` 目录下的内容时，必须阅读 `FusouMapForge/AGENTS.md` 文件。至少应当遵守以下约束：

- 将 `FusouMapForge/` 视为原生 HTML/CSS/JavaScript 项目
- 严禁将 C++ 相关/ CMake 相关/ MSVC 相关或平台架构规则应用于此目录
- 仓库根目录下的 `JsonSchema.json` 将严格约束 FusouMapForge 项目导出、导入以及 palette 配置

## 架构边界

- `Core` 不得直接包含 `Windows.h`
- `Core` 不得直接包含 `conioex.h`
- `Core` 不得依赖 `hew_console`
- `Platform` 可以依赖 Windows API、conioex 和 hew_console
- `App` 负责把 `Core` 与 `Platform` 的具体实现组装起来
- 不要把平台相关代码移动到 `Core`
- 不要为了小范围 bug 修复进行跨层重构
- C++ 相关 coding 作业必须阅读并遵守 `.agents/skills/C++CodingStandards/SKILL.md`
- FusouMapForge 相关 JavaScript 作业必须阅读并遵守 `FusouMapForge/.agents/skills/coding-standards/SKILL.md`

## 资源规则

- JSON 资源格式以 `JsonSchema.json` 为准
- 除非任务明确要求修改资源格式，否则不得修改 `JsonSchema.json`
- 除非任务明确要求，否则不得重命名资源文件；如确需重命名，必须同步更新 CMake 资源拷贝配置

## 禁止事项

- 不要引入跨平台相关内容
- 不要添加可选功能
- 不要修改第三方库源码，除非任务明确要求
- 不要提交本地绝对路径、个人信息、机器名或 IDE 私有配置

## 执行方针

- 优先使用最小改动完成任务，非必要不要新增文件、移动文件或调整现有目录结构
- 若需求存在关键歧义，且不同实现路线会显著影响结果、程序行为或改动范围，应在开始施工之前说明差异并请求确认；否则优先按最合理且最小改动的方式执行
- 若需求本身存在冲突、无法实现、前提不足，或执行后很可能引入明显问题，应先明确指出，而不是强行实现
- 如果存在多种实现路线，且它们在改动范围、维护成本或行为结果上差异明显，应先简要说明差异，再由我决定
- 在施工结束后，如果你认为有明显更优方案，可以以建议的形式提出，由我判断要不要执行
- C++ 相关作业以根目录作为主要工作目录；JavaScript 相关作业，以 `FusouMapForge/` 目录为主要工作目录
- 任何检查、修改、新增、删减都不得包含 `.agents` 目录和目录内任何内容

## 修改前检查

开始修改前，先判断本次任务影响范围，检查点如下：

- 是否影响 CMake
- 是否影响 Assets
- 是否影响 `Core` 与 `Platform` 的边界
- 是否影响 Dev 或 Shipping 行为

如果任务存在重大歧义，先停止并说明需要用户决策的点，不要自行扩大范围。

## 验证规则

默认验证目标是 Dev 构建

- 仅修改文档、注释或不影响程序行为的说明文本时，可以不执行构建，但完成时必须说明未执行验证的原因
- 修改 C++ 源码、头文件或资源加载逻辑时，至少执行 Dev 构建
- 修改 `CMakeLists.txt`、构建选项、目标列表、资源拷贝规则或代码生成规则时，必须重新配置 Dev 构建目录并执行 Dev 构建
- 修改 `Assets`、`JsonSchema.json`、palette 资源或资源生成脚本时，至少执行 Dev 构建，并说明是否影响资源拷贝、代码生成或 FusouMapForge
- 修改 `FusouMapForge/` 时，应遵守 `FusouMapForge/AGENTS.md` 的验证要求；除非同时影响 C++ 游戏本体、根目录 CMake 或共享资源格式，否则不需要执行 C++ 构建
- 如果因本地环境缺失、工具链不可用或任务范围限制导致无法验证，必须说明具体原因，不得声称已验证

## 完成标准

完成后必须说明：

- 修改了哪些文件
- 行为发生了什么变化
- 是否修改了 CMake
- 是否修改了 Assets
- 是否影响 Dev 或 Shipping
- 执行了哪些构建或验证命令
- 如果没有执行验证，说明原因
