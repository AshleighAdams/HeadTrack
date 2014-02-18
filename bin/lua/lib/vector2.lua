_R = _R or debug.getregistry()

_R.Vector2 = {}
local meta = {}

function _R.Vector2:Length()
	
	return math.sqrt(self.x*self.x + self.y*self.y)
end

function _R.Vector2:Normalize()
	self = self / self:Length()
end

function _R.Vector2:GetNormal()
	return self / self:Length()
end

function _R.Vector2:ToAngle()
	local yaw
	
	yaw = math.deg(math.atan2(self.y, self.x))
	
	if yaw < 0 then
		yaw = yaw + 360
	elseif yaw > 360 then
		yaw = yaw - 360
	end
	
	return yaw
end



function meta:__mul(other)
	if type(other) == "number" then
		return Vector2(self.x * other, self.y * other)
	end
	
	return Vector2(self.x * other.x, self.y * other.y)
end

function meta:__add(other)
	if type(other) == "number" then
		return Vector2(self.x + other, self.y + other)
	end
	
	return Vector2(self.x + other.x, self.y + other.y)
end

function meta:__sub(other)
	if type(other) == "number" then
		return Vector2(self.x - other, self.y - other)
	end
	
	return Vector2(self.x - other.x, self.y - other.y)
end

function meta:__div(other)
	if type(other) == "number" then
		return Vector2(self.x / other, self.y / other)
	end
	
	return Vector2(self.x / other.x, self.y / other.y)
end

function meta:__pow(o)
	if type(o) ~= "number" then
		error("power not a number!")
	end
	return Vector2(self.x ^ o, self.y ^ o)
end

function meta:__unm()
	return Vector2(-self.x, -self.y)
end

function meta:__eq(o)
	return self.x == o.x and self.y == o.y
end

function meta:__tostring()
	return tostring(self.x) .. ", " .. tostring(self.y)
end

function meta:__index(k)
	return _R.Vector2[k]
end


function Vector2(x, y)
	local ret = {}
	
	setmetatable(ret, meta)
	
	if y == nil then
		ret.x = x or 0
		ret.y = y or 0
	else
		ret.x = math.cos(math.rad(x))
		ret.y = math.sin(math.rad(x))
	end
	
	return ret
end