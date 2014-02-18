_R = _R or debug.getregistry()

_R.Angle = {}
local meta = {}

local function AjustIt(x)
	if x >= 180 then
		x = x - 360
	elseif x <= -180 then
		x = x + 360
	end
	return x
end

function _R.Angle:Distance(other)
	local p = self.p - other.p
	local y = self.y - other.y
	local r = self.r - other.r
	
	p = AjustIt(p)
	y = AjustIt(y)
	r = AjustIt(r)
	
	return math.sqrt(p*p + y*y + r*r)
end

function _R.Angle:Approach(ang, factor)
	local p = self.p - ang.p
	local y = self.y - ang.y
	local r = self.r - ang.r
	
	p = AjustIt(p)
	y = AjustIt(y)
	r = AjustIt(r)
	
	return Angle(self.p - p * factor,
				self.y - y * factor,
				self.r - r * factor)
end

function _R.Angle:Unpack()
	return self.p, self.y, self.r
end

function meta:__mul(other)
	if type(other) == "number" then
		return Angle(self.p * other, self.y * other, self.r * other)
	end
	
	return Angle(self.p * other.p, self.y * other.y, self.r * other.r)
end

function meta:__add(other)
	if type(other) == "number" then
		return Angle(self.p + other, self.y + other, self.r + other)
	end
	
	return Angle(self.p + other.p, self.y + other.y, self.r + other.r)
end

function meta:__sub(other)
	if type(other) == "number" then
		return Angle(self.p - other, self.y - other, self.r - other)
	end
	
	return Angle(self.p - other.p, self.y - other.y, self.r - other.r)
end

function meta:__div(other)
	if type(other) == "number" then
		return Angle(self.p / other, self.y / other, self.r / other)
	end
	
	return Angle(self.p / other.p, self.y / other.y, self.r / other.r)
end

function meta:__pow(o)
	if type(o) ~= "number" then
		error("power not a number!")
	end
	return Angle(self.p ^ o, self.y ^ o, self.r ^ o)
end

function meta:__unm()
	return Angle(-self.p, -self.y, -self.r)
end

function meta:__eq(o)
	return self.p == o.p and self.y == o.y and self.r == o.r
end

function meta:__tostring()
	return tostring(self.p) .. ", " .. tostring(self.y) .. ", " .. tostring(self.r)
end

local members = { p = true, y = true, r = true }

function meta:__newindex(k, v)
	if members[k] then
		local nv = v % 360
		rawset(self._m, k, nv)
	end
end

function meta:__index(k)
	if not members[k] then
		return _R.Angle[k]
	end
	return self._m[k]
end


function Angle(p, y, r)
	local ret = {}
	ret._m = {}
	
	setmetatable(ret, meta)
	
	ret.p = p or 0
	ret.y = y or 0
	ret.r = r or 0
	
	return ret
end