-- This is the one that works best for me, you may beed to change the variables below to suit you and your phone.

smoothing = 0 -- Applied before it is processed, but creates input lag

movement_threshold_min = 0.001 -- 0.04
movement_threshold_max = 0.004 -- 0.04
movement_threshold_change_ceof = 800
movement_threshold_change_ceof_down = 0.005 -- fall of slower so gyro drift doesn't annoy us

max_desync_threshold = 1 -- will "resync" when out of this range
force_resync_threshold = 100 -- force it now, it's too far


-- !!! do not change below unless you know what you're doing !!! --

include("lib/angle.lua")
include("lib/vector2.lua")
include("scale.lua") -- To scale/change output

local movement_threshold = movement_threshold_max
local stilltime = 0

local out = Angle(0, 0, 0)
local last = Angle(0, 0, 0)
local offset = Angle(0, 0, 0)
local lasttime = gettime()

local avg = {}
for i = 1, smoothing do
	table.insert(avg, Angle())
end

function clean(p, y, r)
	local cur = Angle(p, y, r)
	
	if smoothing ~= 0 then
		table.remove(avg, 1)	
		table.insert(avg, cur)
		
		local vec_p = Vector2()
		local vec_y = Vector2()
		local vec_r = Vector2()
		
		for k,v in pairs(avg) do
			vec_p = vec_p + Vector2(v.p)
			vec_y = vec_y + Vector2(v.y)
			vec_r = vec_r + Vector2(v.r)
		end
		
		vec_p:Normalize()
		vec_y:Normalize()
		vec_r:Normalize()
		
		cur.p = vec_p:ToAngle()
		cur.y = vec_y:ToAngle()
		cur.r = vec_r:ToAngle()
	end
	
	local t = gettime() - lasttime
	lasttime = gettime()
	local dist = cur:Distance(last) * t
		
	-- If we've moved quickly, we don't want to take small movements into consideration...
	if dist > movement_threshold_min then
		stilltime = stilltime + (dist - movement_threshold_min) * t * movement_threshold_change_ceof
	else
		stilltime = stilltime - t * movement_threshold_change_ceof * movement_threshold_change_ceof_down
	end
	stilltime = math.min(1, math.max(0, stilltime))
	
	local x = stilltime
	local to_min, to_max = movement_threshold_min, movement_threshold_max
	
	local from_diff = 1 - 0
	local to_diff = to_max - to_min
	
	local y = (x - 0) / from_diff
	movement_threshold = to_min + to_diff * y
	
	
	-- Slowly reset the offset
	local offset_amm = offset:Distance(Angle(0, 0, 0))
	if dist > movement_threshold or offset_amm > force_resync_threshold then
		if offset_amm > max_desync_threshold then
			offset = offset:Approach(Angle(0, 0, 0), t)
		end
		
		out = cur + offset
	else
		offset = out - cur
	end
	
	last = cur
	return out:Unpack()
end
