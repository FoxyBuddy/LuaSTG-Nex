# LuaSTG Nexus 更新历史记录
* LuaSTG Nexus v0.1.2
	* 游戏对象
		* 移除：实验性移除游戏对象的render回调属性和用于渲染的对象属性（By phsonh）
	* 图形/渲染
		* 新增：新增RenderEx函数，专门负责执行渲染操作，游戏内逻辑对象只进行运动学操作（By phsonh）
* LuaSTG Nexus v0.1.1
	* 游戏对象
		* 移除：清理不再使用的`maxvy`、`maxvx`、`maxv`、`ag`属性（By Foxy）
* LuaSTG Nexus v0.1.0
	* 游戏对象
		* 移除：移除了游戏对象用于编辑器内调色的`_blend`、`_a`、`_r`、`_g`、`_b`属性和`maxvy`、`maxvx`、`maxv`、`ag`运动属性（By Foxy）
		* 修改：`omiga`改名为`omega`，`omiga`从引擎中移除，结束长达十余年的命名错误问题（By Foxy）
		* 修改：拆除LoadImage、LoadImageGroup、LoadAnimation的对象判定属性，贴图与判定彻底解耦（By Foxy）
	* 适配性：从此版本引擎开始破坏性更新较多，请等待新Data的到来
* LuaSTG Nexus v0.0.1
	* 引擎基础：
		* 由于Sub已无人维护，Nexus项目成立，继承LuaSTG Sub代码库，继承版本为Sub v0.21.129
    * 图形/渲染
        * 新增：新增 `lstg.GetImageBlend` 方法用于获取图片精灵资源的混合模式
        * 新增：新增 `lstg.GetImageColor` 方法用于获取图片精灵资源的顶点颜色