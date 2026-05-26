# AGENTS.md

## 项目概要

FusouMapForge 是一个小型原生前端项目，运行在浏览器环境中。为 Fusou 项目提供可视化的 `.json` 资产创建和编辑。
本文件仅适用于 FusouMapForge 子目录内的文件，不是主 C++ 游戏项目，也不适用 C++相关的 CMake、MSVC、Dev/Shipping 构建规则。
项目使用 HTML、CSS 和 JavaScript 实现，不依赖 React、Vue 等前端框架。以静态页面方式组织，入口文件为 `index.html`。
JavaScript 代码通过页面直接加载执行，不依赖打包构建流程。项目维持当前静态页面直接运行方式，不额外要求新的启动流程。

## 开发环境

- 操作系统：Windows x64

## 仓库结构

- `index.html` 静态页面入口
- `app.css` 页面样式
- `app.js` 负责主要启动流程
- `state.js` 负责编辑器状态
- `schema.js` 负责资源格式和 schema 相关逻辑
- `io.js` 负责导入、导出和文件读写相关逻辑
- `helpers/` 存放通用辅助能力
- `ui/` 存放界面和 DOM 相关逻辑
- `features/` 存放具体功能实现与行为组织
- `generated/` 存放生成文件或由外部流程同步的文件

## 架构边界

- 除非任务明确要求，否则不得破坏现有运行方式、初始化流程和已存在页面行为
- 未经明确要求，不要引入新的框架、构建工具、第三方依赖或工程化改造
- JavaScript 相关 coding 作业必须阅读并遵守 `.agents/skills/coding-standards/SKILL.md`
- 该项目所导出的 `.json` 资产将被放置在 `Fusou\Assets` 目录下

## 资源与 Schema 规则

- 导入和导出的 JSON 必须符合仓库根目录 `JsonSchema.json`
- 除非任务明确要求修改资源格式，否则不得修改导出 JSON 的顶层结构、字段名或字段语义
- 修改导入、导出、palette 或 schema 相关逻辑时，必须说明是否影响 Fusou 游戏本体读取 `Assets` 中的资源
- 不得为了让 FusouMapForge 的实现更方便而放宽 `JsonSchema.json` 约束

## 完成标准与报告

完成后必须说明：

- 修改了哪些文件
- 行为发生了什么变化
- 是否影响导入、导出、palette、schema 或文件命名
- 是否修改或依赖根目录 `JsonSchema.json`
- 是否新增依赖、框架、构建工具或启动流程
- 执行了哪些浏览器运行检查或手动验证
- 如果没有执行验证，说明原因
