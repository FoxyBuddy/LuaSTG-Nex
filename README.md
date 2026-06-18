# LuaSTG Nexus  

[![C++ CI](https://github.com/Legacy-LuaSTG-Engine/LuaSTG-Sub/actions/workflows/cpp-ci.yml/badge.svg)](https://github.com/Legacy-LuaSTG-Engine/LuaSTG-Sub/actions/workflows/cpp-ci.yml)

---

## 介绍

LuaSTG 救赎计划的引擎部分——LuaSTG Nexus，该分支是[LuaSTG Sub](https://github.com/Legacy-LuaSTG-Engine/LuaSTG-Sub/)的下一代引擎分支。

为了解决存续长达十余年的屎山和历史遗留问题，该分支规划从零开始使用SDL3下的GPU API参考先前版本的引擎实现完全重写渲染架构，绕过DirectX原生代码来实现新型渲染封装和自定义渲染管线、材质渲染等新功能。~~你可以理解成国产的LuaSTG Flux但更先进~~

引擎将参考使用Lua编写的2D游戏平台引擎，例如[LÖVE（或称作Love2d）](https://github.com/love2d/love)和[Amara2](https://github.com/BigBossErndog/Amara2)。为了抛弃老掉牙的架构，新引擎默认只支持amd64（x64）架构和DirectX 12，远期规划为增设部分跨平台需求。

其他的LuaSTG分支收录在 [Legacy LuaSTG Engine 组织首页](https://github.com/Legacy-LuaSTG-Engine)。

⚠️ **警告：极度实验性 & 快速迭代中** ⚠️

目前正在招收：
 - 2~3名图形学编程经验强且空闲时间多的大佬。
 - 1\~2名拥有较强项目管理能力且能够在未来3\~5年内保持一定强度的维护的大佬。

新社群为了防止对原社群产生较大冲击或造成争议，群聊暂不大面积开放。如果有对新引擎提出想法可私聊邮箱：721597824@qq.com、foxylikesraccoon@gmail.com询问或协助进入新社群。为了不发生大规模的摩擦，新社群与原社群将不会有太多的重合。新社群群规相对更加严苛且思想更加发散，目的是防止重蹈覆辙原社群的问题。

## 下载  

由于Nexus所在的LuaSTG架构分支：LuaSTG Aether还在开发阶段且THlib也近乎完全重写架构，请敬请期待公测！移步至 [LuaSTG-FutureData](https://github.com/phsonh/LuaSTG-Nex) 查看更多！

> THlib：一套东方原作风格的脚本和素材库，包含关卡背景库、自机库、子弹库、符卡系统、关卡组、replay 系统等，可能被更多人更熟悉的是“东方弹幕祀典”  

> Thlib的素材未来将重绘成无版权高清素材，我们期待美工大佬的加入！THlib 目前含有东方 Project 官方作品的素材，以及一些来源和版权不明的素材，请不要在私坑中使用。（为了照顾原作党的手感我们也保留了原作素材插件）

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

LuaSTG Sub由于社群在2025年下半年发生数次大规模震动，多个大佬退出社群，且社群内部已经是处于极端高熵状态，无法再进行系统性的维护。在2026年3月中旬，Sub分支除了[璀境石](https://github.com/Demonese)依然进行极少量的更新不再进行任何维护，社群内部已经是一潭死水，为了对社群续命我们启动了Nexus项目。

Nexus项目社群的平均年龄只有20.2岁，主要开发成员的平均年龄只有20岁，所以在可预见的未来内会有大量的更新。

## 主要贡献者  

- 所有前辈们：隔壁的桌子（原型）、[9chu](https://github.com/9chu)（Plus）、 [ESC](https://github.com/ExboCooope)&[Xiliusha](https://github.com/Xiliusha)（Ex Plus）、[璀境石](https://github.com/Demonese)（Sub）
- 新架构领导人：[穿球鞋的狐狸哥](https://github.com/FoxyBuddy)
- 所有编写者或提供贡献的人：[eva](https://github.com/1492083648)、[phsonh](https://github.com/phsonh)、[Qerfcxz](https://github.com/Qerfcxz)、[鱼灰](https://github.com/yuhuifishash)、凌镜IceCraft（暂无账号）、B07（暂无账号）等
- ……

---

## Introduce

The engine component of the LuaSTG Redemption Project—LuaSTG Nexus, the next generation engine branch of [LuaSTG Sub](https://github.com/Legacy-LuaSTG-Engine/LuaSTG-Sub/).

To resolve the ‘mountain of shit code’ and legacy issues that have persisted for over a decade, this branch plans to completely rewrite the rendering architecture from scratch using the GPU API under SDL3, drawing on the implementation of previous engine versions. It will bypass the native DirectX code to implement a new rendering wrapper and new features such as a custom rendering pipeline and material rendering. ~~You could think of it as a Chinese version of LuaSTG Flux, but more advanced~~

The engine will get inspiration from 2D game platform engines written in Lua, such as [LÖVE (also known as Love2d)](https://github.com/love2d/love) and [Amara2](https://github.com/BigBossErndog/Amara2). In order to move away from outdated architectures, the new engine will, by default, only support the amd64 (x64) architecture and DirectX 12, with plans to add support for certain cross-platform requirements in the long term.

Other LuaSTG branches are hosted on the [Legacy LuaSTG Engine organisation homepage](https://github.com/Legacy-LuaSTG-Engine).

⚠️ **Warning: Highly experimental & undergoing rapid iteration** ⚠️

We are currently recruiting: 
- 2–3 experts with strong experience in graphics programming and plenty of spare time. 
- 1–2 experts with strong project management skills who can maintain a consistent level of activity over the next 3–5 years.

To avoid causing significant disruption or controversy within the original community, the new group chat will not be widely opened for the time being. If you have any ideas for the new engine, please send a private message to the following email addresses: 721597824@qq.com or foxylikesraccoon@gmail.com to consult or help to join the new community. To avoid large-scale friction, there will be minimal overlap between the new community and the original one. The new community’s rules will be relatively stricter and its approach more open-minded, with the aim of preventing a repeat of the issues encountered in the original community.

## Download

As Nexus (and Aether branch) is still in development and has it rewritten its codebase, please stay tuned for the open beta! Head over to [LuaSTG-FutureData](https://github.com/FoxyBuddy/LuaSTG-Sub-FutureData) to find out more!

> THlib: A set of official Touhou Project games style scripts and resources libraries, including stage backgrounds library, player library, bullet library, SpellCard system, stage group, replay system, etc., which may be more familiar to more people is "东方弹幕祀典"

> The assets in THlib are set to undergo a major overhaul in the future, and we look forward to welcoming talented artists to join us! THlib contains the resources from the official Touhou Project games, as well as some resources with unknown sources and copyrights. Please do not use this in private projects.(To cater to the preferences of fans of the original Touhou games, we have also retained the original asset plugin.)

## Engine core utility

* Graphic API: Direct3D 11 -> SDL3 + Direct3D 12
* Audio API: XAudio2 -> SDL3 + XAudio2 12

## Requirements

* Operating System: Windows 10 1809 or above
* Graphics Card: compatible with Direct3D12
* Audio Card: compatible with XAudio2

## Build Projects

See [Build Projects](./BUILD.md).

## Project and Maintainer Status

### Project

Due to several major upheavals within the LuaSTG Sub community in the second half of 2025, a number of key figures left the community, and the community itself had already reached a state of extreme entropy, making systematic maintenance no longer feasible. By mid-March 2026, apart from [璀境石](https://github.com/Demonese), which continued to receive a minimal number of updates, the Sub branch was no longer being maintained at all, and the community had become a stagnant pool. In order to keep the community alive, we launched the Nexus project.

The average age of the Nexus project community is just 20.2, whilst the average age of the core development team is only 20, so we can expect a great many updates in the foreseeable future.

## Major Contributors

- All our seniors：隔壁的桌子（Prototype）、[9chu](https://github.com/9chu)（Plus）、 [ESC](https://github.com/ExboCooope)&[Xiliusha](https://github.com/Xiliusha)（Ex Plus）、[璀境石](https://github.com/Demonese)（Sub）
- Leaders of the New Architecture：[穿球鞋的狐狸哥](https://github.com/FoxyBuddy)
- All authors and contributors：[eva](https://github.com/1492083648)、[phsonh](https://github.com/phsonh)、[Qerfcxz](https://github.com/Qerfcxz)、[鱼灰](https://github.com/yuhuifishash)、凌镜IceCraft（No account yet）、B07（No account yet）, etc.
- ...
