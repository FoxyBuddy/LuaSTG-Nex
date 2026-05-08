#include "LuaBinding/LuaWrapper.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"
#include <algorithm>
#include <cstring>
#include <string>
#include <unordered_map>

void luastg::binding::ResourceManager::Register(lua_State* L) noexcept
{
	struct Wrapper
	{
		static std::unordered_map<std::string, float>& NexSoundLevels() noexcept {
			static std::unordered_map<std::string, float> levels;
			return levels;
		}
		static std::unordered_map<std::string, float>& NexMusicLevels() noexcept {
			static std::unordered_map<std::string, float> levels;
			return levels;
		}
		static std::unordered_map<std::string, std::string>& NexTextCache() noexcept {
			static std::unordered_map<std::string, std::string> texts;
			return texts;
		}
		static float CheckLevel(lua_State* L, int index, float default_value, char const* api_name) {
			float const level = static_cast<float>(luaL_optnumber(L, index, default_value));
			if (level < 0.0f || level > 1.0f)
				luaL_error(L, "%s: level must be in range [0, 1].", api_name);
			return level;
		}
		static int SetResLoadInfo(lua_State* L) noexcept {
			ResourceMgr::SetResourceLoadingLog((bool)lua_toboolean(L, 1));
			return 0;
		}
		static int SetResourceStatus(lua_State* L) noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			if (strcmp(s, "global") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::Global);
			else if (strcmp(s, "stage") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::Stage);
			else if (strcmp(s, "none") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::None);
			else
				return luaL_error(L, "invalid argument #1 for 'SetResourceStatus', requires 'stage', 'global' or 'none'.");
			return 0;
		}
		static int GetResourceStatus(lua_State* L) noexcept
		{
			switch (LRES.GetActivedPoolType()) {
			case ResourcePoolType::Global:
				lua_pushstring(L, "global");
				break;
			case ResourcePoolType::Stage:
				lua_pushstring(L, "stage");
				break;
			case ResourcePoolType::None:
				lua_pushstring(L, "none");
				break;
			default:
				return luaL_error(L, "can't get resource pool status at this time.");
			}
			return 1;
		}
		static int LoadTexture(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			if (!pActivedPool->LoadTexture(name, path, lua_toboolean(L, 3) == 0 ? false : true))
				return luaL_error(L, "can't load texture from file '%s'.", path);
			return 0;
		}
		static int LoadSprite(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* texname = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->CreateSprite(
				name,
				texname,
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4),
				luaL_checknumber(L, 5),
				luaL_checknumber(L, 6)
			))
			{
				return luaL_error(L, "load image failed (name='%s', tex='%s').", name, texname);
			}
			return 0;
		}
		static int CopySprite(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* src_name = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->CopySprite(name, src_name))
			{
				return luaL_error(L, "copy image failed (name='%s', src='%s').", name, src_name);
			}
			return 0;
		}
		static int LoadAnimation(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (lua_istable(L, 2)) {
				std::vector<core::SmartReference<IResourceSprite>> sprites;
				sprites.reserve(lua_objlen(L, 2));
				for (int i = 1; i <= static_cast<int>(lua_objlen(L, 2)); i += 1) {
					lua_pushinteger(L, i);
					lua_gettable(L, 2);
					char const* sprite_name = luaL_checkstring(L, -1);
					auto sprite = LRES.FindSprite(sprite_name);
					if (!sprite)
						return luaL_error(L, "load animation failed (name='%s'), sprite '%s' not found", name, sprite_name);
					sprites.push_back(sprite);
					lua_pop(L, 1);
				}
				if (!pActivedPool->CreateAnimation(
					name,
					sprites,
					luaL_checkinteger(L, 3)
				)) {
					return luaL_error(L, "load animation failed (name='%s').", name);
				}
			}
			else {
				const char* texname = luaL_checkstring(L, 2);
				if (!pActivedPool->CreateAnimation(
					name,
					texname,
					luaL_checknumber(L, 3),
					luaL_checknumber(L, 4),
					luaL_checknumber(L, 5),
					luaL_checknumber(L, 6),
					(int)luaL_checkinteger(L, 7),
					(int)luaL_checkinteger(L, 8),
					(int)luaL_checkinteger(L, 9)
				)) {
					return luaL_error(L, "load animation failed (name='%s', tex='%s').", name, texname);
				}
			}
			
			return 0;
		}
		static int NexImageLoadTexture(lua_State* L) noexcept
		{
			if (lua_gettop(L) != 2)
				return luaL_error(L, "Resource.Image.LoadTexture(name, path) expected 2 arguments.");

			char const* name = luaL_checkstring(L, 1);
			char const* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadTexture(name, path, false))
				return luaL_error(L, "Resource.Image.LoadTexture: can't load texture from file '%s'.", path);

			return 0;
		}

		static int NexImageLoadSprite(lua_State* L) noexcept
		{
			int const argc = lua_gettop(L);
			if (argc != 6 && argc != 8)
				return luaL_error(L, "Resource.Image.LoadSprite(name, texture, x, y, w, h[, scale_x, scale_y]) expected 6 or 8 arguments.");

			char const* name = luaL_checkstring(L, 1);
			char const* texname = luaL_checkstring(L, 2);
			double const x = luaL_checknumber(L, 3);
			double const y = luaL_checknumber(L, 4);
			double const w = luaL_checknumber(L, 5);
			double const h = luaL_checknumber(L, 6);
			double const scale_x = argc >= 8 ? luaL_checknumber(L, 7) : 1.0;
			double const scale_y = argc >= 8 ? luaL_checknumber(L, 8) : 1.0;

			if (w <= 0.0 || h <= 0.0)
				return luaL_error(L, "Resource.Image.LoadSprite: width and height must be positive.");
			if (scale_x == 0.0 || scale_y == 0.0)
				return luaL_error(L, "Resource.Image.LoadSprite: scale_x and scale_y must be non-zero.");

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->CreateSprite(name, texname, x, y, w, h))
				return luaL_error(L, "Resource.Image.LoadSprite: failed (name='%s', texture='%s').", name, texname);

			core::SmartReference<IResourceSprite> sprite = LRES.FindSprite(name);
			if (!sprite)
				return luaL_error(L, "Resource.Image.LoadSprite: sprite '%s' was created but cannot be found.", name);
			sprite->SetScale(scale_x, scale_y);

			return 0;
		}

		static int NexImageLoadFullSprite(lua_State* L) noexcept
		{
			int const argc = lua_gettop(L);
			if (argc != 2 && argc != 4)
				return luaL_error(L, "Resource.Image.LoadFullSprite(name, texture[, scale_x, scale_y]) expected 2 or 4 arguments.");

			char const* name = luaL_checkstring(L, 1);
			char const* texname = luaL_checkstring(L, 2);
			double const scale_x = argc >= 4 ? luaL_checknumber(L, 3) : 1.0;
			double const scale_y = argc >= 4 ? luaL_checknumber(L, 4) : 1.0;

			core::Vector2U size;
			if (!LRES.GetTextureSize(texname, size))
				return luaL_error(L, "Resource.Image.LoadFullSprite: texture '%s' not found.", texname);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->CreateSprite(name, texname, 0.0, 0.0, static_cast<double>(size.x), static_cast<double>(size.y)))
				return luaL_error(L, "Resource.Image.LoadFullSprite: failed (name='%s', texture='%s').", name, texname);

			core::SmartReference<IResourceSprite> sprite = LRES.FindSprite(name);
			if (!sprite)
				return luaL_error(L, "Resource.Image.LoadFullSprite: sprite '%s' was created but cannot be found.", name);
			sprite->SetScale(scale_x, scale_y);

			return 0;
		}

		static int NexImageGetTextureSize(lua_State* L) noexcept
		{
			return GetTextureSize(L);
		}

		static int NexImageGetSpriteSize(lua_State* L) noexcept
		{
			return GetImageSize(L);
		}

		static int NexImageGetSpriteScale(lua_State* L) noexcept
		{
			core::SmartReference<IResourceSprite> sprite = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!sprite)
				return luaL_error(L, "sprite '%s' not found.", luaL_checkstring(L, 1));
			lua_pushnumber(L, sprite->GetScaleX());
			lua_pushnumber(L, sprite->GetScaleY());
			return 2;
		}

		static int NexAudioLoadSound(lua_State* L) noexcept
		{
			int const argc = lua_gettop(L);
			if (argc != 2 && argc != 3)
				return luaL_error(L, "Resource.Audio.LoadSound(name, path[, level]) expected 2 or 3 arguments.");

			char const* name = luaL_checkstring(L, 1);
			char const* path = luaL_checkstring(L, 2);
			float const level = CheckLevel(L, 3, 1.0f, "Resource.Audio.LoadSound");

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadSoundEffect(name, path))
				return luaL_error(L, "Resource.Audio.LoadSound: failed (name='%s', path='%s').", name, path);

			NexSoundLevels()[name] = level;
			return 0;
		}

		static int NexAudioLoadMusic(lua_State* L) noexcept
		{
			int const argc = lua_gettop(L);
			if (argc != 2 && argc != 3 && argc != 5)
				return luaL_error(L, "Resource.Audio.LoadMusic(name, path[, level[, loop_start, loop_end]]) expected 2, 3, or 5 arguments.");

			char const* name = luaL_checkstring(L, 1);
			char const* path = luaL_checkstring(L, 2);
			float const level = CheckLevel(L, 3, 1.0f, "Resource.Audio.LoadMusic");
			double const loop_start = argc >= 5 ? luaL_checknumber(L, 4) : 0.0;
			double const loop_end = argc >= 5 ? luaL_checknumber(L, 5) : 0.0;

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadMusic(name, path, loop_start, loop_end, false))
				return luaL_error(L, "Resource.Audio.LoadMusic: failed (name='%s', path='%s', loop=%f~%f).", name, path, loop_start, loop_end);

			NexMusicLevels()[name] = level;
			return 0;
		}

		static int NexAudioPlaySound(lua_State* L) noexcept
		{
			int const argc = lua_gettop(L);
			if (argc != 1 && argc != 2)
				return luaL_error(L, "Resource.Audio.PlaySound(name[, level]) expected 1 or 2 arguments.");

			char const* name = luaL_checkstring(L, 1);
			float const level = CheckLevel(L, 2, 1.0f, "Resource.Audio.PlaySound");

			core::SmartReference<IResourceSoundEffect> sound = LRES.FindSound(name);
			if (!sound)
				return luaL_error(L, "sound '%s' not found.", name);

			auto const it = NexSoundLevels().find(name);
			float const base_level = it == NexSoundLevels().end() ? 1.0f : it->second;
			sound->Play(base_level * level, 0.0f);
			return 0;
		}

		static int NexAudioPlayMusic(lua_State* L) noexcept
		{
			int const argc = lua_gettop(L);
			if (argc != 1 && argc != 2)
				return luaL_error(L, "Resource.Audio.PlayMusic(name[, level]) expected 1 or 2 arguments.");

			char const* name = luaL_checkstring(L, 1);
			float const level = CheckLevel(L, 2, 1.0f, "Resource.Audio.PlayMusic");

			core::SmartReference<IResourceMusic> music = LRES.FindMusic(name);
			if (!music)
				return luaL_error(L, "music '%s' not found.", name);

			auto const it = NexMusicLevels().find(name);
			float const base_level = it == NexMusicLevels().end() ? 1.0f : it->second;
			music->Play(base_level * level, 0.0);
			return 0;
		}

		static int NexFileLoadText(lua_State* L) noexcept
		{
			if (lua_gettop(L) != 2)
				return luaL_error(L, "Resource.File.LoadText(name, path) expected 2 arguments.");

			char const* name = luaL_checkstring(L, 1);
			char const* path = luaL_checkstring(L, 2);

			core::SmartReference<core::IData> data;
			if (!core::FileSystemManager::readFile(path, data.put()))
				return luaL_error(L, "Resource.File.LoadText: cannot read file '%s'.", path);

			std::string text(static_cast<char const*>(data->data()), data->size());
			NexTextCache()[name] = text;
			lua_pushlstring(L, text.data(), text.size());
			return 1;
		}

		static int NexFileGetText(lua_State* L) noexcept
		{
			if (lua_gettop(L) != 1)
				return luaL_error(L, "Resource.File.GetText(name) expected 1 argument.");

			char const* name = luaL_checkstring(L, 1);
			auto const it = NexTextCache().find(name);
			if (it == NexTextCache().end()) {
				lua_pushnil(L);
				return 1;
			}

			lua_pushlstring(L, it->second.data(), it->second.size());
			return 1;
		}

		static int NexFileWriteText(lua_State* L) noexcept
		{
			if (lua_gettop(L) != 2)
				return luaL_error(L, "Resource.File.WriteText(path, text) expected 2 arguments.");

			char const* path = luaL_checkstring(L, 1);
			size_t length = 0;
			char const* text = luaL_checklstring(L, 2, &length);

			core::SmartReference<core::IData> data;
			if (!core::IData::create(length, data.put()))
				return luaL_error(L, "Resource.File.WriteText: cannot allocate buffer.");
			if (length > 0)
				std::memcpy(data->data(), text, length);

			bool const ok = core::FileSystemManager::writeFile(path, data.get());
			lua_pushboolean(L, ok ? 1 : 0);
			return 1;
		}

		static int NexEffectLoadParticle(lua_State* L) noexcept
		{
			return LoadPS(L);
		}

		static int LoadPS(lua_State* L) noexcept
		{
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			
			const char* name = luaL_checkstring(L, 1);
			const char* img_name = luaL_checkstring(L, 3);
			if (lua_type(L, 2) == LUA_TTABLE) {
				hgeParticleSystemInfo info;
				bool ret = TranslateTableToParticleInfo(L, 2, info);
				if (!ret) return luaL_error(L, "load particle failed (name='%s', define=?, img='%s').", name, img_name);
				if (!pActivedPool->LoadParticle(
					name,
					info,
					img_name,
					luaL_optnumber(L, 4, 0.0f),
					luaL_optnumber(L, 5, 0.0f),
					lua_toboolean(L, 6) == 0 ? false : true
				))
				{
					return luaL_error(L, "load particle failed (name='%s', define=table, img='%s').", name, img_name);
				}
				return 0;
			}
			else {
				const char* path = luaL_checkstring(L, 2);

				if (!pActivedPool->LoadParticle(
					name,
					path,
					img_name,
					luaL_optnumber(L, 4, 0.0f),
					luaL_optnumber(L, 5, 0.0f),
					lua_toboolean(L, 6) == 0 ? false : true
				))
				{
					return luaL_error(L, "load particle failed (name='%s', file='%s', img='%s').", name, path, img_name);
				}
				return 0;
			}
		}
		static int LoadSound(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadSoundEffect(name, path))
				return luaL_error(L, "load sound failed (name=%s, path=%s)", name, path);
			return 0;
		}
		static int LoadMusic(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			double loop_end = luaL_checknumber(L, 3);
			double loop_duration = luaL_checknumber(L, 4);
			double loop_start = std::max(0., loop_end - loop_duration);

			if (!pActivedPool->LoadMusic(
				name,
				path,
				loop_start,
				loop_end,
				(lua_gettop(L) >= 5) ? lua_toboolean(L, 5) : false
				))
			{
				return luaL_error(L, "load music failed (name=%s, path=%s, loop=%f~%f)", name, path, loop_start, loop_end);
			}
			return 0;
		}
		static int LoadFont(lua_State* L) noexcept
		{
			bool bSucceed = false;
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (lua_gettop(L) == 2)
			{
				// HGE字体 mipmap=true
				bSucceed = pActivedPool->LoadSpriteFont(name, path);
			}
			else
			{
				if (lua_isboolean(L, 3))
				{
					// HGE字体 mipmap=user_defined
					bSucceed = pActivedPool->LoadSpriteFont(name, path, lua_toboolean(L, 3) == 0 ? false : true);
				}
				else
				{
					// fancy2d字体
					const char* texpath = luaL_checkstring(L, 3);
					if (lua_gettop(L) == 4)
						bSucceed = pActivedPool->LoadSpriteFont(name, path, texpath, lua_toboolean(L, 4) == 0 ? false : true);
					else
						bSucceed = pActivedPool->LoadSpriteFont(name, path, texpath);
				}
			}

			if (!bSucceed)
				return luaL_error(L, "can't load font from file '%s'.", path);
			return 0;
		}
		static int LoadTTF(lua_State* L) noexcept
		{
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool) {
				return luaL_error(L, "can't load resource at this time.");
			}
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);
			bool result = pActivedPool->LoadTTFFont(name, path, (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4));
			lua_pushboolean(L, result);
			return 1;
		}
		static int LoadTrueTypeFont(lua_State* L) noexcept
		{
			lua::stack_t S(L);

			// 先检查有没有资源池
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
			{
				return luaL_error(L, "can't load resource at this time.");
			}
			
			// 第一个参数，资源名
			std::string_view const name = S.get_value<std::string_view>(1);
			
			// 第二个参数，字体组
			if (!lua_istable(L, 2))
			{
				return luaL_error(L, "invalid parameter #2, required table");
			}
			int const cnt = (int)lua_objlen(L, 2);
			std::vector<core::Graphics::TrueTypeFontInfo> fonts(cnt);
			for (int i = 1; i <= cnt; i += 1)
			{
				auto& font = fonts[i - 1];
				font.source = "";
				font.font_face = 0;
				font.font_size = core::Vector2F(0.0f, 0.0f);
				font.is_force_to_file = false;
				font.is_buffer = false;

				lua_pushinteger(L, i);		// name param fonts i
				lua_gettable(L, 2);			// name param fonts font
				if (!lua_istable(L, -1))
				{
					return luaL_error(L, "invalid value #%d in parameter #2, required table", i);
				}

				lua_getfield(L, -1, "source"); // name param fonts font ?
				if (lua_type(L, -1) == LUA_TSTRING) // name param fonts font v
				{
					font.source = S.get_value<std::string_view>(-1);
				}
				lua_pop(L, 1);				// name param fonts font

				lua_getfield(L, -1, "font_face"); // name param fonts font ?
				if (lua_type(L, -1) == LUA_TNUMBER) // name param fonts font v
				{
					font.font_face = (uint32_t)luaL_checkinteger(L, -1);
				}
				lua_pop(L, 1);				// name param fonts font

				lua_getfield(L, -1, "width"); // name param fonts font ?
				if (lua_type(L, -1) == LUA_TNUMBER) // name param fonts font v
				{
					font.font_size.x = (float)luaL_checknumber(L, -1);
				}
				lua_pop(L, 1);				// name param fonts font

				lua_getfield(L, -1, "height"); // name param fonts font ?
				if (lua_type(L, -1) == LUA_TNUMBER) // name param fonts font v
				{
					font.font_size.y = (float)luaL_checknumber(L, -1);
				}
				lua_pop(L, 1);				// name param fonts font

				lua_pop(L, 1);				// name param fonts
			}

			bool result = pActivedPool->LoadTrueTypeFont(name.data(), fonts.data(), fonts.size());
			lua_pushboolean(L, result);
			
			return 1;
		}
		static int LoadFX(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadFX(name, path))
				return luaL_error(L, "load fx failed (name=%s, path=%s)", name, path);

			return 0;
		}
		static int LoadModel(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* model_path = luaL_checkstring(L, 2);
			
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			if (!pActivedPool->LoadModel(
				name,
				model_path))
			{
				return luaL_error(L, "load model failed (name='%s', model='%s').", name, model_path);
			}
			return 0;
		}
		static int CreateRenderTarget(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			
			if (lua_gettop(L) >= 3)
			{
				const int width = (int)luaL_checkinteger(L, 2);
				const int height = (int)luaL_checkinteger(L, 3);
				if (width < 1 || height < 1)
					return luaL_error(L, "invalid render target size (%dx%d).", width, height);
				bool depth_buffer = true;
				if (lua_gettop(L) >= 4)
					depth_buffer = lua_toboolean(L, 4);
				if (!pActivedPool->CreateRenderTarget(name, width, height, depth_buffer))
					return luaL_error(L, "can't create render target with name '%s'.", name);
			}
			else
			{
				if (!pActivedPool->CreateRenderTarget(name, 0, 0, true))
					return luaL_error(L, "can't create render target with name '%s'.", name);
			}
			
			return 0;
		}
		static int IsRenderTarget(lua_State* L) noexcept
		{
			core::SmartReference<IResourceTexture> p = LRES.FindTexture(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "render target '%s' not found.", luaL_checkstring(L, 1));
			lua_pushboolean(L, p->IsRenderTarget());
			return 1;
		}
		static int SetTexturePreMulAlphaState(lua_State* L) noexcept
		{
			core::SmartReference<IResourceTexture> p = LRES.FindTexture(luaL_checkstring(L, 1));
			if (p)
			{
				p->GetTexture()->setPremultipliedAlpha(lua_toboolean(L, 2));
				return 0;
			}
			return luaL_error(L, "texture '%s' not found.", luaL_checkstring(L, 1));
		}
		static int SetTextureSamplerState(lua_State* L)noexcept
		{
			lua::stack_t S(L);
			std::string_view const sampler_name = S.get_value<std::string_view>(2);
			if (sampler_name == "" || sampler_name == "point+wrap" || sampler_name == "point+clamp" || sampler_name == "linear+wrap" || sampler_name == "linear+clamp")
			{
				std::string_view const tex_name = S.get_value<std::string_view>(1);
				core::SmartReference<IResourceTexture> p = LRES.FindTexture(tex_name.data());
				if (!p)
				{
					spdlog::error("[luastg] lstg.SetTextureSamplerState failed: can't find texture '{}'", tex_name);
					return luaL_error(L, "can't find texture '%s'", tex_name.data());
				}

				// 映射
				core::Graphics::IRenderer::SamplerState state = core::Graphics::IRenderer::SamplerState::LinearClamp;
				if (sampler_name == "point+wrap") state = core::Graphics::IRenderer::SamplerState::PointWrap;
				else if (sampler_name == "point+clamp") state = core::Graphics::IRenderer::SamplerState::PointClamp;
				else if (sampler_name == "linear+wrap") state = core::Graphics::IRenderer::SamplerState::LinearWrap;
				else if (sampler_name == "" || sampler_name == "linear+clamp") state = core::Graphics::IRenderer::SamplerState::LinearClamp;
				else return luaL_error(L, "unknown sampler state '%s'", sampler_name.data());

				// 设置
				core::IGraphicsSampler* p_sampler = LAPP.getRenderer2D()->getKnownSamplerState(state);
				p->GetTexture()->setSamplerState(p_sampler);

				return 0;
			}
			else
			{
				return luaL_error(L, "unsupported deprecated usage");
			}
		}
		static int GetTextureSize(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			core::Vector2U size;
			if (!LRES.GetTextureSize(name, size))
				return luaL_error(L, "texture '%s' not found.", name);
			lua_pushinteger(L, (lua_Integer)size.x);
			lua_pushinteger(L, (lua_Integer)size.y);
			return 2;
		}
		static int RemoveResource(lua_State* L) noexcept
		{
			ResourcePoolType t;
			const char* s = luaL_checkstring(L, 1);
			if (strcmp(s, "global") == 0)
				t = ResourcePoolType::Global;
			else if (strcmp(s, "stage") == 0)
				t = ResourcePoolType::Stage;
			else if (strcmp(s, "none") != 0)
				t = ResourcePoolType::None;
			else
				return luaL_error(L, "invalid argument #1 for 'RemoveResource', requires 'stage', 'global' or 'none'.");

			if (lua_gettop(L) == 1)
			{
				switch (t)
				{
				case ResourcePoolType::Stage:
					LRES.GetResourcePool(ResourcePoolType::Stage)->Clear();
					break;
				case ResourcePoolType::Global:
					LRES.GetResourcePool(ResourcePoolType::Global)->Clear();
					break;
				default:
					break;
				}
			}
			else
			{
				ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 2));
				const char* tResourceName = luaL_checkstring(L, 3);

				switch (t)
				{
				case ResourcePoolType::Stage:
					LRES.GetResourcePool(ResourcePoolType::Stage)->RemoveResource(tResourceType, tResourceName);
					break;
				case ResourcePoolType::Global:
					LRES.GetResourcePool(ResourcePoolType::Global)->RemoveResource(tResourceType, tResourceName);
					break;
				default:
					break;
				}
			}
			
			return 0;
		}
		static int CheckRes(lua_State* L) noexcept
		{
			ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 1));
			const char* tResourceName = luaL_checkstring(L, 2);
			// 先在全局池中寻找再到关卡池中找
			if (LRES.GetResourcePool(ResourcePoolType::Global)->CheckResourceExists(tResourceType, tResourceName))
				lua_pushstring(L, "global");
			else if (LRES.GetResourcePool(ResourcePoolType::Stage)->CheckResourceExists(tResourceType, tResourceName))
				lua_pushstring(L, "stage");
			else
				lua_pushnil(L);
			return 1;
		}
		static int EnumRes(lua_State* L) noexcept
		{
			ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 1));
			LRES.GetResourcePool(ResourcePoolType::Global)->ExportResourceList(L, tResourceType);
			LRES.GetResourcePool(ResourcePoolType::Stage)->ExportResourceList(L, tResourceType);
			return 2;
		}

		static int SetImageScale(lua_State* L) noexcept
		{
			if (lua_gettop(L) <= 1)
			{
				float x = static_cast<float>(luaL_checknumber(L, 1));
				if (x == 0.f)
					return luaL_error(L, "invalid argument #1 for 'SetImageScale'.");
				LRES.SetGlobalImageScaleFactor(x);
			}
			else
			{
				core::SmartReference<IResourceSprite> p = LRES.FindSprite(luaL_checkstring(L, 1));
				if (!p)
					return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
				float x = (float)luaL_checknumber(L, 2);
				p->GetSprite()->setUnitsPerPixel(x);
			}
			return 0;
		}
		static int GetImageScale(lua_State* L) noexcept
		{
			if (lua_gettop(L) <= 0)
			{
				lua_Number ret = LRES.GetGlobalImageScaleFactor();
				lua_pushnumber(L, ret);
				return 1;
			}
			else
			{
				core::SmartReference<IResourceSprite> p = LRES.FindSprite(luaL_checkstring(L, 1));
				if (!p)
					return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
				lua_pushnumber(L, p->GetSprite()->getUnitsPerPixel());
				return 1;
			}
		}
		static int GetImageSize(lua_State* L) noexcept
		{
			core::SmartReference<IResourceSprite> p = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
			core::RectF rect = p->GetSprite()->getTextureRect();
			lua_pushnumber(L, rect.b.x - rect.a.x);
			lua_pushnumber(L, rect.b.y - rect.a.y);
			return 2;
		}
		static int SetImageState(lua_State* L) noexcept
		{
			core::SmartReference<IResourceSprite> p = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3) {
				p->SetColor(*Color::Cast(L, 3));
			}
			else if (lua_gettop(L) == 6)
			{
				p->SetColor(
					*Color::Cast(L, 3),
					*Color::Cast(L, 4),
					*Color::Cast(L, 5),
					*Color::Cast(L, 6)
				);
			}
			return 0;
		}

		//为什么没人提出来这个接口！这个接口给的是图片名称和混合颜色
		static int GetImageColor(lua_State* L) noexcept
		{
			core::SmartReference<IResourceSprite> p = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));

			std::string ColorSuffix[4] = { "Color(", "Color(", "Color(", "Color(" }; //转换到Lstg的格式

			//有没有更好的方法，这也太绿皮了
			for (int i = 0; i < 4; i++) {
				ColorSuffix[i] = ColorSuffix[i] + std::to_string((p->GetColor()[0].a)) + ", " + std::to_string((p->GetColor()[0].r)) + ", " + std::to_string((p->GetColor()[0].g)) + ", " + std::to_string((p->GetColor()[0].b)) + ")";
				if (i < 3) {
					ColorSuffix[i] = ColorSuffix[i] + ", ";
				}
			}
			lua_pushstring(L, (ColorSuffix[0]+ColorSuffix[1]+ColorSuffix[2]+ColorSuffix[3]).c_str());
			return 1;
		}

		//为什么没人提出来这个接口！现在的写法，bullet就是屎山！导致了高光污染！
		static int GetImageBlend(lua_State* L) noexcept
		{
			core::SmartReference<IResourceSprite> p = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
			
			switch (p->GetBlendMode()) {
			case BlendMode::MulAlpha:
				lua_pushstring(L, "mul+alpha");
				break;
			case BlendMode::MulAdd:
				lua_pushstring(L, "mul+add");
				break;
			case BlendMode::MulRev:
				lua_pushstring(L, "mul+rev");
				break;
			case BlendMode::MulSub:
				lua_pushstring(L, "mul+sub");
				break;
			case BlendMode::AddAlpha:
				lua_pushstring(L, "add+alpha");
				break;
			case BlendMode::AddAdd:
				lua_pushstring(L, "add+add");
				break;
			case BlendMode::AddRev:
				lua_pushstring(L, "add+rev");
				break;
			case BlendMode::AddSub:
				lua_pushstring(L, "add+sub");
				break;
			case BlendMode::AlphaBal:
				lua_pushstring(L, "alpha+bal");
				break;
			case BlendMode::MulMin:
				lua_pushstring(L, "mul+min");
				break;
			case BlendMode::MulMax:
				lua_pushstring(L, "mul+max");
				break;
			case BlendMode::MulMutiply:
				lua_pushstring(L, "mul+mul");
				break;
			case BlendMode::MulScreen:
				lua_pushstring(L, "mul+screen");
				break;
			case BlendMode::AddMin:
				lua_pushstring(L, "add+min");
				break;
			case BlendMode::AddMax:
				lua_pushstring(L, "add+max");
				break;
			case BlendMode::AddMutiply:
				lua_pushstring(L, "add+mul");
				break;
			case BlendMode::AddScreen:
				lua_pushstring(L, "add+screen");
				break;
			case BlendMode::One:
				lua_pushstring(L, "one");
				break;
			default:
				break;
			}

			
			return 1;
		}

		static int SetImageCenter(lua_State* L) noexcept
		{
			core::SmartReference<IResourceSprite> p = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
			p->GetSprite()->setTextureCenter(core::Vector2F(
				static_cast<float>(luaL_checknumber(L, 2)),
				static_cast<float>(luaL_checknumber(L, 3)))
			);
			return 0;
		}

		static int SetAnimationScale(lua_State* L) noexcept
		{
			core::SmartReference<IResourceAnimation> p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));
			if (!p->IsSpriteCloned())
				return luaL_error(L, "SetAnimationScale on animation '%s' is invalid, please set each sprite separately.");
			float x = (float)luaL_checknumber(L, 2);
			for (size_t i = 0; i < p->GetCount(); ++i)
				p->GetSprite((uint32_t)i)->GetSprite()->setUnitsPerPixel(x);
			return 0;
		}
		static int GetAnimationScale(lua_State* L) noexcept
		{
			core::SmartReference<IResourceAnimation> p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));
			if (!p->IsSpriteCloned())
				return luaL_error(L, "GetAnimationScale on animation '%s' is invalid, please get from each sprite separately.");
			lua_pushnumber(L, p->GetSprite(0)->GetSprite()->getUnitsPerPixel());
			return 1;
		}
		static int SetAnimationState(lua_State* L) noexcept
		{
			core::SmartReference<IResourceAnimation> p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3)
			{
				p->SetVertexColor(*Color::Cast(L, 3));
			}
			else if (lua_gettop(L) == 6)
			{
				core::Color4B tColors[] = {
					*Color::Cast(L, 3),
					*Color::Cast(L, 4),
					*Color::Cast(L, 5),
					*Color::Cast(L, 6)
				};
				p->SetVertexColor(tColors);
			}
			return 0;
		}
		static int SetAnimationCenter(lua_State* L) noexcept
		{
			core::SmartReference<IResourceAnimation> p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));
			if (!p->IsSpriteCloned())
				return luaL_error(L, "SetAnimationCenter on animation '%s' is invalid, please set each sprite separately.");
			for (size_t i = 0; i < p->GetCount(); ++i)
			{
				p->GetSprite((uint32_t)i)->GetSprite()->setTextureCenter(core::Vector2F(
					static_cast<float>(luaL_checknumber(L, 2)),
					static_cast<float>(luaL_checknumber(L, 3))
				));
			}
			return 0;
		}

		static int SetFontState(lua_State* L) noexcept
		{
			core::SmartReference<IResourceFont> p = LRES.FindSpriteFont(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "sprite font '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3)
			{
				p->SetBlendColor(*Color::Cast(L, 3));
			}
			return 0;
		}

		static int CacheTTFString(lua_State* L) {
			size_t len = 0;
			const char* str = luaL_checklstring(L, 2, &len);
			LRES.CacheTTFFontString(luaL_checkstring(L, 1), str, len);
			return 0;
		}
	};

	luaL_Reg const lib[] = {
		{ "SetResLoadInfo", &Wrapper::SetResLoadInfo },
		{ "SetResourceStatus", &Wrapper::SetResourceStatus },
		{ "GetResourceStatus", &Wrapper::GetResourceStatus },
		{ "LoadTexture", &Wrapper::LoadTexture },
		{ "LoadImage", &Wrapper::LoadSprite },
		{ "CopyImage", &Wrapper::CopySprite },
		{ "LoadAnimation", &Wrapper::LoadAnimation },
		{ "LoadPS", &Wrapper::LoadPS },
		{ "LoadSound", &Wrapper::LoadSound },
		{ "LoadMusic", &Wrapper::LoadMusic },
		{ "LoadFont", &Wrapper::LoadFont },
		{ "LoadTTF", &Wrapper::LoadTTF },
		{ "LoadTrueTypeFont", &Wrapper::LoadTrueTypeFont },
		{ "LoadFX", &Wrapper::LoadFX },
		{ "LoadModel", &Wrapper::LoadModel },
		{ "CreateRenderTarget", &Wrapper::CreateRenderTarget },
		{ "IsRenderTarget", &Wrapper::IsRenderTarget },
		{ "SetTexturePreMulAlphaState", &Wrapper::SetTexturePreMulAlphaState },
		{ "SetTextureSamplerState", &Wrapper::SetTextureSamplerState },
		{ "GetTextureSize", &Wrapper::GetTextureSize },
		{ "RemoveResource", &Wrapper::RemoveResource },
		{ "CheckRes", &Wrapper::CheckRes },
		{ "EnumRes", &Wrapper::EnumRes },

		{ "SetImageScale", &Wrapper::SetImageScale },
		{ "GetImageScale", &Wrapper::GetImageScale },
		{ "GetImageSize", &Wrapper::GetImageSize },
		{ "SetImageState", &Wrapper::SetImageState },
		{ "GetImageBlend", &Wrapper::GetImageBlend },
		{ "GetImageColor", &Wrapper::GetImageColor },
		{ "SetImageCenter", &Wrapper::SetImageCenter },

		{ "SetAnimationScale", &Wrapper::SetAnimationScale },
		{ "GetAnimationScale", &Wrapper::GetAnimationScale },
		{ "SetAnimationState", &Wrapper::SetAnimationState },
		{ "SetAnimationCenter", &Wrapper::SetAnimationCenter },

		{ "SetFontState", &Wrapper::SetFontState },

		{ "CacheTTFString", &Wrapper::CacheTTFString },

		{ NULL, NULL },
	};

	luaL_Reg const lib_empty[] = {
		{ NULL, NULL },
	};

	luaL_register(L, LUASTG_LUA_LIBNAME, lib);                    // ??? lstg
	luaL_register(L, LUASTG_LUA_LIBNAME ".ResourceManager", lib); // ??? lstg lstg.ResourceManager
	lua_setfield(L, -1, "ResourceManager");                       // ??? lstg

	// Nex resource API: lstg.Resource.*
	luaL_Reg const nex_image_lib[] = {
		{ "LoadTexture", &Wrapper::NexImageLoadTexture },
		{ "LoadSprite", &Wrapper::NexImageLoadSprite },
		{ "LoadFullSprite", &Wrapper::NexImageLoadFullSprite },
		{ "GetTextureSize", &Wrapper::NexImageGetTextureSize },
		{ "GetSpriteSize", &Wrapper::NexImageGetSpriteSize },
		{ "GetSpriteScale", &Wrapper::NexImageGetSpriteScale },
		{ NULL, NULL },
	};
	luaL_Reg const nex_audio_lib[] = {
		{ "LoadSound", &Wrapper::NexAudioLoadSound },
		{ "LoadMusic", &Wrapper::NexAudioLoadMusic },
		{ "PlaySound", &Wrapper::NexAudioPlaySound },
		{ "PlayMusic", &Wrapper::NexAudioPlayMusic },
		{ NULL, NULL },
	};
	luaL_Reg const nex_file_lib[] = {
		{ "LoadText", &Wrapper::NexFileLoadText },
		{ "GetText", &Wrapper::NexFileGetText },
		{ "WriteText", &Wrapper::NexFileWriteText },
		{ NULL, NULL },
	};
	luaL_Reg const nex_effect_lib[] = {
		{ "LoadParticle", &Wrapper::NexEffectLoadParticle },
		{ NULL, NULL },
	};

	lua_newtable(L);                  // lstg Resource
	lua_newtable(L);                  // lstg Resource Image
	luaL_register(L, NULL, nex_image_lib);
	lua_setfield(L, -2, "Image");
	lua_newtable(L);                  // lstg Resource Audio
	luaL_register(L, NULL, nex_audio_lib);
	lua_setfield(L, -2, "Audio");
	lua_newtable(L);                  // lstg Resource File
	luaL_register(L, NULL, nex_file_lib);
	lua_setfield(L, -2, "File");
	lua_newtable(L);                  // lstg Resource Effect
	luaL_register(L, NULL, nex_effect_lib);
	lua_setfield(L, -2, "Effect");
	lua_setfield(L, -2, "Resource"); // lstg.Resource = Resource

	lua_pop(L, 1);                                                // ???
}
