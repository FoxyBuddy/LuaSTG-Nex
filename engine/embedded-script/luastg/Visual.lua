local lstg = require("lstg")
local type = type
local setmetatable = setmetatable
local assert = assert
local tostring = tostring

-- VisualNode is the common base for the new rendering-side objects.
-- Convention: x/y/z/rot/hscale/vscale are local transform values.
-- When a node has a parent, its final transform is inherited from the parent.
local VisualNode = {}
VisualNode.__index = VisualNode
VisualNode.__visual_node = true
VisualNode.x = 0
VisualNode.y = 0
VisualNode.z = 0
VisualNode.rot = 0
VisualNode.hscale = 1
VisualNode.vscale = 1
VisualNode.blend = "mul+alpha"
VisualNode.color = nil
VisualNode.hide = false

function VisualNode:world_transform()
    local x = self.x or 0
    local y = self.y or 0
    local z = self.z or 0
    local rot = self.rot or 0
    local hscale = self.hscale or 1
    local vscale = self.vscale or hscale

    local parent = self.parent or self.master
    while parent do
        x = x + (parent.x or 0)
        y = y + (parent.y or 0)
        z = z + (parent.z or 0)
        rot = rot + (parent.rot or 0)
        hscale = hscale * (parent.hscale or 1)
        vscale = vscale * (parent.vscale or parent.hscale or 1)
        parent = parent.parent or parent.master
    end

    return x, y, z, rot, hscale, vscale
end

function VisualNode:render_self(x, y, z)
    if self.hide or not self.img then
        return
    end

    local wx, wy, wz, rot, hscale, vscale = self:world_transform()

    if x ~= nil then
        wx = x
    end
    if y ~= nil then
        wy = y
    end
    if z ~= nil then
        wz = z
    end

    if self.is_animation then
        lstg.RenderAnimationEx(
            self.img,
            self.ani_timer or self.ani or self.timer or 0,
            wx,
            wy,
            wz,
            rot,
            hscale,
            vscale,
            self.blend,
            self.color
        )
    else
        lstg.RenderEx(
            self.img,
            wx,
            wy,
            wz,
            rot,
            hscale,
            vscale,
            self.blend,
            self.color
        )
    end
end

-- VisualClip is one concrete animation state.
-- A VisualTrack activates at most one clip.
local VisualClip = setmetatable({}, { __index = VisualNode })
VisualClip.__index = VisualClip
VisualClip.__visual_node = true

function VisualClip:enter(previous)
end

function VisualClip:leave(next)
end

function VisualClip:frame()
end

function VisualClip:render()
    self:render_self()
end

-- VisualTrack is a mutually-exclusive animation slot.
-- Examples: boss body, boss aura, boss hpbar.
local VisualTrack = setmetatable({}, { __index = VisualNode })
VisualTrack.__index = VisualTrack
VisualTrack.__visual_node = true

function VisualTrack:init(master)
    self.master = master
    self.parent = master
    self.clips = self.clips or {}
    self.current = nil
    self.current_name = nil
end

local function new_visual_instance(class, fields)
    if class.__index == nil then
        class.__index = class
    end
    return setmetatable(fields or {}, class)
end

function VisualTrack:add_clip(name, clip_class, ...)
    self.clips = self.clips or {}

    local clip = new_visual_instance(clip_class, {
        track = self,
        parent = self,
        master = self.master,
    })

    if clip.init then
        clip:init(self, ...)
    end

    self.clips[name] = clip
    return clip
end

function VisualTrack:play(name)
    if self.current_name == name then
        return self.current
    end

    local next_clip = assert(
        self.clips and self.clips[name],
        "unknown visual clip: " .. tostring(name)
    )

    local previous_name = self.current_name
    local previous_clip = self.current

    if previous_clip and previous_clip.leave then
        previous_clip:leave(name)
    end

    self.current_name = name
    self.current = next_clip

    if next_clip.enter then
        next_clip:enter(previous_name)
    end

    return next_clip
end

function VisualTrack:frame()
    if self.current and self.current.frame then
        self.current:frame()
    end
end

function VisualTrack:render()
    if self.hide then
        return
    end

    if self.current and self.current.render then
        self.current:render()
    end
end

function lstg.RenderVisual(node, x, y, z)
    if node and node.render_self then
        return node:render_self(x, y, z)
    end

    if node and node.render then
        return node:render()
    end
end

-- Compatibility convenience:
-- Render(visual_node) goes through the new non-polluting path.
local legacy_Render = lstg.Render

function lstg.Render(first, ...)
    if type(first) == "table" and first.__visual_node then
        return lstg.RenderVisual(first, ...)
    end

    return legacy_Render(first, ...)
end

lstg.VisualNode = VisualNode
lstg.VisualClip = VisualClip
lstg.VisualTrack = VisualTrack

return lstg