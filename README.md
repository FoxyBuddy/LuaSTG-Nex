# LuaSTG Nexus  

[![C++ CI](https://github.com/Legacy-LuaSTG-Engine/LuaSTG-Sub/actions/workflows/cpp-ci.yml/badge.svg)](https://github.com/Legacy-LuaSTG-Engine/LuaSTG-Sub/actions/workflows/cpp-ci.yml)

---

## 介绍  

LuaSTG Nexus 分支是 [LuaSTG Sub](https://github.com/Legacy-LuaSTG-Engine/LuaSTG-Sub/)  的下一代引擎分支，目标是继承LuaSTG Sub的优良体系并解决当前引擎的历史遗留问题。

Nexus分支打算从零开始，使用SDL3下的GPU API重写渲染架构，绕过DirectX原生代码来实现新型渲染封装和自定义渲染管线、材质渲染等新功能。~~你可以理解成国产的LuaSTG Flux但更先进~~

引擎将默认只支持amd64（x64）架构，默认渲染后端为DirectX 12，远期规划为支持部分跨平台需求。

其他分支收录在 [Legacy LuaSTG Engine 组织首页](https://github.com/Legacy-LuaSTG-Engine)。

⚠️ **警告：极度实验性 & 快速迭代中** ⚠️

目前正在招收图形学编程经验强且空闲时间多的大佬。

## 下载  

由于Nexus所在的LuaSTG分支：LuaSTG Aether还在开发阶段且近乎完全重写架构，请敬请期待公测！移步至 [LuaSTG-FutureData](https://github.com/phsonh/LuaSTG-Nex) 查看更多！

> THlib：一套东方原作风格的脚本和素材库，包含关卡背景库、自机库、子弹库、符卡系统、关卡组、replay 系统等，可能被更多人更熟悉的是“东方弹幕祀典”  

> Thlib的素材未来将重绘成无版权高清素材，我们期待美工大佬的加入！THlib 目前含有东方 Project 官方作品的素材，以及一些来源和版权不明的素材，请不要在私坑中使用。

## 引擎主驱动库  

* 图形 API：Direct3D 11 -> SDL3 + Direct3D 12
* 音频 API：XAudio2 -> SDL3 + XAudio2

## 配置要求  

* 系统要求：Windows 10 1809 或以上
* 显卡需求：支持 Direct3D 12
* 声卡需求：支持 XAudio2  

## 编译项目  

请阅读[编译项目](./BUILD.md)。

## 项目和维护者状态  

### 项目

Sub曾在2026年3月进入近两个月的无人维护期，在一些开发者的要求和新Data翻新需求下，我们启动了Nexus项目。

Nexus项目社群的平均年龄只有20.2岁，主要开发成员的平均年龄只有20岁，所以在可预见的未来内会有大量的更新。

## 主要贡献者  

- 隔壁的桌子（LuaSTG 开发者）  
- [9chu](https://github.com/9chu)（LuaSTG Plus 开发者）  
- [ESC](https://github.com/ExboCooope)（LuaSTG Ex Plus 开发者）  
- [Xiliusha](https://github.com/Xiliusha)（LuaSTG Ex Plus 开发者）  
- [璀境石](https://github.com/Demonese)（LuaSTG Sub 开发者）
- [穿球鞋的狐狸哥](https://github.com/FoxyBuddy)（LuaSTG Nexus 开发者）
- [eva](https://github.com/1492083648)（LuaSTG Nexus 开发者）
- [phsonh](https://github.com/phsonh)（LuaSTG Nexus 开发者）
- [鱼灰]（LuaSTG Nexus 开发者）

---

## Introduce

LuaSTG Nexus, is the next generation engine branch of [LuaSTG Sub](https://github.com/Legacy-LuaSTG-Engine/LuaSTG-Sub/), aim to inherit excellent framework of LuaSTG Sub and resolve historical issues in the current engine.

The Nexus branch aims to start from scratch and rewrite the rendering architecture using the GPU API in SDL3, bypassing the native DirectX to implement a new rendering wrapper and new features such as a custom rendering pipeline and material rendering. ~~You could treat as Chinese version for LuaSTG Flux but more advanced~~

The engine will only support the amd64 (x64) architecture, with DirectX 12 as the default rendering backend by default; long-term plans include support for certain cross-platform requirements.

Other branches are listed on the homepage of the [Legacy LuaSTG Engine organization](https://github.com/Legacy-LuaSTG-Engine).

⚠️ **Warning: Highly experimental & undergoing rapid iteration & AIGC + Vibe programming in progress** ⚠️

LuaSTG Nexus will utilize Vibe Coding (aka AI Programming) to increase productivity, to prevent unforseen bugs from AI generating codes, the team will reserve some mamual productivity for verification and debugging in traditional programming methods. We are currently waiting experienced developers excel in graphic programming and having plenty of time.

## Download

As Nexus (and Aether branch) is still in development and has it rewritten its codebase, please stay tuned for the open beta! Head over to [LuaSTG-FutureData](https://github.com/FoxyBuddy/LuaSTG-Sub-FutureData) to find out more!

> THlib: A set of official Touhou Project games style scripts and resources libraries, including stage backgrounds library, player library, bullet library, SpellCard system, stage group, replay system, etc., which may be more familiar to more people is "东方弹幕祀典"

> The assets in THlib are set to undergo a major overhaul in the future, and we look forward to welcoming talented artists to join us! THlib contains the resources from the official Touhou Project games, as well as some resources with unknown sources and copyrights. Please do not use this in private projects.

## Engine core utility

* Graphic API: Direct3D 11 -> SDL3 + Direct3D 12
* Audio API: XAudio2 -> SDL3 + XAudio2 12

## Requirements

* Operating System: Windows 10 1909 or above
* Graphics Card: compatible with Direct3D12
* Audio Card: compatible with XAudio2

## Build Projects

See [Build Projects](./BUILD.md).

## Project and Maintainer Status

### Project

Sub entered a period of nearly two months without maintenance in March 2026; in response to requests from some developers and the need for a new data overhaul, we launched the Nexus project.

The average age of the Nexus project community is just 20.2, whilst the average age of the core development team is only 20, so we can expect a great many updates in the foreseeable future.

## Major Contributors

- 隔壁的桌子 (developer of vanilla LuaSTG)  
- [9chu](https://github.com/9chu) (developer of LuaSTG Plus)  
- [ESC](https://github.com/ExboCooope) (developer of LuaSTG Ex Plus)  
- [Xiliusha](https://github.com/Xiliusha) (developer of LuaSTG Ex Plus)  
- [璀境石](https://github.com/Demonese) (developer of LuaSTG Sub)
- [穿球鞋的狐狸哥](https://github.com/FoxyBuddy) (developer of LuaSTG Nexus)
- [eva](https://github.com/1492083648) (developer of LuaSTG Nexus)
- [phsonh](https://github.com/phsonh) (developer of LuaSTG Nexus)
- [鱼灰] (developer of LuaSTG Nexus)
