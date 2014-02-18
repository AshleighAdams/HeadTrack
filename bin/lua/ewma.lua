-- This is used in one of the first videos
include("lib/angle.lua")
include("lib/ewma.lua")
include("scale.lua")

local weight = 64
local factor = 1024 * 1024

local ewma_p = EWMA(factor, weight)
local ewma_y = EWMA(factor, weight)
local ewma_r = EWMA(factor, weight)

function clean(p, y, r)
	local by,bp,br;
	by = y
	bp = p
	br = r

	ewma_p:Add(p)
	ewma_y:Add(y)
	ewma_r:Add(r)

	p = ewma_p:Get()
	y = ewma_y:Get()
	r = ewma_r:Get()
	
	local dy = math.abs(by - y)
	local dp = math.abs(bp - p)
	local dr = math.abs(br - r)
	
	dy = math.max(2, 100 - dy * 50)
	dp = math.max(2, 100 - dp * 50)
	dr = math.max(2, 100 - dr * 50)

	ewma_y:Set(2048, dy)
	ewma_p:Set(2048, dp)
	ewma_r:Set(2048, dr)

	return p, y, r
end
